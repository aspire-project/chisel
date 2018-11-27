#ifndef GLOBAL_REDUCTION_H
#define GLOBAL_REDUCTION_H

#include <vector>

#include "clang/AST/RecursiveASTVisitor.h"

#include "Reduction.h"

class GlobalElementCollectionVisitor;

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
  bool test(std::vector<DDElement> &ToBeRemoved);
  std::vector<DDElementVector>
  refineChunks(std::vector<DDElementVector> &Chunks);

  std::vector<clang::Decl *> Decls;
  std::map<clang::Decl *, std::vector<clang::DeclRefExpr *>> UseInfo;

  GlobalElementCollectionVisitor *CollectionVisitor;
};

class GlobalElementCollectionVisitor
    : public clang::RecursiveASTVisitor<GlobalElementCollectionVisitor> {
public:
  GlobalElementCollectionVisitor(GlobalReduction *R) : Consumer(R) {}

  bool VisitDeclRefExpr(clang::DeclRefExpr *DRE);
  bool VisitEnumDecl(clang::EnumDecl *ED);
  bool VisitFunctionDecl(clang::FunctionDecl *FD);
  bool VisitRecordDecl(clang::RecordDecl *RD);
  bool VisitTypedefDecl(clang::TypedefDecl *TD);
  bool VisitVarDecl(clang::VarDecl *VD);

private:
  GlobalReduction *Consumer;

  void findAndInsert(clang::Decl *D, clang::DeclRefExpr *DRE);
};

#endif // GLOBAL_REDUCTION_H
