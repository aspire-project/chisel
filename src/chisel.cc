#include <clang-c/Index.h>
#include <stdlib.h>
#include <sys/stat.h>

#include <iostream>
#include <string>

#include "dce.h"
#include "hierarchical_delta_debugging.h"
#include "options.h"
#include "report.h"
#include "stats.h"

void stat() {
  auto stats = Stats::getStatementCount(Option::inputFile.c_str());
  std::cout << "# Functions  : " << stats[1] << std::endl;
  std::cout << "# Statements : " << stats[0] << std::endl;
  exit(0);
}

int main(int argc, char *argv[]) {
  Option::handleOptions(argc, argv);
  std::string bestNow = Option::outputDir + "/best_now.c";
  std::string orig = Option::outputDir + "/orig.c";

  if (Option::stat)
    stat();
  mkdir(Option::outputDir.c_str(), ACCESSPERMS);

  HierarchicalDeltaDebugging reducer;
  std::string cmd = "cp " + Option::inputFile + " " + bestNow;
  system(cmd.c_str());
  cmd = "cp " + Option::inputFile + " " + orig;
  system(cmd.c_str());

  if (Option::profile)
    Report::totalProfiler.startTimer();
  int wc0 = 0, wc = 0;
  while (true) {
    reducer.init(bestNow.c_str());
    wc0 = Stats::getWordCount(bestNow.c_str());
    if (!Option::skipDCE) {
      DCE::removeDeadcode(bestNow.c_str());
    }
    if (!Option::skipGlobal) {
      reducer.globalReduction(bestNow.c_str(), Option::inputFile.c_str());
    }
    if (!Option::skipLocal) {
      reducer.localReduction(bestNow.c_str(), Option::inputFile.c_str());
    }

    wc = Stats::getWordCount(bestNow.c_str());
    if (wc == wc0) {
      break;
    }
  }
  if (Option::profile)
    Report::totalProfiler.stopTimer();

  DCE::removeVacuousElements(bestNow.c_str());

  std::string bestClean = Option::outputDir + "/best_clean.c";
  cmd = "awk NF " + bestNow + " > " + bestClean;
  system(cmd.c_str());
  cmd = "mv " + bestClean + " " + bestNow;
  system(cmd.c_str());
  reducer.fin();

  cmd = "cp " + bestNow + " " + Option::outputFile;
  system(cmd.c_str());
  remove(Option::inputFile.c_str());
  cmd = "cp " + orig + " " + Option::inputFile;
  system(cmd.c_str());

  if (Option::profile)
    Report::print();

  return 0;
}
