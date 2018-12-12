#ifndef REDUCTION_H
#define REDUCTION_H

#include <vector>

#include "ProbabilisticModel.h"
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

  virtual bool test(DDElementVector &ToBeRemoved) = 0;
  virtual bool callOracle();
  virtual bool isInvalidChunk(DDElementVector &Chunk) = 0;

  DDElementSet toSet(DDElementVector &Vec);
  DDElementSet setDifference(DDElementSet &A, DDElementSet &B);

private:
  ProbabilisticModel TheModel;
  std::set<DDElementVector> Cache;
  DDElementVector toVector(DDElementSet &Set);

  std::vector<DDElementVector> getCandidates(DDElementVector &Decls,
                                             int ChunkSize);
};

#endif // REDUCTION_H
