#ifndef INCLUDE_CURSOR_UTILS_H_
#define INCLUDE_CURSOR_UTILS_H_

#include <clang-c/Index.h>

#include <map>
#include <string>
#include <vector>

bool operator<(CXCursor const &lhs, CXCursor const &rhs);
bool operator==(CXCursor const &lhs, CXCursor const &rhs);
bool operator!=(CXCursor const &lhs, CXCursor const &rhs);
std::ostream &operator<<(std::ostream &, const CXCursor &);

class CursorUtils {
public:
  static void prettyPrintSet(std::vector<CXCursor> c);
  static std::vector<std::vector<CXCursor>> split(std::vector<CXCursor> vec,
                                                  int n);
  static bool contains(std::vector<CXCursor> vec, CXCursor cursor);
  static std::vector<CXCursor> subtract(std::vector<CXCursor> a,
                                        std::vector<CXCursor> b);
  static std::string getCursorKindName(CXCursor cursor);
  static std::string getCursorSpelling(CXCursor cursor);
  static int getStartLine(CXCursor cursor);
  static int getStartCol(CXCursor cursor);
  static int getEndLine(CXCursor cursor);
  static int getEndCol(CXCursor cursor);
  static bool isFunction(CXCursor cursor);
  static bool isField(CXCursor cursor);
  static bool isExpr(CXCursor cursor);
  static bool isStmt(CXCursor cursor);
  static bool isParm(CXCursor cursor);
  static bool isUnexposed(CXCursor cursor);
  static bool isLiteral(CXCursor cursor);
  static bool isExtraSemicolon(CXCursor cursor);
  static bool stmtConditions(CXCursor cursor);
};

#endif // INCLUDE_CURSOR_UTILS_H_
