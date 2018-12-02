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
  std::string TempFileName =
      FileManager::GetInstance()->getTempFileName("local");

  if (Reduction::callOracle()) {
    FileManager::GetInstance()->updateBest();
    Profiler::GetInstance()->incrementSuccessfulLocalReductionCounter();
    if (OptionManager::SaveTemp)
      writeToFile(TempFileName + ".success.c");
    return true;
  } else {
    if (OptionManager::SaveTemp)
      writeToFile(TempFileName + ".fail.c");
    return false;
  }
}

clang::SourceLocation
LocalReduction::getEndLocation(clang::SourceLocation Loc) {
  const clang::SourceManager &SM = Context->getSourceManager();
  clang::Token Tok;

  clang::SourceLocation Beginning =
      clang::Lexer::GetBeginningOfToken(Loc, SM, Context->getLangOpts());
  clang::Lexer::getRawToken(Beginning, Tok, SM, Context->getLangOpts());

  clang::SourceLocation End;
  if (Tok.getKind() == clang::tok::semi ||
      Tok.getKind() == clang::tok::r_brace) {
    End = Loc.getLocWithOffset(1);
  } else {
    End = getEndLocationAfter(Loc, ';');
  }
  return End;
}

bool LocalReduction::test(std::vector<DDElement> &ToBeRemoved) {
  std::vector<clang::SourceRange> Ranges;
  std::vector<std::string> Reverts;

  for (auto Element : ToBeRemoved) {
    clang::SourceLocation Start =
        Element.get<Stmt *>()->getSourceRange().getBegin();
    clang::SourceLocation End;

    clang::Stmt *S = Element.get<Stmt *>();

    if (CompoundStmt *CS = llvm::dyn_cast<CompoundStmt>(S)) {
      End = CS->getRBracLoc().getLocWithOffset(1);
    } else if (IfStmt *IS = llvm::dyn_cast<IfStmt>(S)) {
      End = getEndLocation(IS->getSourceRange().getEnd());
    } else if (WhileStmt *WS = llvm::dyn_cast<WhileStmt>(S)) {
      End = getEndLocation(WS->getSourceRange().getEnd());
    } else if (LabelStmt *LS = llvm::dyn_cast<LabelStmt>(S)) {
      auto SubStmt = LS->getSubStmt();
      if (CompoundStmt *LS_CS = llvm::dyn_cast<CompoundStmt>(SubStmt)) {
        End = LS_CS->getRBracLoc().getLocWithOffset(1);
      } else {
        End = getEndLocationAfter(SubStmt->getSourceRange(), ';');
      }
    } else if (BinaryOperator *BO = llvm::dyn_cast<BinaryOperator>(S)) {
      End = getEndLocationAfter(BO->getSourceRange(), ';');
    } else if (ReturnStmt *RS = llvm::dyn_cast<ReturnStmt>(S)) {
      End = getEndLocationAfter(RS->getSourceRange(), ';');
    } else if (GotoStmt *GS = llvm::dyn_cast<GotoStmt>(S)) {
      End = getEndLocationAfter(GS->getSourceRange(), ';');
    } else if (BreakStmt *BS = llvm::dyn_cast<BreakStmt>(S)) {
      End = getEndLocationAfter(BS->getSourceRange(), ';');
    } else if (ContinueStmt *CS = llvm::dyn_cast<ContinueStmt>(S)) {
      End = getEndLocationAfter(CS->getSourceRange(), ';');
    } else if (DeclStmt *DS = llvm::dyn_cast<DeclStmt>(S)) {
      End = DS->getSourceRange().getEnd().getLocWithOffset(1);
    } else if (CallExpr *CE = llvm::dyn_cast<CallExpr>(S)) {
      End = getEndLocationAfter(CE->getSourceRange(), ';');
    } else if (UnaryOperator *UO = llvm::dyn_cast<UnaryOperator>(S)) {
      End = getEndLocationAfter(UO->getSourceRange(), ';');
    } else {
      return false;
    }

    if (End.isInvalid() || Start.isInvalid())
      return false;

    SourceRange Range(Start, End);
    Ranges.emplace_back(Range);
    std::string Revert = getSourceText(Range);
    Reverts.emplace_back(Revert);
    removeSourceText(SourceRange(Start, End));
  }

  writeToFile(OptionManager::InputFile);

  if (callOracle()) {
    return true;
  } else {
    for (int i = 0; i < Reverts.size(); i++) {
      TheRewriter.ReplaceText(Ranges[i], Reverts[i]);
    }
    writeToFile(OptionManager::InputFile);
    return false;
  }
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

bool LocalReduction::danglingLabel(DDElementVector &FunctionStmts,
                                   DDElementVector &AllRemovedStmts) {
  auto FSet = toSet(FunctionStmts);
  auto ASet = toSet(AllRemovedStmts);
  auto Remaining = setDifference(FSet, ASet);
  std::vector<GotoStmt *> Gotos;
  std::vector<LabelStmt *> Labels;
  for (auto S : Remaining) {
    if (GotoStmt *GS = llvm::dyn_cast<GotoStmt>(S.get<Stmt *>())) {
      Gotos.emplace_back(GS);
    } else if (LabelStmt *LS = llvm::dyn_cast<LabelStmt>(S.get<Stmt *>())) {
      Labels.emplace_back(LS);
    }
  }

  if (std::any_of(Gotos.begin(), Gotos.end(), [&](GotoStmt *G) {
        return (std::find(Labels.begin(), Labels.end(),
                          G->getLabel()->getStmt()) == Labels.end());
      }))
    return true;
  return false;
}

bool LocalReduction::brokenDependency(DDElementVector &Chunk) {
  return !(std::all_of(std::begin(Chunk), std::end(Chunk), [&](DDElement i) {
    Stmt *S = i.get<Stmt *>();
    if (DeclStmt *DS = llvm::dyn_cast<DeclStmt>(S)) {
      return UseInfo[*(DS->decl_begin())].size() == 0;
    } else {
      return true;
    }
  }));
}

std::vector<DDElementVector>
LocalReduction::refineChunks(std::vector<DDElementVector> &Chunks) {
  std::vector<DDElementVector> Result;
  DDElementVector FunctionStmts = getAllChildren(CurrentFunction->getBody());
  for (auto &Chunk : Chunks) {
    DDElementVector AllRemovedStmts;
    for (auto S : Chunk) {
      auto Children = getAllChildren(S.get<Stmt *>());
      AllRemovedStmts.insert(AllRemovedStmts.end(), Children.begin(),
                             Children.end());
    }
    if (noReturn(FunctionStmts, AllRemovedStmts))
      continue;
    if (danglingLabel(FunctionStmts, AllRemovedStmts))
      continue;
    if (brokenDependency(Chunk))
      continue;
    Result.emplace_back(Chunk);
  }
  return Result;
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
    Stmts.emplace_back(S);
  }
  return Stmts;
}

