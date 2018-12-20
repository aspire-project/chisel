#include "OptionManager.h"

#include <getopt.h>
#include <string>

#include "llvm/Support/FileSystem.h"
#include "llvm/Support/Program.h"
#include "llvm/Support/raw_ostream.h"

const std::string usage_simple("Usage: chisel [OPTIONS]... ORACLE PROGRAM");
const std::string error_message("Try 'chisel --help' for more information.");

void OptionManager::showUsage() {
  llvm::errs()
      << usage_simple << "\n"
      << "Options:"
      << "\n"
      << "  --help                 Show this help message\n"
      << "  --output OUTPUT        De-bloated C file\n"
      << "  --output_dir OUTDIR    Output directory\n"
      << "  --save_temp            Save intermediate results\n"
      << "  --no_d_tree            Disable decision tree learning\n"
      << "  --no_delay_learning    Learn a new model for every iteration\n"
      << "  --skip_global          Skip global-level reduction\n"
      << "  --skip_local           Skip function-level reduction\n"
      << "  --no_cache             Do not cache intermediate results\n"
      << "  --skip_local_dep       Disable local dependency checking\n"
      << "  --skip_global_dep      Disable global dependency checking\n"
      << "  --skip_dce             Do not perform static unreachability "
         "analysis\n"
      << "  --no_profile           Do not print profiling report\n"
      << "  --debug                Print debug information\n"
      << "  --stat                 Count the number of statements\n";
}

static struct option long_options[] = {
    {"help", no_argument, 0, 'h'},
    {"output", required_argument, 0, 'o'},
    {"output_dir", required_argument, 0, 't'},
    {"save_temp", no_argument, 0, 's'},
    {"no_d_tree", no_argument, 0, 'D'},
    {"no_delay_learning", no_argument, 0, 'd'},
    {"skip_global", no_argument, 0, 'g'},
    {"skip_local", no_argument, 0, 'l'},
    {"no_cache", no_argument, 0, 'c'},
    {"skip_local_dep", no_argument, 0, 'L'},
    {"skip_global_dep", no_argument, 0, 'G'},
    {"skip_dce", no_argument, 0, 'C'},
    {"no_profile", no_argument, 0, 'p'},
    {"debug", no_argument, 0, 'v'},
    {"stat", no_argument, 0, 'S'},
    {0, 0, 0, 0}};

static const char *optstring = "ho:t:sDdglcLGCpvS";

std::string OptionManager::InputFile = "";
std::string OptionManager::OutputFile = "";
std::string OptionManager::OracleFile = "";
std::string OptionManager::OutputDir = "chisel-out";
bool OptionManager::SaveTemp = false;
bool OptionManager::DecisionTree = true;
bool OptionManager::DelayLearning = true;
bool OptionManager::SkipGlobal = false;
bool OptionManager::SkipLocal = false;
bool OptionManager::NoCache = false;
bool OptionManager::SkipGlobalDep = false;
bool OptionManager::SkipLocalDep = false;
bool OptionManager::SkipDCE = false;
bool OptionManager::Profile = true;
bool OptionManager::Debug = false;
bool OptionManager::Stat = false;

void OptionManager::handleOptions(int argc, char *argv[]) {
  char c;
  while ((c = getopt_long(argc, argv, optstring, long_options, 0)) != -1) {
    switch (c) {
    case 'h':
      showUsage();
      exit(0);

    case 'o':
      OptionManager::OutputFile = std::string(optarg);
      break;

    case 't':
      OptionManager::OutputDir = std::string(optarg);
      break;

    case 's':
      OptionManager::SaveTemp = true;
      break;

    case 'D':
      OptionManager::DecisionTree = false;
      break;

    case 'd':
      OptionManager::DelayLearning = false;
      break;

    case 'g':
      OptionManager::SkipGlobal = true;
      break;

    case 'l':
      OptionManager::SkipLocal = true;
      break;

    case 'c':
      OptionManager::NoCache = true;
      break;

    case 'L':
      OptionManager::SkipLocalDep = true;
      break;

    case 'G':
      OptionManager::SkipGlobalDep = true;
      break;

    case 'C':
      OptionManager::SkipDCE = true;
      break;

    case 'p':
      OptionManager::Profile = false;
      break;

    case 'v':
      OptionManager::Debug = true;
      break;

    case 'S':
      OptionManager::Stat = true;
      break;

    default:
      llvm::errs() << "Invalid option.\n";
      llvm::errs() << usage_simple << "\n";
      llvm::errs() << error_message << "\n";
      exit(1);
    }
  }

  if (optind + 2 > argc && !OptionManager::Stat) {
    llvm::errs() << "chisel: You must specify oracle and input.\n";
    llvm::errs() << usage_simple << "\n";
    llvm::errs() << error_message << "\n";
    exit(1);
  } else if (optind + 1 > argc && OptionManager::Stat) {
    llvm::errs() << "chisel: You must specify the input.\n";
    exit(1);
  }

  if (!OptionManager::Stat) {
    OptionManager::OracleFile = std::string(argv[optind]);
    OptionManager::InputFile = std::string(argv[optind + 1]);

    if (!llvm::sys::fs::exists(OptionManager::OracleFile)) {
      llvm::errs() << "The specified oracle file " << OptionManager::OracleFile
                   << " does not exist.\n";
      exit(1);
    } else if (!llvm::sys::fs::can_execute(OptionManager::OracleFile)) {
      llvm::errs() << "The specified oracle file " << OptionManager::OracleFile
                   << " is not executable.\n";
      exit(1);
    } else if (!llvm::sys::fs::exists(OptionManager::InputFile)) {
      llvm::errs() << "The specified input file " << OptionManager::InputFile
                   << " does not exist.\n";
      exit(1);
    } else if (llvm::sys::ExecuteAndWait(OptionManager::OracleFile,
                                         {OptionManager::OracleFile})) {
      llvm::errs() << "The specified oracle file " << OptionManager::OracleFile
                   << " cannot execute correctly.\n";
      exit(1);
    }

    if (OptionManager::OutputFile == "") {
      OptionManager::OutputFile = OptionManager::InputFile + ".chisel.c";
    }

    llvm::outs() << "Oracle: " << OptionManager::OracleFile << "\n";
    llvm::outs() << "Input: " << OptionManager::InputFile << "\n";
  } else {
    OptionManager::InputFile = std::string(argv[optind]);

    if (!llvm::sys::fs::exists(OptionManager::InputFile)) {
      llvm::errs() << "The specified input file " << OptionManager::InputFile
                   << " does not exist."
                   << "\n";
      exit(1);
    }
  }
}
