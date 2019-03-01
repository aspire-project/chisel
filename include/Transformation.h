#ifndef TRANSFORMATION_H
#define TRANSFORMATION_H

#include "clang/AST/ASTConsumer.h"
#include "clang/AST/ASTContext.h"
#include "clang/AST/Expr.h"
#include "clang/Rewrite/Core/Rewriter.h"

/// \brief Represesnts a transformation action on an AST
class Transformation : public clang::ASTConsumer {
public:
  Transformation() {}
  ~Transformation() {}

protected:
  virtual void Initialize(clang::ASTContext &Ctx);

  std::vector<clang::Stmt *> getAllChildren(clang::Stmt *S);

  void removeSourceText(const clang::SourceLocation &B,
                        const clang::SourceLocation &E);

  clang::ASTContext *Context;
  clang::Rewriter TheRewriter;
};

#endif // TRANSFORMATION_H
