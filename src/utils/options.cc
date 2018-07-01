#include <cstring>
#include <getopt.h>
#include <iostream>
#include <string>
#include <unistd.h>

#include "options.h"

const std::string usage_simple("Usage: chisel [OPTIONS]... ORACLE PROGRAM");
const std::string error_message("Try 'chisel --help' for more information.");

void Option::showUsage() {
  std::cerr << usage_simple << std::endl
            << "Options:" << std::endl
            << "  --help                 Show this help message" << std::endl
            << "  --output OUTPUT        De-bloated C file" << std::endl
            << "  --output_dir OUTDIR    Output directory" << std::endl
            << "  --save_temp            Save intermediate results" << std::endl
            << "  --no_d_tree            Disable decision tree learning"
            << std::endl
            << "  --no_delay_learning    Learn a new model for every iteration"
            << std::endl
            << "  --skip_global          Skip global-level reduction"
            << std::endl
            << "  --skip_local           Skip function-level reduction"
            << std::endl
            << "  --no_cache             Do not cache intermediate results"
            << std::endl
            << "  --no_local_dep         Disable local dependency checking"
            << std::endl
            << "  --no_global_dep        Disable global dependency checking"
            << std::endl
            << "  --skip_dce             Do not perform static unreachability "
               "analysis"
            << std::endl
            << "  --no_profile           Do not print profiling report"
            << std::endl
            << "  --verbose              Print output information" << std::endl
            << "  --stat                 Count the number of statements"
            << std::endl;
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
    {"no_local_dep", no_argument, 0, 'L'},
    {"no_global_dep", no_argument, 0, 'G'},
    {"skip_dce", no_argument, 0, 'C'},
    {"no_profile", no_argument, 0, 'p'},
    {"verbose", no_argument, 0, 'v'},
    {"stat", no_argument, 0, 'S'},
    {0, 0, 0, 0}};

static const char *optstring = "ho:t:sDdglcLGCpvS";

std::string Option::inputFile = "";
std::string Option::outputFile = "";
std::string Option::oracleFile = "";
std::string Option::outputDir = "chisel-out";
bool Option::saveTemp = false;
bool Option::decisionTree = true;
bool Option::delayLearning = true;
bool Option::skipGlobal = false;
bool Option::skipLocal = false;
bool Option::noCache = true;
bool Option::globalDep = true;
bool Option::localDep = true;
bool Option::skipDCE = false;
bool Option::profile = true;
bool Option::verbose = false;
bool Option::stat = false;

void Option::handleOptions(int argc, char *argv[]) {
  char c;
  while ((c = getopt_long(argc, argv, optstring, long_options, 0)) != -1) {
    switch (c) {
    case 'h':
      showUsage();
      exit(0);

    case 'o':
      Option::outputFile = std::string(optarg);
      break;

    case 't':
      Option::outputDir = std::string(optarg);
      break;

    case 's':
      Option::saveTemp = true;
      break;

    case 'D':
      Option::decisionTree = false;
      break;

    case 'd':
      Option::delayLearning = false;
      break;

    case 'g':
      Option::skipGlobal = true;
      break;

    case 'l':
      Option::skipLocal = true;
      break;

    case 'c':
      Option::noCache = true;
      break;

    case 'L':
      Option::localDep = false;
      break;

    case 'G':
      Option::globalDep = false;
      break;

    case 'C':
      Option::skipDCE = true;
      break;

    case 'p':
      Option::profile = false;
      break;

    case 'v':
      Option::verbose = true;
      break;

    case 'S':
      Option::stat = true;
      break;

    default:
      std::cerr << "Invalid option." << std::endl;
      std::cerr << usage_simple << std::endl;
      std::cerr << error_message << std::endl;
      exit(1);
    }
  }

  if (optind + 2 > argc && !Option::stat) {
    std::cerr << "chisel: You must specify oracle and input." << std::endl;
    std::cerr << usage_simple << std::endl;
    std::cerr << error_message << std::endl;
    exit(1);
  } else if (optind + 1 > argc && Option::stat) {
    std::cerr << "chisel: You must specify the input." << std::endl;
    exit(1);
  }

  if (!Option::stat) {
    Option::oracleFile = std::string(argv[optind]);
    Option::inputFile = std::string(argv[optind + 1]);

    if (access(Option::oracleFile.c_str(), F_OK) == -1) {
      std::cerr << "The specified oracle file " << Option::oracleFile
                << " does not exist." << std::endl;
      exit(1);
    } else if (access(Option::inputFile.c_str(), F_OK) == -1) {
      std::cerr << "The specified input file " << Option::inputFile
                << " does not exist." << std::endl;
      exit(1);
    }

    if (strcmp(Option::outputFile.c_str(), "") == 0) {
      Option::outputFile = Option::inputFile + ".chisel.c";
    }

    std::cout << "Oracle: " << Option::oracleFile << std::endl;
    std::cout << "Input: " << Option::inputFile << std::endl;
  } else {
    Option::inputFile = std::string(argv[optind]);

    if (access(Option::inputFile.c_str(), F_OK) == -1) {
      std::cerr << "The specified input file " << Option::inputFile
                << " does not exist." << std::endl;
      exit(1);
    }
  }
}
