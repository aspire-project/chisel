#include <string>

#include <spdlog/sinks/stdout_sinks.h>
#include <spdlog/spdlog.h>

#include "clang/Basic/Diagnostic.h"
#include "clang/Basic/TargetInfo.h"
#include "clang/Frontend/CompilerInstance.h"
#include "clang/Lex/Preprocessor.h"
#include "clang/Parse/ParseAST.h"

//#include "dce.h"
#include "FileManager.h"
#include "GlobalReduction.h"
#include "LocalReduction.h"
#include "OptionManager.h"
#include "Profiler.h"
#include "Reduction.h"
#include "Report.h"
#include "Stats.h"

void stat() {
  /*  auto stats = Stats::getStatementCount(OptionManager::InputFile.c_str());
    std::cout << "# Functions  : " << stats[1] << std::endl;
    std::cout << "# Statements : " << stats[0] << std::endl;
  */
  exit(0);
}

void initialize() {
  auto Logger = spdlog::stdout_logger_mt("Logger");
  if (OptionManager::Verbose) {
    Logger->set_level(spdlog::level::debug);
  } else {
    Logger->set_level(spdlog::level::info);
  }

  FileManager::Initialize();
  Profiler::Initialize();

}

void finalize() {
  FileManager::Finalize();
  Profiler::Finalize();
}

bool run(Reduction *R) {
  std::unique_ptr<clang::CompilerInstance> CI(new clang::CompilerInstance);
  CI->createDiagnostics();
  clang::TargetOptions &TO = CI->getTargetOpts();
  TO.Triple = llvm::sys::getDefaultTargetTriple();
  clang::TargetInfo *Target = clang::TargetInfo::CreateTargetInfo(
      CI->getDiagnostics(), CI->getInvocation().TargetOpts);
  CI->setTarget(Target);

  CI->createFileManager();
  CI->createSourceManager(CI->getFileManager());
  CI->createPreprocessor(clang::TU_Complete);
  CI->createASTContext();

  CI->setASTConsumer(std::unique_ptr<clang::ASTConsumer>(R));
  clang::Preprocessor &PP = CI->getPreprocessor();
  PP.getBuiltinInfo().initializeBuiltins(PP.getIdentifierTable(),
                                         PP.getLangOpts());

  if (!CI->InitializeSourceManager(clang::FrontendInputFile(
          OptionManager::InputFile, clang::InputKind::C))) {
    return false;
  }

  CI->createSema(clang::TU_Complete, 0);
  clang::DiagnosticsEngine &Diag = CI->getDiagnostics();
  Diag.setSuppressAllDiagnostics(true);
  Diag.setIgnoreAllWarnings(true);

  ParseAST(CI->getSema());

  CI->getDiagnosticClient().EndSourceFile();

  return true;
}

int main(int argc, char *argv[]) {
  OptionManager::handleOptions(argc, argv);
  initialize();

  Profiler::GetInstance()->beginChisel();

  int wc0 = std::numeric_limits<int>::max();
  int wc = Stats::getWordCount(OptionManager::InputFile.c_str());

  clang::CompilerInstance *CI;
  while (wc < wc0) {
    wc0 = wc;
    if (!OptionManager::SkipGlobal) {
      Reduction *GR = new GlobalReduction();
      run(GR);
    }
    if (!OptionManager::SkipLocal) {
      Reduction *LR = new LocalReduction();
      run(LR);
    }
    wc = Stats::getWordCount(OptionManager::InputFile.c_str());
    //    wc0 = Stats::getWordCount(bestNow.c_str());
    //    if (!OptionManager::skipDCE) {
    //      DCE::removeDeadcode(bestNow.c_str());
    //    }

    //    wc = Stats::getWordCount(bestNow.c_str());
  }
  //  DCE::removeVacuousElements(bestNow.c_str());
  Profiler::GetInstance()->endChisel();

  Report::print();
  finalize();

  return 0;
}
