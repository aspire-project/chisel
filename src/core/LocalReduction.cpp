#include "LocalReduction.h"

#include <spdlog/spdlog.h>

#include "clang/Lex/Lexer.h"

#include "FileManager.h"
#include "OptionManager.h"
#include "Profiler.h"

using BinaryOperator = clang::BinaryOperator;
using BreakStmt = clang::BreakStmt;
using CallExpr = clang::CallExpr;
using ContinueStmt = clang::ContinueStmt;
using CompoundStmt = clang::CompoundStmt;
using DeclGroupRef = clang::DeclGroupRef;
using DeclStmt = clang::DeclStmt;
using FunctionDecl = clang::FunctionDecl;
using GotoStmt = clang::GotoStmt;
using IfStmt = clang::IfStmt;
using LabelStmt = clang::LabelStmt;
using ReturnStmt = clang::ReturnStmt;
using SourceRange = clang::SourceRange;
using SourceLocation = clang::SourceLocation;
using Stmt = clang::Stmt;
using UnaryOperator = clang::UnaryOperator;
using WhileStmt = clang::WhileStmt;
using VarDecl = clang::VarDecl;
using Decl = clang::Decl;
using LabelDecl = clang::LabelDecl;
using Expr = clang::Expr;
using DeclRefExpr = clang::DeclRefExpr;

void LocalReduction::Initialize(clang::ASTContext &Ctx) {
  Reduction::Initialize(Ctx);
  CollectionVisitor = new LocalElementCollectionVisitor(this);
}

bool LocalReduction::HandleTopLevelDecl(DeclGroupRef D) {
  for (DeclGroupRef::iterator I = D.begin(), E = D.end(); I != E; ++I) {
    CollectionVisitor->TraverseDecl(*I);
  }
  return true;
}

void LocalReduction::HandleTranslationUnit(clang::ASTContext &Ctx) {
  for (auto const &f : Functions) {
    Queue.push(f->getBody());

    if (FunctionDecl *FD = llvm::dyn_cast<FunctionDecl>(f))
      CurrentFunction = FD;

    while (!Queue.empty()) {
      Stmt *S = Queue.front();
      Queue.pop();
      doHierarchicalDeltaDebugging(S);
    }
  }
}

DDElement LocalReduction::CastElement(Stmt *S) { return S; }

bool LocalReduction::callOracle() {
  Profiler::GetInstance()->incrementLocalReductionCounter();

  if (Reduction::callOracle()) {
    Profiler::GetInstance()->incrementSuccessfulLocalReductionCounter();
    FileManager::GetInstance()->saveTemp("local", true);
    return true;
  } else {
    FileManager::GetInstance()->saveTemp("local", false);
    return false;
  }
}

bool LocalReduction::test(DDElementVector &ToBeRemoved) {
  std::vector<clang::SourceRange> Ranges;
  std::vector<llvm::StringRef> Reverts;

  for (auto Element : ToBeRemoved) {
    if (Element.isNull())
      continue;

    clang::SourceLocation Start =
        Element.get<Stmt *>()->getSourceRange().getBegin();
    clang::SourceLocation End;
    clang::Stmt *S = Element.get<Stmt *>();
    if (DeclStmt *DS = llvm::dyn_cast<DeclStmt>(S))
      continue;

    End = getEndOfStmt(S);

    if (End.isInvalid() || Start.isInvalid())
      return false;

    SourceRange Range(Start, End);
    Ranges.emplace_back(Range);
    llvm::StringRef Revert = getSourceText(Start, End);
    Reverts.emplace_back(Revert);
    removeSourceText(Start, End);
  }
  TheRewriter.overwriteChangedFiles();
  if (callOracle()) {
    return true;
  } else {
    for (int i = 0; i < Reverts.size(); i++) {
      TheRewriter.ReplaceText(Ranges[i], Reverts[i]);
    }
    TheRewriter.overwriteChangedFiles();
    return false;
  }

  return false;
}

DDElementVector LocalReduction::getAllChildren(Stmt *S) {
  std::queue<Stmt *> ToVisit;
  DDElementVector AllChildren;
  ToVisit.push(S);
  while (!ToVisit.empty()) {
    auto C = ToVisit.front();
    ToVisit.pop();
    AllChildren.emplace_back(C);
    for (auto const &Child : C->children()) {
      if (Child != NULL)
        ToVisit.push(Child);
    }
  }

  return AllChildren;
}

int LocalReduction::countReturnStmts(DDElementVector &Elements) {
  int NumReturns = 0;
  for (auto const &E : Elements)
    if (ReturnStmt *RS = llvm::dyn_cast<ReturnStmt>(E.get<Stmt *>()))
      NumReturns++;
  return NumReturns;
}

bool LocalReduction::noReturn(DDElementVector &FunctionStmts,
                              DDElementVector &AllRemovedStmts) {
  if (CurrentFunction->getReturnType().getTypePtr()->isVoidType())
    return false;
  if (countReturnStmts(FunctionStmts) == countReturnStmts(AllRemovedStmts))
    return true;
  return false;
}

