#include "StatsManager.h"

#include <fstream>

#include "clang/AST/Expr.h"

#include "Frontend.h"

int StatsManager::NumOfWords = 0;
int StatsManager::NumOfStatements = 0;
int StatsManager::NumOfFunctions = 0;

void StatsManager::ComputeStats(std::string &FileName) {
  NumOfWords = 0;
  NumOfStatements = 0;
  NumOfFunctions = 0;
  StatsComputer *S = new StatsComputer();
  Frontend::Parse(FileName, S);

  std::ifstream IFS(FileName.c_str());
  std::string Word;
  while (IFS >> Word) {
    NumOfWords++;
  }
}

void StatsManager::Print() {
  llvm::outs() << "# Functions  : " << NumOfFunctions << "\n";
  llvm::outs() << "# Statements : " << NumOfStatements << "\n";
}

void StatsManager::IncreaseNumOfFunctions() { NumOfFunctions++; }

void StatsManager::IncreaseNumOfStatements() { NumOfStatements++; }

void StatsComputer::Initialize(clang::ASTContext &Ctx) {
  Visitor = new StatsVisitor();
}

bool StatsComputer::HandleTopLevelDecl(clang::DeclGroupRef D) {
  for (clang::DeclGroupRef::iterator I = D.begin(), E = D.end(); I != E; ++I) {
    Visitor->TraverseDecl(*I);
  }
  return true;
}

bool StatsVisitor::VisitFunctionDecl(clang::FunctionDecl *FD) {
  StatsManager::IncreaseNumOfFunctions();
  return true;
}

bool StatsVisitor::VisitStmt(clang::Stmt *S) {
  if (clang::Expr *E = llvm::dyn_cast<clang::Expr>(S)) {
    if (clang::BinaryOperator *BO = llvm::dyn_cast<clang::BinaryOperator>(E))
      if (BO->isAssignmentOp())
        StatsManager::IncreaseNumOfStatements();
  } else if (clang::CompoundStmt *C = llvm::dyn_cast<clang::CompoundStmt>(S)) {
  } else {
    StatsManager::IncreaseNumOfStatements();
  }
  return true;
}
