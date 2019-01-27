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
class DeadCodeElimination {
public:
  static void Run();
};

class ClangDeadcodeElimination : public Transformation {
  friend class LocalElementCollectionVisitor;

public:
  ClangDeadcodeElimination() : CollectionVisitor(NULL) {}
  ~ClangDeadcodeElimination() { delete CollectionVisitor; }

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
  DeadcodeElementCollectionVisitor(ClangDeadcodeElimination *R) : Consumer(R) {}

  bool VisitVarDecl(clang::VarDecl *VD);
  bool VisitLabelStmt(clang::LabelStmt *LS);

private:
  ClangDeadcodeElimination *Consumer;
};

class DCEFrontend {
public:
  static bool Parse(std::string &Filename, ClangDeadcodeElimination *R);
};

class BlockEliminationVisitor;

class BlockElimination : public Transformation {
  friend class BlockEliminationVisitor;

public:
  BlockElimination() : Visitor(NULL) {}
  ~BlockElimination() { delete Visitor; }

  void removeBlock(clang::CompoundStmt *CS);

private:
  void Initialize(clang::ASTContext &Ctx);
  bool HandleTopLevelDecl(clang::DeclGroupRef D);
  void HandleTranslationUnit(clang::ASTContext &Ctx);

  std::set<clang::Stmt *> FunctionBodies;

  BlockEliminationVisitor *Visitor;
};

class BlockEliminationVisitor
    : public clang::RecursiveASTVisitor<BlockEliminationVisitor> {
public:
  BlockEliminationVisitor(BlockElimination *R) : Consumer(R) {}

  bool VisitFunctionDecl(clang::FunctionDecl *FD);
  bool VisitCompoundStmt(clang::CompoundStmt *CS);

private:
  BlockElimination *Consumer;
};

#endif // DEADCODE_ELIMINATION_H
