#ifndef REFORMAT_H
#define REFORMAT_H

#include "clang/AST/RecursiveASTVisitor.h"
#include "clang/Format/Format.h"

#include "Transformation.h"

class Reformat : public Transformation {
public:
  Reformat() {}
  ~Reformat() {}

private:
  void Initialize(clang::ASTContext &Ctx);
  void HandleTranslationUnit(clang::ASTContext &Ctx);
  void doReformatting(std::string FileName, clang::format::FormatStyle FS);
  std::vector<clang::tooling::Range> createRanges(llvm::MemoryBuffer *Code);
};

#endif // REFORMAT_H
