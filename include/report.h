#ifndef INCLUDE_REPORT_H_
#define INCLUDE_REPORT_H_

#include "counting.h"
#include "profiling.h"

class Report {
public:
  static Profiler totalProfiler;
  static Profiler learningProfiler;
  static Profiler oracleProfiler;
  static Counter globalCallsCounter;
  static Counter localCallsCounter;
  static Counter successfulGlobalCallsCounter;
  static Counter successfulLocalCallsCounter;
  static void print();
};

#endif // INCLUDE_REPORT_H_
