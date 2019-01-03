#include "DeadcodeElimination.h"

#include "clang/Lex/Lexer.h"

#include "FileManager.h"
#include "OptionManager.h"

void DeadcodeElimination::Initialize(clang::ASTContext &Ctx) {
  Transformation::Initialize(Ctx);
  CollectionVisitor = new DeadcodeElementCollectionVisitor(this);
}

bool DeadcodeElimination::HandleTopLevelDecl(clang::DeclGroupRef D) {
  for (clang::DeclGroupRef::iterator I = D.begin(), E = D.end(); I != E; ++I) {
    CollectionVisitor->TraverseDecl(*I);
  }
  return true;
}

bool DeadcodeElimination::isConstant(clang::Expr *E) {
  if (clang::StringLiteral *L = llvm::dyn_cast<clang::StringLiteral>(E))
    return true;
  if (clang::IntegerLiteral *L = llvm::dyn_cast<clang::IntegerLiteral>(E))
    return true;
  if (clang::CharacterLiteral *L = llvm::dyn_cast<clang::CharacterLiteral>(E))
    return true;
  if (clang::CompoundLiteralExpr *L =
          llvm::dyn_cast<clang::CompoundLiteralExpr>(E))
    return true;
  if (clang::FloatingLiteral *L = llvm::dyn_cast<clang::FloatingLiteral>(E))
    return true;
  if (clang::ImaginaryLiteral *L = llvm::dyn_cast<clang::ImaginaryLiteral>(E))
    return true;
  return false;
}

void DeadcodeElimination::removeUnusedVariables() {
  for (auto entry : UseInfo) {
    if (clang::VarDecl *VD = llvm::dyn_cast<clang::VarDecl>(entry.first)) {
      if (VD->hasInit())
        if (!isConstant(VD->getInit()))
          continue;
      if (entry.second.size() == 0) {
        clang::SourceLocation Start = VD->getSourceRange().getBegin();
        clang::SourceLocation End =
            getEndLocationUntil(VD->getSourceRange(), ';');
        if (End.isInvalid())
          continue;
        removeSourceText(Start, End);
      }
    }
  }
  TheRewriter.overwriteChangedFiles();
}

void DeadcodeElimination::HandleTranslationUnit(clang::ASTContext &Ctx) {
  removeUnusedVariables();
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
