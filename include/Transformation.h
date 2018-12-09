#ifndef TRANSFORMATION_H
#define TRANSFORMATION_H

#include "clang/AST/ASTConsumer.h"
#include "clang/AST/ASTContext.h"
#include "clang/AST/Expr.h"
#include "clang/Rewrite/Core/Rewriter.h"

class Transformation : public clang::ASTConsumer {
public:
  Transformation() {}
  ~Transformation() {}


protected:
  virtual void Initialize(clang::ASTContext &Ctx);

  clang::SourceLocation getEndOfStmt(clang::Stmt *S);
  clang::SourceLocation getEndLocation(clang::SourceLocation Loc);
  clang::SourceLocation getEndLocationAfter(clang::SourceRange Range,
                                            char Symbol);
  clang::SourceLocation getEndLocationUntil(clang::SourceRange Range,
                                            char Symbol);
  clang::SourceLocation getEndLocationFromBegin(clang::SourceRange Range);
  int getOffsetUntil(const char *Buf, char Symbol);
  clang::SourceLocation getDeclGroupRefEndLoc(clang::DeclGroupRef DGR);
  llvm::StringRef getSourceText(const clang::SourceLocation &B,
                                const clang::SourceLocation &E);

  void removeSourceText(const clang::SourceLocation &B,
                        const clang::SourceLocation &E);

  clang::ASTContext *Context;
  clang::Rewriter TheRewriter;
};

#endif // TRANSFORMATION_H
