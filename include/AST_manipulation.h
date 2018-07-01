#ifndef INCLUDE_AST_MANIPULATION_H_
#define INCLUDE_AST_MANIPULATION_H_

#include <clang-c/Index.h>

#include <vector>

class ASTManipulation {
public:
  static void removeElementsEfficient(const char *srcPath, const char *dstPath,
                                      std::vector<CXCursor> cursors);
  static void removeLineRange(const char *srcPath, const char *dstPath,
                              unsigned int startLine, unsigned int startCol,
                              unsigned int endLine, unsigned int endCol);
};

#endif // INCLUDE_AST_MANIPULATION_H_