bool LocalReduction::danglingLabel(DDElementSet &Remaining) {
  std::set<LabelStmt *> LabelDefs;
  std::set<LabelStmt *> LabelUses;

  for (auto const &E : Remaining) {
    if (E.isNull() || !E.is<Stmt *>())
      continue;
    Stmt *S = E.get<Stmt *>();
    if (GotoStmt *GS = llvm::dyn_cast<GotoStmt>(S))
      LabelUses.insert(GS->getLabel()->getStmt());
    else if (LabelStmt *LS = llvm::dyn_cast<LabelStmt>(S))
      LabelDefs.insert(LS);
  }

  return !(std::includes(LabelDefs.begin(), LabelDefs.end(), LabelUses.begin(),
                         LabelUses.end()));
}

std::vector<DeclRefExpr *> LocalReduction::getDeclRefExprs(Expr *E) {
  std::vector<DeclRefExpr *> result;
  DDElementVector Children = getAllChildren(E);
  for (auto const &C : Children) {
    if (C.isNull())
      continue;
    Stmt *S = C.get<Stmt *>();
    if (DeclRefExpr *DRE = llvm::dyn_cast<DeclRefExpr>(S))
      result.emplace_back(DRE);
  }
  return result;
}

void LocalReduction::addDefUse(DeclRefExpr *DRE, std::set<Decl *> &DU) {
  if (VarDecl *VD = llvm::dyn_cast<VarDecl>(DRE->getDecl()))
    DU.insert(DRE->getDecl());
}

bool LocalReduction::brokenDependency(DDElementSet &Remaining) {
  std::set<Decl *> Defs, Uses;
  for (auto const &E : Remaining) {
    if (E.isNull() || !E.is<Stmt *>())
      continue;
    Stmt *S = E.get<Stmt *>();
    if (BinaryOperator *BO = llvm::dyn_cast<BinaryOperator>(S)) {
      if (BO->isCompoundAssignmentOp() || BO->isShiftAssignOp()) {
        for (auto C : getDeclRefExprs(BO->getLHS())) {
          addDefUse(C, Defs);
          addDefUse(C, Uses);
        }
        for (auto C : getDeclRefExprs(BO->getRHS()))
          addDefUse(C, Uses);
      } else if (BO->isAssignmentOp()) {
        for (auto C : getDeclRefExprs(BO->getLHS()))
          addDefUse(C, Defs);
        for (auto C : getDeclRefExprs(BO->getRHS()))
          addDefUse(C, Uses);
      } else {
        for (auto C : getDeclRefExprs(BO->getLHS()))
          addDefUse(C, Uses);
        for (auto C : getDeclRefExprs(BO->getRHS()))
          addDefUse(C, Uses);
      }
    } else if (UnaryOperator *UO = llvm::dyn_cast<UnaryOperator>(S)) {
      switch (UO->getOpcode()) {
      case clang::OO_PlusPlus:
      case clang::OO_MinusMinus:
        for (auto C : getDeclRefExprs(UO->getSubExpr())) {
          addDefUse(C, Defs);
          addDefUse(C, Uses);
        }
        break;
      case clang::OO_Amp:
        for (auto C : getDeclRefExprs(UO->getSubExpr()))
          addDefUse(C, Defs);
        break;
      default:
        for (auto C : getDeclRefExprs(UO->getSubExpr()))
          addDefUse(C, Uses);
      }
    } else if (DeclStmt *DS = llvm::dyn_cast<DeclStmt>(S)) {
      for (auto D : DS->decls())
        Defs.insert(D);
    } else if (CallExpr *CE = llvm::dyn_cast<CallExpr>(S)) {
      for (int I = 0; I < CE->getNumArgs(); I++)
        for (auto C : getDeclRefExprs(CE->getArg(I)))
          addDefUse(C, Defs);
    } else if (DeclRefExpr *DRE = llvm::dyn_cast<DeclRefExpr>(S)) {
        addDefUse(DRE, Uses);
    }
    for (auto P : CurrentFunction->parameters())
      Defs.insert(P);
  }
  return !(std::includes(Defs.begin(), Defs.end(), Uses.begin(), Uses.end()));
}

bool LocalReduction::isInvalidChunk(DDElementVector &Chunk) {
  DDElementVector FunctionStmts = getAllChildren(CurrentFunction->getBody());
  DDElementVector AllRemovedStmts;
  for (auto S : Chunk) {
    auto Children = getAllChildren(S.get<Stmt *>());
    AllRemovedStmts.insert(AllRemovedStmts.end(), Children.begin(),
                           Children.end());
  }
  auto FSet = toSet(FunctionStmts);
  auto ASet = toSet(AllRemovedStmts);
  auto Remaining = setDifference(FSet, ASet);
  if (noReturn(FunctionStmts, AllRemovedStmts))
    return true;
  if (danglingLabel(Remaining))
    return true;
  if (brokenDependency(Remaining))
    return true;
  return false;
}

