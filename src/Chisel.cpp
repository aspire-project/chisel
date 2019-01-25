#include <string>

#include <spdlog/spdlog.h>
#include <spdlog/sinks/stdout_sinks.h>
#include <spdlog/sinks/basic_file_sink.h>

#include "DeadcodeElimination.h"
#include "FileManager.h"
#include "Frontend.h"
#include "GlobalReduction.h"
#include "LocalReduction.h"
#include "OptionManager.h"
#include "Profiler.h"
#include "Reduction.h"
#include "Reformat.h"
#include "Report.h"
#include "StatsManager.h"

void initialize() {
  FileManager::Initialize();

  auto ConsolSink = std::make_shared<spdlog::sinks::stdout_sink_mt>();
  auto FileSink = std::make_shared<spdlog::sinks::basic_file_sink_mt>(
      OptionManager::OutputDir + "/log.txt", true);
  auto Logger = std::make_shared<spdlog::logger>(
      "Logger", spdlog::sinks_init_list{ConsolSink, FileSink});
  ConsolSink->set_pattern("%v");
  if (OptionManager::Debug) {
    ConsolSink->set_level(spdlog::level::debug);
    FileSink->set_level(spdlog::level::debug);
    Logger->set_level(spdlog::level::debug);
  } else {
    ConsolSink->set_level(spdlog::level::info);
    FileSink->set_level(spdlog::level::info);
    Logger->set_level(spdlog::level::info);
  }
  spdlog::register_logger(Logger);
  Profiler::Initialize();
  spdlog::get("Logger")->info("Oracle: {}", OptionManager::OracleFile);
  spdlog::get("Logger")->info("Input: {}", OptionManager::InputFile);
  spdlog::get("Logger")->info("Output Directory: {}", OptionManager::OutputDir);
}

void finalize() {
  FileManager::Finalize();
  Profiler::Finalize();
}

int main(int argc, char *argv[]) {
  OptionManager::handleOptions(argc, argv);
  initialize();

  Profiler::GetInstance()->beginChisel();

  StatsManager::ComputeStats(OptionManager::InputFile);
  int wc0 = std::numeric_limits<int>::max();
  int wc = StatsManager::GetNumOfWords();

  if (OptionManager::Stat) {
    StatsManager::Print();
    return 0;
  }

  int Iteration = 0;
  while (wc < wc0) {
    Iteration++;
    spdlog::get("Logger")->info("Iteration {} (Word: {})", Iteration, wc);
    wc0 = wc;

    if (!OptionManager::SkipDCE) {
      DeadcodeElimination *DCE = new DeadcodeElimination();
      DCEFrontend::Parse(OptionManager::InputFile, DCE);
    }
    if (!OptionManager::SkipGlobal) {
      spdlog::get("Logger")->info("Start global reduction");
      Reduction *GR = new GlobalReduction();
      Frontend::Parse(OptionManager::InputFile, GR);
    }
    if (!OptionManager::SkipLocal) {
      spdlog::get("Logger")->info("Start local reduction");
      Reduction *LR = new LocalReduction();
      Frontend::Parse(OptionManager::InputFile, LR);
    }
    StatsManager::ComputeStats(OptionManager::InputFile);
    wc = StatsManager::GetNumOfWords();
  }

  Transformation *R = new Reformat();
  Frontend::Parse(OptionManager::InputFile, R);

  Profiler::GetInstance()->endChisel();

  Report::print();
  finalize();

  return 0;
}
