#include <clang-c/Index.h>
#include <stdlib.h>

#include <algorithm>
#include <cstring>
#include <fstream>
#include <iostream>
#include <iterator>
#include <limits>
#include <map>
#include <mlpack/core.hpp>
#include <mlpack/methods/decision_tree/decision_tree.hpp>
#include <string>
#include <unordered_set>
#include <vector>

#include "AST_manipulation.h"
#include "cursor_utils.h"
#include "model.h"
#include "options.h"
#include "report.h"
#include "rules.h"

#include "delta_debugging.h"

std::vector<std::vector<CXCursor>> cache;

std::vector<CXCursor> DeltaDebugging::getImmediateChildren(CXCursor root) {
  return visitor.getImmediateChildren(root);
}

std::vector<CXCursor> DeltaDebugging::getAllCursors() {
  return visitor.getAllCursors();
}

std::vector<CXCursor> DeltaDebugging::getGlobalElements() {
  return visitor.getGlobalElements();
}

std::vector<CXCursor> DeltaDebugging::getFunctionBodies() {
  return visitor.getFunctionBodies();
}

void DeltaDebugging::removeFromAllCursors(std::vector<CXCursor> toBeRemoved,
                                          bool deepRemove) {
  std::vector<CXCursor> children;
  if (deepRemove) {
    for (auto c : toBeRemoved) {
      std::vector<CXCursor> temp = visitor.getAllChildren(c);
      children.insert(children.end(), temp.begin(), temp.end());
    }
  }
  children.insert(children.end(), toBeRemoved.begin(), toBeRemoved.end());
  for (auto x : children) {
    auto temp = visitor.getAllCursors();
    temp.erase(std::remove(temp.begin(), temp.end(), x), temp.end());
    visitor.setAllCursors(temp);
  }
}

bool DeltaDebugging::test(std::vector<CXCursor> toBeRemoved,
                          const char *srcPath, const char *dstPath,
                          LEVEL level) {
  if (level == GLOBAL)
    Report::globalCallsCounter.increment();
  else
    Report::localCallsCounter.increment();
  ASTManipulation::removeElementsEfficient(srcPath, dstPath, toBeRemoved);
  std::string cmd = std::string(Option::oracleFile) + " 2> /dev/null";
  if (Option::profile)
    Report::oracleProfiler.startTimer();
  bool status = system(cmd.c_str());
  if (Option::profile)
    Report::oracleProfiler.stopTimer();
  if (status == SUCCESS) {
    if (level == GLOBAL)
      Report::successfulGlobalCallsCounter.increment();
    else
      Report::successfulLocalCallsCounter.increment();

    std::string bestNow = Option::outputDir + "/best_now.c";
    if (Option::saveTemp) {
      std::string cmd = std::string("cp ") + dstPath + " " + Option::outputDir +
                        "/" + dstPath + "." +
                        std::to_string(Report::localCallsCounter.count() +
                                       Report::globalCallsCounter.count()) +
                        "." + std::to_string(level) + ".success.c";
      system(cmd.c_str());
    }
    cmd = std::string("cp ") + dstPath + " " + bestNow.c_str();
    system(cmd.c_str());
  } else {
    if (Option::saveTemp) {
      std::string cmd = std::string("cp ") + dstPath + " " + Option::outputDir +
                        "/" + dstPath + "." +
                        std::to_string(Report::localCallsCounter.count() +
                                       Report::globalCallsCounter.count()) +
                        "." + std::to_string(level) + ".fail.c";
      system(cmd.c_str());
    }
  }
  return status;
}

bool DeltaDebugging::cacheContains(std::vector<std::vector<CXCursor>> cache,
                                   std::vector<CXCursor> vec) {
  for (auto c : cache) {
    if (c == vec)
      return true;
  }
  return false;
}

