#ifndef STATS_MANAGER_H
#define STATS_MANAGER_H

#include <string>
#include <vector>

#include "clang/AST/ASTConsumer.h"
#include "clang/AST/RecursiveASTVisitor.h"
#include "clang/AST/Stmt.h"

class StatsManager {
public:
  StatsManager() {}
  StatsManager(std::string &FileName);
  ~StatsManager() {}

  void print();
  int getNumOfWords() { return NumOfWords; }
  int getNumOfStatements() { return NumOfStatements; }
  int getNumOfFunctions() { return NumOfFunctions; }
  void increaseNumOfFunctions();
  void increaseNumOfStatements();


private:
  void computeStats();

  std::string FileName;
  int NumOfWords = 0;
  int NumOfStatements = 0;
  int NumOfFunctions = 0;
};

class StatsVisitor;

class StatsComputer : public clang::ASTConsumer {
  friend class StatsVisitor;

public:
  StatsComputer(StatsManager *S) : Manager(S), Visitor(NULL) {}
  ~StatsComputer() { delete Visitor; }

private:
  void Initialize(clang::ASTContext &Ctx);
  bool HandleTopLevelDecl(clang::DeclGroupRef D);

  StatsManager *Manager;
  StatsVisitor *Visitor;
};

class StatsVisitor : public clang::RecursiveASTVisitor<StatsVisitor> {
public:
  StatsVisitor(StatsManager *S) : Manager(S) {}

  bool VisitFunctionDecl(clang::FunctionDecl *FD);
  bool VisitStmt(clang::Stmt *S);

private:
  StatsManager *Manager;
};

#endif // STATS_MANAGER_H
