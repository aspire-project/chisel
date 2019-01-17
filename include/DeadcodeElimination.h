#ifndef DEADCODE_ELIMINATION_H
#define DEADCODE_ELIMINATION_H

#include <queue>
#include <vector>

#include "clang/AST/RecursiveASTVisitor.h"

#include "Reduction.h"

class DeadcodeElementCollectionVisitor;

/// \brief Represents a sound dead-code elimination phase
///
/// DeadcodeElimination removes unused statements without calling the oracle,
/// and removes unused labels and unused variable declarations that are side-effect free.
class DeadcodeElimination : public Transformation {
  friend class LocalElementCollectionVisitor;

public:
  DeadcodeElimination() : CollectionVisitor(NULL) {}
  ~DeadcodeElimination() { delete CollectionVisitor; }

  void removeUnusedElements();
  std::map<clang::Decl *, clang::SourceRange> LocationMapping;
  std::vector<clang::SourceLocation> UnusedLocations;

private:
  void Initialize(clang::ASTContext &Ctx);
  bool HandleTopLevelDecl(clang::DeclGroupRef D);
  clang::SourceRange getRemoveRange(clang::SourceLocation Loc);
  bool isConstant(clang::Stmt *S);

  DeadcodeElementCollectionVisitor *CollectionVisitor;
};

class DeadcodeElementCollectionVisitor
    : public clang::RecursiveASTVisitor<DeadcodeElementCollectionVisitor> {
public:
  DeadcodeElementCollectionVisitor(DeadcodeElimination *R) : Consumer(R) {}

  bool VisitVarDecl(clang::VarDecl *VD);
  bool VisitLabelStmt(clang::LabelStmt *LS);

private:
  DeadcodeElimination *Consumer;
};

class DCEFrontend {
public:
  static bool Parse(std::string &Filename, DeadcodeElimination *R);
};

#endif // DEADCODE_ELIMINATION_H
