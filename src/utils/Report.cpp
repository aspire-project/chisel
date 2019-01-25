#include "Report.h"

#include <spdlog/spdlog.h>

#include "llvm/Support/Format.h"
#include "llvm/Support/raw_ostream.h"

#include "FileManager.h"
#include "OptionManager.h"
#include "Profiler.h"
#include "StatsManager.h"

void Report::print() {
  Profiler *Prof = Profiler::GetInstance();

  int TotalWidth = 52;
  int LeftColWidth = 25;
  const char *RightColFmt = "%21.1f";
  std::string Bar(TotalWidth, '=');

  spdlog::get("Logger")->info(Bar);
  spdlog::get("Logger")->info("{:^52}", "Report");
  spdlog::get("Logger")->info(Bar);
  spdlog::get("Logger")->info("{:>25}{:>21.1f}s", "Total Time :",
                              Prof->getChiselTimeRecord().getWallTime());
  spdlog::get("Logger")->info("{:>25}{:>21.1f}s", "Oracle Time :",
                              Prof->getOracleTimeRecord().getWallTime());
  if (!OptionManager::SkipLearning) {
    spdlog::get("Logger")->info("{:>25}{:>21.1f}s", "Learning Time :",
                                Prof->getLearningTimeRecord().getWallTime());
  }
  if (!OptionManager::SkipGlobal) {
    int Ratio = Prof->getGlobalReductionCounter()
                    ? Prof->getSuccessfulGlobalReductionCounter() * 100 /
                          Prof->getGlobalReductionCounter()
                    : 0;
    spdlog::get("Logger")->info("{:>25}{:>5}% ({:>5} / {:>5})",
                                "Global Success Ratio :", Ratio,
                                Prof->getSuccessfulGlobalReductionCounter(),
                                Prof->getGlobalReductionCounter());
  }
  if (!OptionManager::SkipLocal) {
    int Ratio = Prof->getLocalReductionCounter()
                    ? Prof->getSuccessfulLocalReductionCounter() * 100 /
                          Prof->getLocalReductionCounter()
                    : 0;
    spdlog::get("Logger")->info("{:>25}{:>5}% ({:>5} / {:>5})",
                                "Local Success Ratio :", Ratio,
                                Prof->getSuccessfulLocalReductionCounter(),
                                Prof->getLocalReductionCounter());
  }
  StatsManager::ComputeStats(FileManager::GetInstance()->getOriginFilePath());
  spdlog::get("Logger")->info("{:>25}{:>22}", "#Functions (Original) :",
                              StatsManager::GetNumOfFunctions());
  spdlog::get("Logger")->info("{:>25}{:>22}", "#Statements (Original) :",
                              StatsManager::GetNumOfStatements());
  StatsManager::ComputeStats(OptionManager::InputFile);
  spdlog::get("Logger")->info("{:>25}{:>22}", "#Functions (Reduced) :",
                              StatsManager::GetNumOfFunctions());
  spdlog::get("Logger")->info("{:>25}{:>22}", "#Statements (Reduced) :",
                              StatsManager::GetNumOfStatements());
}
