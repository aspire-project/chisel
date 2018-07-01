#ifndef INCLUDE_STATS_H_
#define INCLUDE_STATS_H_

#include <clang-c/Index.h>

#include <algorithm>
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <iterator>
#include <string>
#include <vector>

#include "stats.h"

class Stats {
public:
  static int getTokenCount(const char *srcPath);
  static int getWordCount(const char *srcPath);
  static std::vector<int> getStatementCount(const char *srcPath);
};

#endif // INCLUDE_STATS_H_