void LocalReduction::doHierarchicalDeltaDebugging(Stmt *S) {
  if (S == NULL)
    return;
  clang::SourceLocation Start = S->getSourceRange().getBegin();
  const clang::SourceManager &SM = Context->getSourceManager();
  std::string Loc = Start.printToString(SM);

  if (IfStmt *IS = llvm::dyn_cast<IfStmt>(S)) {
    spdlog::get("Logger")->debug("HDD IF at " + Loc);
    reduceIf(IS);
  } else if (WhileStmt *WS = llvm::dyn_cast<WhileStmt>(S)) {
    spdlog::get("Logger")->debug("HDD WHILE at " + Loc);
    reduceWhile(WS);
  } else if (CompoundStmt *CS = llvm::dyn_cast<CompoundStmt>(S)) {
    spdlog::get("Logger")->debug("HDD Compound at " + Loc);
    reduceCompound(CS);
  } else if (LabelStmt *LS = llvm::dyn_cast<LabelStmt>(S)) {
    spdlog::get("Logger")->debug("HDD Label at " + Loc);
    reduceLabel(LS);
  } else {
    return;
    // TODO add other cases: For, Do, Switch...
  }
}

std::vector<Stmt *> LocalReduction::getBodyStatements(CompoundStmt *CS) {
  std::vector<Stmt *> Stmts;
  for (auto S : CS->body()) {
    if (S != NULL)
      Stmts.emplace_back(S);
  }
  return Stmts;
}

void LocalReduction::reduceIf(IfStmt *IS) {
  SourceLocation BeginIf = IS->getSourceRange().getBegin();
  SourceLocation EndIf = getEndOfStmt(IS);
  SourceLocation EndCond =
      getEndLocationUntil(IS->getCond()->getSourceRange().getEnd(), ')');
  SourceLocation EndThen = getEndOfStmt(IS->getThen());

  if (BeginIf.isInvalid() || EndIf.isInvalid() || EndCond.isInvalid() ||
      EndThen.isInvalid())
    return;

  llvm::StringRef RevertIf = getSourceText(BeginIf, EndIf);

  if (IS->getElse()) {
    SourceLocation ElseLoc = IS->getElseLoc();
    if (ElseLoc.isInvalid())
      return;

    removeSourceText(BeginIf, EndCond);
    removeSourceText(ElseLoc, EndIf);
    TheRewriter.overwriteChangedFiles();
    if (callOracle()) {
      Queue.push(IS->getThen());
    } else {
      TheRewriter.ReplaceText(SourceRange(BeginIf, EndIf), RevertIf);
      TheRewriter.overwriteChangedFiles();
      removeSourceText(BeginIf, ElseLoc.getLocWithOffset(3));
      TheRewriter.overwriteChangedFiles();
      if (callOracle()) {
        Queue.push(IS->getElse());
      } else {
        TheRewriter.ReplaceText(SourceRange(BeginIf, EndIf), RevertIf);
        TheRewriter.overwriteChangedFiles();
        Queue.push(IS->getThen());
        Queue.push(IS->getElse());
      }
    }
  } else {
    removeSourceText(BeginIf, EndCond);
    TheRewriter.overwriteChangedFiles();
    if (callOracle()) {
      Queue.push(IS->getThen());
    } else {
      TheRewriter.ReplaceText(SourceRange(BeginIf, EndIf), RevertIf);
      TheRewriter.overwriteChangedFiles();
      Queue.push(IS->getThen());
    }
  }
}

void LocalReduction::reduceWhile(WhileStmt *WS) {
  auto Body = WS->getBody();
  SourceLocation BeginWhile = WS->getSourceRange().getBegin();
  SourceLocation EndWhile = getEndOfStmt(WS);
  SourceLocation EndCond =
      getEndLocationUntil(WS->getCond()->getSourceRange().getEnd(), ')');

  llvm::StringRef Revert =
      getSourceText(BeginWhile, EndWhile);

  removeSourceText(BeginWhile, EndCond);
  TheRewriter.overwriteChangedFiles();
  if (callOracle()) {
    Queue.push(Body);
  } else {
    TheRewriter.ReplaceText(SourceRange(BeginWhile, EndWhile), Revert);
    TheRewriter.overwriteChangedFiles();
    Queue.push(Body);
  }
}

void LocalReduction::reduceCompound(CompoundStmt *CS) {
  auto Stmts = getBodyStatements(CS);
  DDElementVector Elements;
  Elements.resize(Stmts.size());
  std::transform(Stmts.begin(), Stmts.end(), Elements.begin(), CastElement);
  DDElementSet Removed = doDeltaDebugging(Elements);
  for (auto S : Stmts) {
    if (Removed.find(S) == Removed.end())
      Queue.push(S);
  }
}

void LocalReduction::reduceLabel(LabelStmt *LS) {
  Queue.push(LS->getSubStmt());
}

bool LocalElementCollectionVisitor::VisitFunctionDecl(FunctionDecl *FD) {
  spdlog::get("Logger")->debug("Visit Function Decl: {}",
                               FD->getNameInfo().getAsString());
  if (FD->isThisDeclarationADefinition())
    Consumer->Functions.emplace_back(FD);
  return true;
}
