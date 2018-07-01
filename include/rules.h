#ifndef INCLUDE_RULES_H_
#define INCLUDE_RULES_H_

#include <clang-c/Index.h>

#include <map>
#include <vector>

class Rule {
public:
  static bool mainExists(std::vector<CXCursor> cursors);
  static bool isTypeDependencyBroken(std::vector<CXCursor> allCursors,
                                     std::vector<CXCursor> cursors);
  static bool isUninitialized(std::vector<CXCursor> allCursors);
  static bool containsOneReturn(std::vector<CXCursor> subset, CXCursor func,
                                std::vector<CXCursor> allFunctionElems);
};

#endif // INCLUDE_RULES_H_
