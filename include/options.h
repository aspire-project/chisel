#ifndef INCLUDE_OPTIONS_H_
#define INCLUDE_OPTIONS_H_

#include <string>

class Option {
public:
  static std::string inputFile;
  static std::string outputFile;
  static std::string oracleFile;
  static std::string outputDir;
  static bool saveTemp;
  static bool decisionTree;
  static bool delayLearning;
  static bool skipGlobal;
  static bool skipLocal;
  static bool noCache;
  static bool globalDep;
  static bool localDep;
  static bool skipDCE;
  static bool profile;
  static bool verbose;
  static bool stat;

  static void showUsage();
  static void handleOptions(int argc, char *argv[]);
};

#endif // INCLUDE_OPTIONS_H_
