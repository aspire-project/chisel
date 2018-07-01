#include <clang-c/Index.h>

#include <vector>

class Visitor {
public:
  CXCursor getFunction(std::vector<CXCursor> vecs, CXCursor cursor);
  std::vector<CXCursor> getAllChildren(CXCursor rootCursor);
  std::vector<CXCursor> getImmediateChildren(CXCursor root);
  std::vector<CXCursor> getAllCursors();
  std::vector<CXCursor> getGlobalElements();
  std::vector<CXCursor> getFunctionBodies();
  void setAllCursors(std::vector<CXCursor> newVec);
  void setGlobalElements(std::vector<CXCursor> newVec);
  void setFunctionBodies(std::vector<CXCursor> newVec);
  void clear();
  void init(const char *inputPath);
  void fin();

private:
  CXIndex indx;
  CXTranslationUnit tu;
};
