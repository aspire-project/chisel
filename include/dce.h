#ifndef INCLUDE_DCE_H_
#define INCLUDE_DCE_H_

#include <clang-c/Index.h>

#include <vector>

class DCE {
public:
  static void removeDeadcode(const char *inputPath);
  static void removeVacuousElements(const char *);

private:
  static CXCursor findMoreGeneralCursor(CXCursor cursor);
  static std::vector<CXCursor> getUselessStatements(CXCursor rootCursor);
};

#endif // INCLUDE_DCE_H_
