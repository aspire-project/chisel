#ifndef INCLUDE_DELTA_DEBUGGING_H_
#define INCLUDE_DELTA_DEBUGGING_H_

#include <clang-c/Index.h>

#include <vector>

#include "model.h"
#include "report.h"
#include "visitor.h"

enum LEVEL { GLOBAL = 0, LOCAL = 1 };

enum STATUS { SUCCESS = 0, FAIL = 1 };

class DeltaDebugging {
public:
  void ddmin(std::vector<CXCursor> source, const char *srcPath,
             const char *dstPath, LEVEL level);
  void init(const char *inputPath);
  void fin();
  std::vector<CXCursor> getGlobalElements();
  std::vector<CXCursor> getFunctionBodies();
  std::vector<CXCursor> getImmediateChildren(CXCursor rootCursor);
  std::vector<CXCursor> getAllCursors();
  bool test(std::vector<CXCursor> toBeRemoved, const char *srcPath,
            const char *dstPath, LEVEL level);
  void removeFromAllCursors(std::vector<CXCursor> toBeRemoved, bool deepRemove);

private:
  Model model;
  Visitor visitor;
  bool cacheContains(std::vector<std::vector<CXCursor>> cache,
                     std::vector<CXCursor> vec);
  std::vector<std::vector<CXCursor>>
  filterSubsets(std::vector<std::vector<CXCursor>> subsets, LEVEL level);
};

#endif // INCLUDE_DELTA_DEBUGGING_H_
