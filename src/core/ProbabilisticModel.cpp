#include "ProbabilisticModel.h"

#include <fstream>
#include <sstream>
#include <vector>

#include <mlpack/core.hpp>
#include <mlpack/methods/decision_tree/decision_tree.hpp>

#include "OptionManager.h"
#include "Profiler.h"

void ProbabilisticModel::initialize(DDElementVector &Source) {
  if (OptionManager::SkipLearning)
    return;
  clear();
  DDElementVector EmptyVector;
  addForTraining(Source, EmptyVector, true);
}

void ProbabilisticModel::clear() {
  if (OptionManager::SkipLearning)
    return;
  TrainingSet.reset();
  TrainingLabels.reset();
}

void ProbabilisticModel::train(int Iteration) {
  if (OptionManager::SkipLearning)
    return;
  Profiler::GetInstance()->beginLearning();
  bool ShouldTrain =
      !(Iteration > 100 && Iteration % (Iteration / 100 + 1) != 0);
  if ((!OptionManager::SkipDelayLearning && ShouldTrain) ||
      OptionManager::SkipDelayLearning) {
    arma::mat TransTrainingSet = trans(TrainingSet);
    MyDecisionTree.Train<>(TransTrainingSet, TrainingLabels, 2, 1);
  }
  Profiler::GetInstance()->endLearning();
}

void ProbabilisticModel::printVector(arma::mat FeatureVector, bool Label) {
  for (int i = 0; i < FeatureVector.n_cols; i++)
    llvm::outs() << "[" << FeatureVector[0, i] << "]";
  llvm::outs() << ": " << Label;
  llvm::outs() << "\n";
}

void ProbabilisticModel::addForTraining(DDElementVector &Source,
                                        DDElementVector &Chunk, bool Label) {
  if (OptionManager::SkipLearning)
    return;
  Profiler::GetInstance()->beginLearning();
  arma::mat Diff = createFeatureVector(Source, Chunk);
  TrainingSet = join_cols(TrainingSet, Diff);
  TrainingLabels.resize(TrainingLabels.n_elem + 1);
  TrainingLabels(TrainingLabels.n_elem - 1) = Label ? 0 : 1;
  Profiler::GetInstance()->endLearning();
}

arma::uvec
ProbabilisticModel::sortCandidates(DDElementVector &Source,
                                   std::vector<DDElementVector> &Chunks) {
  arma::uvec EmptyVector;
  if (OptionManager::SkipLearning)
    return EmptyVector;
  Profiler::GetInstance()->beginLearning();
  arma::mat Data, Probabilities;
  for (auto Chunk : Chunks)
    Data = join_cols(Data, createFeatureVector(Source, Chunk));
  arma::Row<size_t> Predictions;
  arma::mat TestData = trans(Data);
  MyDecisionTree.Classify<>(TestData, Predictions, Probabilities);
  arma::uvec SortedIndex = sort_index(Probabilities.row(0), "descend");
  if (Chunks.size() == Source.size() || Chunks.size() == Source.size() - 1)
    return SortedIndex;
  Profiler::GetInstance()->endLearning();
  return SortedIndex;
}

arma::mat ProbabilisticModel::createFeatureVector(DDElementVector &Source,
                                                  DDElementVector &Chunk) {
  if (Chunk.size() == 0)
    return arma::ones(1, Source.size());

  arma::mat Result = arma::zeros(1, Source.size());
  for (auto C : Chunk) {
    auto It = std::find(Source.begin(), Source.end(), C);
    if (It != std::end(Chunk)) {
      int Index = std::distance(Source.begin(), It);
      Result(0, Index) = 1;
    }
  }
  return Result;
}
