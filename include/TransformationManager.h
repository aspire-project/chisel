#include "clang/AST/ASTContext.h"
#include "clang/AST/RecursiveASTVisitor.h"

class TransformationManager {
public:
  static TransformationManager *GetInstance();
  bool initializeCompilerInstance(std::string &ErrorMsg);
  bool doTransformation(std::string &ErrorMsg, int &ErrorCode);
private:
  TransformationManager();
  ~TransformationManager();
};