std::vector<std::vector<CXCursor>>
DeltaDebugging::filterSubsets(std::vector<std::vector<CXCursor>> subsets,
                              LEVEL level) {
  // Removing redundant subsets
  std::vector<std::vector<CXCursor>> finalSubsets;
  std::vector<CXCursor> tempAllCursors, children;
  for (auto subset : subsets) {
    tempAllCursors = visitor.getAllCursors();
    children.clear();
    for (auto c : subset) {
      std::vector<CXCursor> temp = visitor.getAllChildren(c);
      children.insert(children.end(), temp.begin(), temp.end());
    }
    children.insert(children.end(), subset.begin(), subset.end());
    tempAllCursors = CursorUtils::subtract(tempAllCursors, children);

    bool isInCache = false;
    if (!Option::noCache)
      isInCache = cacheContains(cache, tempAllCursors);
    auto f = visitor.getFunction(visitor.getAllCursors(), subset[0]);
    if (!((Option::globalDep && level == GLOBAL && Rule::mainExists(subset)) ||
          (isInCache && !Option::noCache) ||
          ((Option::globalDep || Option::localDep) &&
           Rule::isTypeDependencyBroken(tempAllCursors, subset)) ||
          (Option::localDep && level > GLOBAL &&
           Rule::isUninitialized(tempAllCursors)) ||
          (Option::localDep && level > GLOBAL &&
           !Rule::containsOneReturn(children, f, visitor.getAllChildren(f))))) {
      finalSubsets.emplace_back(subset);
    }
  }
  return finalSubsets;
}

void DeltaDebugging::ddmin(std::vector<CXCursor> source, const char *srcPath,
                           const char *dstPath, LEVEL level) {
  int status = FAIL;
  if (source.size() >= 1) {
    std::vector<CXCursor> source_ = source;
    int n = 2;
    std::vector<CXCursor> complement, children;
    std::vector<CXCursor> tempGlobalElements, tempFunctionBodies,
        tempAllCursors;
    if (Option::decisionTree) {
      model.addForTraining(source, source, SUCCESS);
    }
    int iteration = 0;
    while (static_cast<int>(source_.size()) >= 2) {
      std::vector<std::vector<CXCursor>> subsets =
          CursorUtils::split(source_, n);
      bool someComplementSucceeding = false;
      arma::uvec order;
      arma::mat probs;

      subsets = filterSubsets(subsets, level);

      if (Option::decisionTree) {
        if (Option::profile)
          Report::learningProfiler.startTimer();
        model.addForPredicting(source, subsets, probs);
        if (Option::profile)
          Report::learningProfiler.stopTimer();
        order = sort_index(probs.row(0), "descend");
      }

      for (int i = 0; i < subsets.size(); i++) {
        iteration++;
        std::vector<CXCursor> subset;
        if (Option::decisionTree) {
          subset = subsets[order(i)];
        } else {
          subset = subsets[i];
        }

        status = FAIL;
        tempAllCursors = visitor.getAllCursors();
        tempGlobalElements = visitor.getGlobalElements();
        tempFunctionBodies = visitor.getFunctionBodies();
        children.clear();
        for (auto c : subset) {
          std::vector<CXCursor> temp = visitor.getAllChildren(c);
          children.insert(children.end(), temp.begin(), temp.end());
        }
        children.insert(children.end(), subset.begin(), subset.end());
        tempAllCursors = CursorUtils::subtract(tempAllCursors, children);

        if (level == GLOBAL) {
          tempGlobalElements =
              CursorUtils::subtract(tempGlobalElements, children);
          tempFunctionBodies =
              CursorUtils::subtract(tempFunctionBodies, children);
        }

        status = test(subset, srcPath, dstPath, level);
        if (!Option::noCache)
          cache.emplace_back(tempAllCursors);
        if (Option::decisionTree) {
          model.addForTraining(source, subset, status);
          if (Option::delayLearning) {
            if (!(iteration > 100 && iteration % (iteration / 100 + 1) != 0)) {
              if (Option::profile)
                Report::learningProfiler.startTimer();
              model.train();
              if (Option::profile)
                Report::learningProfiler.stopTimer();
            }
          } else {
            if (Option::profile)
              Report::learningProfiler.startTimer();
            model.train();
            if (Option::profile)
              Report::learningProfiler.stopTimer();
          }
        }

        if (status == SUCCESS) {
          visitor.setAllCursors(tempAllCursors);
          if (level == GLOBAL) {
            visitor.setGlobalElements(tempGlobalElements);
            visitor.setFunctionBodies(tempFunctionBodies);
          }

          source_ = CursorUtils::subtract(source_, subset);
          n = std::max(n - 1, 2);
          someComplementSucceeding = true;
          break;
        }
      }

      if (!someComplementSucceeding) {
        if (n == static_cast<int>(source_.size()))
          break;

        n = std::min(n * 2, static_cast<int>(source_.size()));
      }
    }
  }
  if (Option::decisionTree) {
    model.clear();
  }
}

void DeltaDebugging::fin() { visitor.fin(); }

void DeltaDebugging::init(const char *inputPath) {
  visitor.clear();
  cache.clear();

  visitor.init(inputPath);
}