void LocalReduction::reduceIf(IfStmt *IS) {
  SourceLocation BeginIf = IS->getSourceRange().getBegin();
  SourceLocation EndIf = getEndLocation(IS->getSourceRange().getEnd());
  SourceLocation EndCond =
      IS->getThen()->getSourceRange().getBegin().getLocWithOffset(-1);
  SourceLocation EndThen = IS->getThen()->getSourceRange().getEnd();

  if (BeginIf.isInvalid() || EndIf.isInvalid() || EndCond.isInvalid() ||
      EndThen.isInvalid())
    return;

  std::string RevertIf = getSourceText(SourceRange(BeginIf, EndIf));

  if (IS->getElse()) {
    SourceLocation ElseLoc = IS->getElseLoc();
    if (ElseLoc.isInvalid())
      return;

    removeSourceText(SourceRange(BeginIf, EndCond));
    removeSourceText(SourceRange(ElseLoc, EndIf));
    writeToFile(OptionManager::InputFile);
    if (callOracle()) {
      Queue.push(IS->getThen());
    } else {
      TheRewriter.ReplaceText(SourceRange(BeginIf, EndIf), RevertIf);
      writeToFile(OptionManager::InputFile);
      removeSourceText(SourceRange(BeginIf, ElseLoc.getLocWithOffset(4)));
      writeToFile(OptionManager::InputFile);
      if (callOracle()) {
        Queue.push(IS->getElse());
      } else {
        TheRewriter.ReplaceText(SourceRange(BeginIf, EndIf), RevertIf);
        writeToFile(OptionManager::InputFile);
        Queue.push(IS->getThen());
        Queue.push(IS->getElse());
      }
    }
  } else {
    removeSourceText(SourceRange(BeginIf, EndCond));
    writeToFile(OptionManager::InputFile);
    if (callOracle()) {
      Queue.push(IS->getThen());
    } else {
      TheRewriter.ReplaceText(SourceRange(BeginIf, EndIf), RevertIf);
      writeToFile(OptionManager::InputFile);
      Queue.push(IS->getThen());
    }
  }
}

void LocalReduction::reduceWhile(WhileStmt *WS) {
  auto Body = WS->getBody();
  SourceLocation BeginWhile = WS->getSourceRange().getBegin();
  SourceLocation EndWhile = getEndLocation(WS->getSourceRange().getEnd());
  SourceLocation EndCond =
      Body->getSourceRange().getBegin().getLocWithOffset(-1);

  std::string RevertWhile = getSourceText(SourceRange(BeginWhile, EndWhile));

  removeSourceText(SourceRange(BeginWhile, EndCond));
  writeToFile(OptionManager::InputFile);
  if (callOracle()) {
    Queue.push(Body);
  } else {
    TheRewriter.ReplaceText(SourceRange(BeginWhile, EndWhile), RevertWhile);
    writeToFile(OptionManager::InputFile);
    Queue.push(Body);
  }
}

void LocalReduction::reduceCompound(CompoundStmt *CS) {
  auto Stmts = getBodyStatements(CS);
  for (auto S : Stmts)
    Queue.push(S);
  DDElementVector Elements;
  Elements.resize(Stmts.size());
  std::transform(Stmts.begin(), Stmts.end(), Elements.begin(), CastElement);
  doDeltaDebugging(Elements);
}

void LocalReduction::reduceLabel(LabelStmt *LS) {
  if (CompoundStmt *CS = llvm::dyn_cast<CompoundStmt>(LS->getSubStmt())) {
    Queue.push(CS);
  }
}

bool LocalElementCollectionVisitor::VisitFunctionDecl(FunctionDecl *FD) {
  spdlog::get("Logger")->debug("Visit Function Decl: {}",
                               FD->getNameInfo().getAsString());
  if (FD->isThisDeclarationADefinition()) {
    Consumer->Functions.emplace_back(FD);
  }
  return true;
}

bool LocalElementCollectionVisitor::VisitDeclRefExpr(clang::DeclRefExpr *DRE) {
  Consumer->UseInfo[DRE->getDecl()].emplace_back(DRE);
  return true;
}
