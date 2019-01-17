#ifndef PROBABILISTIC_MODEL_H_
#define PROBABILISTIC_MODEL_H_

#include <vector>

#include "clang/AST/ASTContext.h"

#include <mlpack/core.hpp>
#include <mlpack/methods/decision_tree/decision_tree.hpp>

using DDElement = llvm::PointerUnion<clang::Decl *, clang::Stmt *>;
using DDElementVector = std::vector<DDElement>;

/// \brief Represents the probabilistic model that is utilized in the reduction phase
class ProbabilisticModel {
public:
  ProbabilisticModel() : MyDecisionTree(2) {}
  void initialize(DDElementVector &Source);
  void clear();
  void train(int Iteration);
  void addForTraining(DDElementVector &Source, DDElementVector &Chunk,
                      bool Label);
  arma::uvec sortCandidates(DDElementVector &Source,
                            std::vector<DDElementVector> &Chunks);

private:
  arma::mat TrainingSet;
  arma::Row<size_t> TrainingLabels;
  mlpack::tree::DecisionTree<> MyDecisionTree;

  arma::mat createFeatureVector(DDElementVector &Source,
                                DDElementVector &Chunk);
  void printVector(arma::mat FeatureVector, bool Label);
};

#endif // PROBABILISTIC_MODEL_H_
