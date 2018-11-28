#include "StatsManager.h"

#include <fstream>

#include "clang/AST/Expr.h"

#include "Frontend.h"

StatsManager::StatsManager(std::string &F) {
  FileName = F;
  computeStats();
}

void StatsManager::computeStats() {
  NumOfWords = 0;
  NumOfStatements = 0;
  NumOfFunctions = 0;
  StatsComputer *S = new StatsComputer(this);
  Frontend::Parse(FileName, S);

  std::ifstream IFS(FileName.c_str());
  std::string Word;
  while (IFS >> Word) {
    NumOfWords++;
  }
}

void StatsManager::print() {
  llvm::outs() << "# Functions  : " << NumOfFunctions << "\n";
  llvm::outs() << "# Statements : " << NumOfStatements << "\n";
}

void StatsManager::increaseNumOfFunctions() { NumOfFunctions++; }

void StatsManager::increaseNumOfStatements() { NumOfStatements++; }

void StatsComputer::Initialize(clang::ASTContext &Ctx) {
  Visitor = new StatsVisitor(Manager);
}

bool StatsComputer::HandleTopLevelDecl(clang::DeclGroupRef D) {
  for (clang::DeclGroupRef::iterator I = D.begin(), E = D.end(); I != E; ++I) {
    Visitor->TraverseDecl(*I);
  }
  return true;
}

bool StatsVisitor::VisitFunctionDecl(clang::FunctionDecl *FD) {
  Manager->increaseNumOfFunctions();
  return true;
}

bool StatsVisitor::VisitStmt(clang::Stmt *S) {
  if (clang::Expr *E = llvm::dyn_cast<clang::Expr>(S)) {
    if (clang::BinaryOperator *BO = llvm::dyn_cast<clang::BinaryOperator>(E))
      if (BO->isAssignmentOp())
        Manager->increaseNumOfStatements();
  } else if (clang::CompoundStmt *C = llvm::dyn_cast<clang::CompoundStmt>(S)) {
  } else {
    Manager->increaseNumOfStatements();
  }
  return true;
}
