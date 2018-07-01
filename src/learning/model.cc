#include <fstream>
#include <iostream>
#include <mlpack/core.hpp>
#include <mlpack/methods/decision_tree/decision_tree.hpp>
#include <sstream>
#include <vector>

#include "cursor_utils.h"
#include "model.h"

void Model::init(MethodKind method) { _method = method; }

void Model::clear() {
  trainingSet.reset();
  trainingLabels.reset();
}

void Model::recordTrainingData(arma::mat data, Label label) {
  trainingSet = join_cols(trainingSet, data);
  trainingLabels.resize(trainingLabels.n_elem + 1);
  trainingLabels(trainingLabels.n_elem - 1) = label;
}

void Model::train() {
  // mlpack requires transposing the matrix
  arma::mat transTrainingSet = trans(trainingSet);
  dt.Train<>(transTrainingSet, trainingLabels, 2, 1);
}

void Model::predict(arma::mat testData, arma::mat &probabilities) {
  arma::Row<size_t> dummyRow;
  dt.Classify<>((testData), dummyRow, probabilities);
}

arma::mat Model::vectorMinus(std::vector<CXCursor> source,
                             std::vector<CXCursor> subset) {
  arma::mat result(1, source.size());
  int i = 0;
  for (std::vector<CXCursor>::iterator it = source.begin(); it != source.end();
       ++it) {
    if (!CursorUtils::contains(subset, *it)) {
      result(0, i) = 0;
    } else {
      result(0, i) = 1;
    }
    i++;
  }
  return result;
}

void Model::addForTraining(std::vector<CXCursor> source,
                           std::vector<CXCursor> subset, unsigned int l) {
  arma::mat vm = vectorMinus(source, subset);
  recordTrainingData(vm, l);
}

void Model::addForPredicting(std::vector<CXCursor> source,
                             std::vector<std::vector<CXCursor>> subsets,
                             arma::mat &probabilities) {
  arma::mat data;
  for (auto subset : subsets)
    data = join_cols(data, vectorMinus(source, subset));
  predict(trans(data), probabilities);
}
