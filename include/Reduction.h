#ifndef REDUCTION_H
#define REDUCTION_H

#include <vector>

#include "Transformation.h"

using DDElement = llvm::PointerUnion<clang::Decl *, clang::Stmt *>;
using DDElementVector = std::vector<DDElement>;
using DDElementSet = std::set<DDElement>;

class Reduction : public Transformation {
public:
  Reduction() {}
  ~Reduction() {}

protected:
  virtual void Initialize(clang::ASTContext &Ctx);

  DDElementSet doDeltaDebugging(std::vector<DDElement> &Decls);

  virtual bool test(std::vector<DDElement> &ToBeRemoved) = 0;
  virtual bool callOracle();
  virtual std::vector<DDElementVector>
  refineChunks(std::vector<DDElementVector> &Chunks) = 0;

  DDElementSet toSet(DDElementVector &Vec);
  DDElementSet setDifference(DDElementSet &A, DDElementSet &B);

private:
  DDElementVector toVector(DDElementSet &Set);
  std::vector<DDElementVector> split(DDElementVector &Vec, int n);
};

#endif // REDUCTION_H
