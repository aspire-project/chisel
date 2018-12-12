#ifndef OPTION_MANAGER_H
#define OPTION_MANAGER_H

#include <string>

class OptionManager {
public:
  static std::string InputFile;
  static std::string OutputFile;
  static std::string OracleFile;
  static std::string OutputDir;
  static bool SaveTemp;
  static bool SkipLearning;
  static bool SkipDelayLearning;
  static bool SkipGlobal;
  static bool SkipLocal;
  static bool NoCache;
  static bool SkipGlobalDep;
  static bool SkipLocalDep;
  static bool SkipDCE;
  static bool Profile;
  static bool Debug;
  static bool Stat;

  static void showUsage();
  static void handleOptions(int argc, char *argv[]);
};

#endif // OPTION_MANAGER_H
