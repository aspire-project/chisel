#include "Profiler.h"

#include "llvm/Support/Timer.h"

Profiler *Profiler::Instance;

void Profiler::Initialize() {
  Instance = new Profiler();

  Instance->ChiselTimer.init("ChiselTimer", "");
  Instance->OracleTimer.init("OracleTimer", "");
  Instance->LearningTimer.init("LearningTimer", "");
}

Profiler *Profiler::GetInstance() {
  if (!Instance)
    Initialize();
  return Instance;
}

void Profiler::Finalize() { delete Instance; }

void Profiler::incrementGlobalReductionCounter() { GlobalReductionCounter++; }

void Profiler::incrementSuccessfulGlobalReductionCounter() {
  SuccessfulGlobalReductionCounter++;
}

void Profiler::incrementLocalReductionCounter() { LocalReductionCounter++; }

void Profiler::incrementSuccessfulLocalReductionCounter() {
  SuccessfulLocalReductionCounter++;
}

void Profiler::beginChisel() {
  assert(ChiselTimer.isInitialized());
  ChiselTimer.startTimer();
}

void Profiler::endChisel() {
  assert(ChiselTimer.isRunning());
  ChiselTimer.stopTimer();
  ChiselTimeRecord += ChiselTimer.getTotalTime();
  ChiselTimer.clear();
}

void Profiler::beginOracle() {
  assert(OracleTimer.isInitialized());
  OracleTimer.startTimer();
}

void Profiler::endOracle() {
  assert(OracleTimer.isRunning());
  OracleTimer.stopTimer();
  OracleTimeRecord += OracleTimer.getTotalTime();
  OracleTimer.clear();
}

void Profiler::beginLearning() {
  assert(LearningTimer.isInitialized());
  LearningTimer.startTimer();
}

void Profiler::endLearning() {
  assert(LearningTimer.isRunning());
  LearningTimer.stopTimer();
  LearningTimeRecord += LearningTimer.getTotalTime();
  LearningTimer.clear();
}
