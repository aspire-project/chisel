#include "report.h"
#include "counting.h"
#include "options.h"
#include "profiling.h"
#include "stats.h"

Profiler Report::totalProfiler;
Profiler Report::learningProfiler;
Profiler Report::oracleProfiler;
Counter Report::globalCallsCounter;
Counter Report::localCallsCounter;
Counter Report::successfulGlobalCallsCounter;
Counter Report::successfulLocalCallsCounter;

void Report::print() {
  std::cout << "========================================\n";
  std::cout << "                 Report                 \n";
  std::cout << "========================================\n";
  auto stmt = Stats::getStatementCount(Option::inputFile.c_str());
  std::cout << "Original Size: " << stmt[0] << " statements" << std::endl;
  stmt = Stats::getStatementCount(Option::outputFile.c_str());
  std::cout << "Reduced Size: " << stmt[0] << " statements" << std::endl;
  if (!Option::skipGlobal)
    std::cout << "Global Success Ratio: "
              << successfulGlobalCallsCounter.count() << "/"
              << globalCallsCounter.count() << std::endl;
  if (!Option::skipLocal)
    std::cout << "Local Success Ratio: " << successfulLocalCallsCounter.count()
              << "/" << localCallsCounter.count() << std::endl;
  if (Option::decisionTree)
    std::cout << "Learning Time: " << learningProfiler.getElapsedTime() << " s"
              << std::endl;
  std::cout << "Oracle Time: " << oracleProfiler.getElapsedTime() << " s"
            << std::endl;
  std::cout << "Total Time: " << totalProfiler.getElapsedTime() << " s"
            << std::endl;
}
