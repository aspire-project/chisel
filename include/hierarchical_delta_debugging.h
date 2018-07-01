#ifndef INCLUDE_HIERARCHICAL_DELTA_DEBUGGING_H_
#define INCLUDE_HIERARCHICAL_DELTA_DEBUGGING_H_

#include <clang-c/Index.h>

#include <queue>
#include <vector>

#include "delta_debugging.h"

class HierarchicalDeltaDebugging {
public:
  void globalReduction(const char *srcPath, const char *dstPath);
  void localReduction(const char *srcPath, const char *dstPath);
  void fin();
  void init(const char *inputPath);

private:
  DeltaDebugging dd;
  std::queue<CXCursor> q;
  const int SUCCESS = 0;
  const int FAIL = 1;
  void clearQueue(std::queue<CXCursor> &q);
  void reductionMessage(std::string kind, unsigned int size);
  void addToQueue(std::vector<CXCursor> cursors);
  std::vector<CXCursor> getElementChildren(CXCursor cursor);
  void reduceIf(CXCursor cursor, const char *srcPath, const char *dstPath);
  void reduceWhile(CXCursor cursor, const char *srcPath, const char *dstPath);
  void reduceSwitch(CXCursor cursor, const char *srcPath, const char *dstPath);
  void reduceLabel(CXCursor cursor, const char *srcPath, const char *dstPath);
  void reduceCaseOrDefault(CXCursor cursor, const char *srcPath,
                           const char *dstPath);
  void reduceBlock(CXCursor cursor, const char *srcPath, const char *dstPath);
  void hierarchicalDD(CXCursor cursor, const char *srcPath,
                      const char *dstPath);
};

#endif // INCLUDE_HIERARCHICAL_DELTA_DEBUGGING_H_
