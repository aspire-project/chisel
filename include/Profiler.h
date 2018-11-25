#ifndef PROFILER_H
#define PROFILER_H

#include "llvm/Support/Timer.h"

class Profiler {
public:
  static void Initialize();
  static Profiler *GetInstance();
  static void Finalize();

  void incrementGlobalReductionCounter();
  void incrementSuccessfulGlobalReductionCounter();
  void incrementLocalReductionCounter();
  void incrementSuccessfulLocalReductionCounter();

  int getGlobalReductionCounter() { return GlobalReductionCounter; }
  int getSuccessfulGlobalReductionCounter() {
    return SuccessfulGlobalReductionCounter;
  }
  int getLocalReductionCounter() { return LocalReductionCounter; }
  int getSuccessfulLocalReductionCounter() {
    return SuccessfulLocalReductionCounter;
  }

  llvm::Timer &getChiselTimer() { return ChiselTimer; }
  llvm::Timer &getLearningTimer() { return LearningTimer; }
  llvm::Timer &getOracleTimer() { return OracleTimer; }

  llvm::TimeRecord &getChiselTimeRecord() { return ChiselTimeRecord; }
  llvm::TimeRecord &getLearningTimeRecord() { return LearningTimeRecord; }
  llvm::TimeRecord &getOracleTimeRecord() { return OracleTimeRecord; }

  void beginChisel();
  void endChisel();

  void beginOracle();
  void endOracle();

private:
  Profiler() {}
  ~Profiler() {}

  static Profiler *Instance;

  int GlobalReductionCounter = 0;
  int SuccessfulGlobalReductionCounter = 0;
  int LocalReductionCounter = 0;
  int SuccessfulLocalReductionCounter = 0;

  llvm::TimeRecord ChiselTimeRecord;
  llvm::TimeRecord LearningTimeRecord;
  llvm::TimeRecord OracleTimeRecord;

  llvm::Timer ChiselTimer;
  llvm::Timer LearningTimer;
  llvm::Timer OracleTimer;
};

#endif // PROFILER_H
