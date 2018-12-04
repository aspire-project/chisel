#include "Report.h"

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



  llvm::outs() << Bar << "\n"
               << llvm::center_justify("Report", TotalWidth) << "\n"
               << Bar << "\n"
               << llvm::right_justify("Total Time :", LeftColWidth)
               << llvm::format(RightColFmt,
                               Prof->getChiselTimeRecord().getWallTime())
               << " s\n"
               << llvm::right_justify("Oracle Time :", LeftColWidth)
               << llvm::format(RightColFmt,
                               Prof->getOracleTimeRecord().getWallTime())
               << " s\n";
  if (OptionManager::DecisionTree) {
    llvm::outs() << llvm::right_justify("Learning Time :", LeftColWidth)
                 << llvm::format(RightColFmt,
                                 Prof->getLearningTimeRecord().getWallTime())
                 << " s\n";
  }
  if (!OptionManager::SkipGlobal) {
    int Ratio = Prof->getGlobalReductionCounter()
                    ? Prof->getSuccessfulGlobalReductionCounter() * 100 /
                          Prof->getGlobalReductionCounter()
                    : 0;
    llvm::outs() << llvm::right_justify("Global Success Ratio :", LeftColWidth)
                 << llvm::format("%5d%%", Ratio) << "  "
                 << llvm::format("(%5d",
                                 Prof->getSuccessfulGlobalReductionCounter())
                 << " / "
                 << llvm::format("%5d)", Prof->getGlobalReductionCounter())
                 << "\n";
  }
  if (!OptionManager::SkipLocal) {
    int Ratio = Prof->getLocalReductionCounter()
                    ? Prof->getSuccessfulLocalReductionCounter() * 100 /
                          Prof->getLocalReductionCounter()
                    : 0;
    llvm::outs() << llvm::right_justify("Local Success Ratio :", LeftColWidth)
                 << llvm::format("%5d%%", Ratio) << "  "
                 << llvm::format("(%5d",
                                 Prof->getSuccessfulLocalReductionCounter())
                 << " / "
                 << llvm::format("%5d)", Prof->getLocalReductionCounter())
                 << "\n";
  }
  StatsManager::ComputeStats(FileManager::GetInstance()->getOriginFilePath());
  llvm::outs() << llvm::right_justify("Original #Functions :", LeftColWidth)
               << llvm::format("%23d", StatsManager::GetNumOfFunctions())
               << "\n";
  llvm::outs() << llvm::right_justify("Original #Statemets :", LeftColWidth)
               << llvm::format("%23d", StatsManager::GetNumOfStatements())
               << "\n";
  StatsManager::ComputeStats(FileManager::GetInstance()->getBestFilePath());
  llvm::outs() << llvm::right_justify("Reduced #Functions :", LeftColWidth)
               << llvm::format("%23d", StatsManager::GetNumOfFunctions())
               << "\n";
  llvm::outs() << llvm::right_justify("Reduced #Statements :", LeftColWidth)
               << llvm::format("%23d", StatsManager::GetNumOfStatements())
               << "\n";
}
