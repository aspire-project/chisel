#include "Report.h"

#include "llvm/Support/Format.h"
#include "llvm/Support/raw_ostream.h"

#include "OptionManager.h"
#include "Profiler.h"
#include "Stats.h"

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
    int Ratio = Prof->getSuccessfulGlobalReductionCounter() * 100 /
                Prof->getGlobalReductionCounter();
    llvm::outs() << llvm::right_justify("Global Success Ratio :", LeftColWidth)
                 << llvm::format("%5d%%", Ratio)
                 << "  "
                 << llvm::format("(%5d",
                                 Prof->getSuccessfulGlobalReductionCounter())
                 << " / "
                 << llvm::format("%5d)", Prof->getGlobalReductionCounter())
                 << "\n";
  }
  if (!OptionManager::SkipLocal) {
    int Ratio = Prof->getSuccessfulLocalReductionCounter() * 100 /
                Prof->getLocalReductionCounter();
    llvm::outs() << llvm::right_justify("Local Success Ratio :", LeftColWidth)
                 << llvm::format("%5d%%", Ratio)
                 << "  "
                 << llvm::format("(%5d",
                                 Prof->getSuccessfulLocalReductionCounter())
                 << " / "
                 << llvm::format("%5d)", Prof->getLocalReductionCounter())
                 << "\n";
  }
  /*  auto stmt = Stats::getStatementCount(OptionManager::inputFile.c_str());
    std::cout << "Original Size: " << stmt[0] << " statements" << std::endl;
    stmt = Stats::getStatementCount(OptionManager::outputFile.c_str());
    std::cout << "Reduced Size: " << stmt[0] << " statements" << std::endl;
  */
}
