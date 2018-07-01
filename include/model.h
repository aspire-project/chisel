#ifndef INCLUDE_MODEL_H_
#define INCLUDE_MODEL_H_

#include <clang-c/Index.h>
#include <iostream>
#include <mlpack/core.hpp>
#include <mlpack/methods/decision_tree/decision_tree.hpp>
#include <vector>

typedef double Feature;
typedef int Label;
typedef double Probability;

enum MethodKind {
  DecisionTree = 0,
};

class Model {
public:
  void init(MethodKind method);
  void clear();
  void recordTrainingData(arma::mat data, Label label);
  void addForTraining(std::vector<CXCursor> source,
                      std::vector<CXCursor> subset, unsigned int l);
  void addForPredicting(std::vector<CXCursor> source,
                        std::vector<std::vector<CXCursor>> subsets,
                        arma::mat &probabilities);
  void train();
  void predict(arma::mat testData, arma::mat &probabilities);

private:
  MethodKind _method;
  arma::mat trainingSet;
  arma::Row<size_t> trainingLabels;
  mlpack::tree::DecisionTree<> dt;
  arma::mat vectorMinus(std::vector<CXCursor> source,
                        std::vector<CXCursor> subset);
};

#endif // INCLUDE_MODEL_H_
