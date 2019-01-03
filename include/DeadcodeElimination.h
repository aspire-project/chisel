#ifndef DEADCODE_ELIMINATION_H
#define DEADCODE_ELIMINATION_H

#include <queue>
#include <vector>

#include "clang/AST/RecursiveASTVisitor.h"

#include "Reduction.h"

class DeadcodeElementCollectionVisitor;

class DeadcodeElimination : public Transformation {
  friend class LocalElementCollectionVisitor;

public:
  DeadcodeElimination() : CollectionVisitor(NULL) {}
  ~DeadcodeElimination() { delete CollectionVisitor; }

  std::map<clang::Decl *, std::vector<clang::DeclRefExpr *>> UseInfo;

private:
  void Initialize(clang::ASTContext &Ctx);
  bool HandleTopLevelDecl(clang::DeclGroupRef D);
  void HandleTranslationUnit(clang::ASTContext &Ctx);

  std::vector<clang::DeclRefExpr *> getDeclRefExprs(clang::Expr *E);
  void addDefUse(clang::DeclRefExpr *DRE, std::set<clang::Decl *> &DU,
                 std::set<clang::DeclRefExpr *> Cache);
  void removeUnusedVariables();
  bool isConstant(clang::Expr *E);

  DeadcodeElementCollectionVisitor *CollectionVisitor;
};

class DeadcodeElementCollectionVisitor
    : public clang::RecursiveASTVisitor<DeadcodeElementCollectionVisitor> {
public:
  DeadcodeElementCollectionVisitor(DeadcodeElimination *R) : Consumer(R) {}

  bool VisitDeclRefExpr(clang::DeclRefExpr *DRE);
  bool VisitVarDecl(clang::VarDecl *VD);

private:
  DeadcodeElimination *Consumer;
};

#endif // DEADCODE_ELIMINATION_H
