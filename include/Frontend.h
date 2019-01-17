#ifndef FRONTEND_H
#define FRONTEND_H

#include "clang/Parse/ParseAST.h"

#include <string>

/// \brief Provides an independent frontend for any action on AST
class Frontend {
public:
  static bool Parse(std::string &FileName, clang::ASTConsumer *C);
};

#endif // FRONTEND_H
