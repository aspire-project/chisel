#ifndef LOCAL_REDUCTION_H
#define LOCAL_REDUCTION_H

#include <queue>
#include <vector>

#include "clang/AST/RecursiveASTVisitor.h"

#include "Reduction.h"

class LocalElementCollectionVisitor;

class LocalReduction : public Reduction {
  friend class LocalElementCollectionVisitor;

public:
  LocalReduction() : CollectionVisitor(NULL) {}
  ~LocalReduction() { delete CollectionVisitor; }

private:
  void Initialize(clang::ASTContext &Ctx);
  bool HandleTopLevelDecl(clang::DeclGroupRef D);
  void HandleTranslationUnit(clang::ASTContext &Ctx);

  std::set<clang::Stmt *> toSet(std::vector<clang::Stmt *> &Vec);
  std::set<clang::Stmt *> setDifference(std::set<clang::Stmt *> &A,
                                        std::set<clang::Stmt *> &B);
  static DDElement CastElement(clang::Stmt *S);

  bool callOracle();
  bool test(DDElementVector &ToBeRemoved);
  bool isInvalidChunk(DDElementVector &Chunk);
  void filterElements(std::vector<clang::Stmt *> &Vec);

  void doHierarchicalDeltaDebugging(clang::Stmt *S);
  void reduceIf(clang::IfStmt *IS);
  void reduceWhile(clang::WhileStmt *WS);
  void reduceFor(clang::ForStmt *FS);
  void reduceCompound(clang::CompoundStmt *CS);
  void reduceLabel(clang::LabelStmt *LS);
  int countReturnStmts(std::set<clang::Stmt *> &Elements);
  bool noReturn(std::set<clang::Stmt *> &FunctionStmts,
                std::set<clang::Stmt *> &AllRemovedStmts);
  bool danglingLabel(std::set<clang::Stmt *> &Remaining);
  bool brokenDependency(std::set<clang::Stmt *> &Remaining);

  std::vector<clang::DeclRefExpr *> getDeclRefExprs(clang::Expr *E);
  std::vector<clang::Stmt *> getBodyStatements(clang::CompoundStmt *CS);

  void addDefUse(clang::DeclRefExpr *DRE, std::set<clang::Decl *> &DU);

  std::vector<clang::FunctionDecl *> Functions;
  std::queue<clang::Stmt *> Queue;

  LocalElementCollectionVisitor *CollectionVisitor;
  clang::FunctionDecl *CurrentFunction;
};

class LocalElementCollectionVisitor
    : public clang::RecursiveASTVisitor<LocalElementCollectionVisitor> {
public:
  LocalElementCollectionVisitor(LocalReduction *R) : Consumer(R) {}

  bool VisitFunctionDecl(clang::FunctionDecl *FD);

private:
  LocalReduction *Consumer;
};

#endif // LOCAL_REDUCTION_H
