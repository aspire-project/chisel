#ifndef GLOBAL_REDUCTION_H
#define GLOBAL_REDUCTION_H

#include <vector>

#include "clang/AST/RecursiveASTVisitor.h"

#include "Reduction.h"

class GlobalElementCollectionVisitor;

/// \brief Represents a global reduction phase
///
/// In global reduction phase, global declarations are reduced.
class GlobalReduction : public Reduction {
  friend class GlobalElementCollectionVisitor;

public:
  GlobalReduction() : CollectionVisitor(NULL) {}
  ~GlobalReduction() { delete CollectionVisitor; }

private:
  void Initialize(clang::ASTContext &Ctx);
  bool HandleTopLevelDecl(clang::DeclGroupRef D);
  void HandleTranslationUnit(clang::ASTContext &Ctx);

  static DDElement CastElement(clang::Decl *D);

  bool callOracle();
  bool test(DDElementVector &ToBeRemoved);
  bool isInvalidChunk(DDElementVector &Chunk);
  void filterElements(DDElementVector &Vec);

  std::vector<clang::Decl *> Decls;
  std::map<clang::Decl *, bool> UseInfo;
  GlobalElementCollectionVisitor *CollectionVisitor;
};

class GlobalElementCollectionVisitor
    : public clang::RecursiveASTVisitor<GlobalElementCollectionVisitor> {
public:
  GlobalElementCollectionVisitor(GlobalReduction *R) : Consumer(R) {}

  bool VisitDeclRefExpr(clang::DeclRefExpr *DRE);
  bool VisitFunctionDecl(clang::FunctionDecl *FD);
  bool VisitVarDecl(clang::VarDecl *VD);
  bool VisitEmptyDecl(clang::EmptyDecl *ED);

private:
  GlobalReduction *Consumer;
};

#endif // GLOBAL_REDUCTION_H
