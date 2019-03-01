#include "Transformation.h"

#include "SourceManager.h"

#include <queue>

#include "clang/Lex/Lexer.h"

void Transformation::Initialize(clang::ASTContext &C) {
  Context = &C;
  TheRewriter.setSourceMgr(Context->getSourceManager(), Context->getLangOpts());
}

std::vector<clang::Stmt *> Transformation::getAllChildren(clang::Stmt *S) {
  std::queue<clang::Stmt *> ToVisit;
  std::vector<clang::Stmt *> AllChildren;
  ToVisit.push(S);
  while (!ToVisit.empty()) {
    auto C = ToVisit.front();
    ToVisit.pop();
    AllChildren.emplace_back(C);
    for (auto const &Child : C->children()) {
      if (Child != NULL)
        ToVisit.push(Child);
    }
  }
  return AllChildren;
}

void Transformation::removeSourceText(const clang::SourceLocation &B,
                                      const clang::SourceLocation &E) {
  llvm::StringRef Text =
      SourceManager::GetSourceText(Context->getSourceManager(), B, E);
  std::string Replacement = "";
  for (auto const &chr : Text) {
    if (chr == '\n')
      Replacement += '\n';
    else if (isprint(chr))
      Replacement += " ";
    else
      Replacement += chr;
  }
  TheRewriter.ReplaceText(clang::SourceRange(B, E), Replacement);
}
