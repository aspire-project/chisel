#ifndef STATS_H
#define STATS_H

#include <vector>

class Stats {
public:
  static int getTokenCount(const char *srcPath);
  static int getWordCount(const char *srcPath);
  static std::vector<int> getStatementCount(const char *srcPath);
};

#endif // STATS_H
