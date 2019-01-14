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
  TrainingSet = arma::mat(Source.size(), 1, arma::fill::ones);
  TrainingLabels = arma::Row<size_t>(1, arma::fill::zeros);
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
    MyDecisionTree.Train(TrainingSet, TrainingLabels, 2, 1);
  }
  Profiler::GetInstance()->endLearning();
}

void ProbabilisticModel::addForTraining(DDElementVector &Source,
                                        DDElementVector &Chunk, bool Label) {
  if (OptionManager::SkipLearning)
    return;
  Profiler::GetInstance()->beginLearning();
  arma::mat Diff = createFeatureVector(Source, Chunk);
  TrainingSet.resize(TrainingSet.n_rows, TrainingSet.n_cols + 1);
  TrainingSet.col(TrainingSet.n_cols - 1) = Diff;
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
  if (Source.size() == 0 || Chunks.size() == 0)
    return EmptyVector;
  Profiler::GetInstance()->beginLearning();
  arma::mat Probabilities;
  arma::Row<size_t> Predictions;

  arma::mat Data(Source.size(), Chunks.size(), arma::fill::ones);
  for (int I = 0; I < Chunks.size(); I++)
    Data.col(I) = createFeatureVector(Source, Chunks[I]);
  MyDecisionTree.Classify(Data, Predictions, Probabilities);
  arma::uvec SortedIndex = sort_index(Probabilities.row(0), "descend");
  if (Chunks.size() == Source.size() || Chunks.size() == Source.size() - 1)
    return SortedIndex;
  Profiler::GetInstance()->endLearning();
  return SortedIndex;
}

arma::mat ProbabilisticModel::createFeatureVector(DDElementVector &Source,
                                                  DDElementVector &Chunk) {
  if (Chunk.size() == 0)
    return arma::ones(Source.size(), 1);

  arma::mat Result(Source.size(), 1, arma::fill::zeros);
  for (auto C : Chunk) {
    if (C.isNull())
      continue;
    auto It = std::find(Source.begin(), Source.end(), C);
    if (It != std::end(Source)) {
      int Index = std::distance(Source.begin(), It);
      Result(Index, 0) = 1;
    }
  }
  return Result;
}
