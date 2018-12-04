#include "DeadcodeElimination.h"

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

void DeadcodeElimination::Initialize(clang::ASTContext &Ctx) {
  Reduction::Initialize(Ctx);
  CollectionVisitor = new DeadcodeElementCollectionVisitor(this);
}

bool DeadcodeElimination::HandleTopLevelDecl(DeclGroupRef D) {
  for (DeclGroupRef::iterator I = D.begin(), E = D.end(); I != E; ++I) {
    CollectionVisitor->TraverseDecl(*I);
  }
  return true;
}

void DeadcodeElimination::removeUnusedVariables() {
  for (auto entry : UseInfo) {
    if (VarDecl *VD = llvm::dyn_cast<VarDecl>(entry.first)) {
      if (entry.second.size() == 0) {
        SourceLocation Start = VD->getSourceRange().getBegin();
        SourceLocation End = getEndLocation(VD->getSourceRange().getEnd());
        if (End.isInvalid())
          continue;
        removeSourceText(SourceRange(Start, End));
      }
    }
  }
  writeToFile(OptionManager::InputFile);
  FileManager::GetInstance()->updateBest();
}

void DeadcodeElimination::HandleTranslationUnit(clang::ASTContext &Ctx) {
  removeUnusedVariables();
}

DDElement DeadcodeElimination::CastElement(Stmt *S) { return S; }

clang::SourceLocation
DeadcodeElimination::getEndLocation(clang::SourceLocation Loc) {
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

bool DeadcodeElimination::test(DDElementVector &Element) { return true; }

std::vector<DDElementVector>
DeadcodeElimination::refineChunks(std::vector<DDElementVector> &Chunks) {
  std::vector<DDElementVector> result;
  return result;
}

bool DeadcodeElementCollectionVisitor::VisitDeclRefExpr(
    clang::DeclRefExpr *DRE) {
  Consumer->UseInfo[DRE->getDecl()].emplace_back(DRE);
  return true;
}

bool DeadcodeElementCollectionVisitor::VisitVarDecl(clang::VarDecl *VD) {
  if (clang::ParmVarDecl *PVD = llvm::dyn_cast<clang::ParmVarDecl>(VD))
    return true;
  std::vector<clang::DeclRefExpr *> Uses;
  Consumer->UseInfo.insert(std::make_pair(VD, Uses));
  return true;
}
