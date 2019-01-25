#include "Reduction.h"

#include <algorithm>
#include <spdlog/spdlog.h>

#include "clang/Basic/SourceManager.h"
#include "llvm/Support/Program.h"

#include "OptionManager.h"
#include "Profiler.h"

void Reduction::Initialize(clang::ASTContext &C) {
  Transformation::Initialize(C);
}

bool Reduction::callOracle() {
  Profiler::GetInstance()->beginOracle();
  int Status = llvm::sys::ExecuteAndWait(OptionManager::OracleFile,
                                         {OptionManager::OracleFile});
  Profiler::GetInstance()->endOracle();
  return (Status == 0);
}

DDElementSet Reduction::toSet(DDElementVector &Vec) {
  DDElementSet S(Vec.begin(), Vec.end());
  return S;
}

DDElementSet Reduction::setDifference(DDElementSet &A, DDElementSet &B) {
  DDElementSet Result;
  std::set_difference(A.begin(), A.end(), B.begin(), B.end(),
                      std::inserter(Result, Result.begin()));
  return Result;
}

DDElementVector Reduction::toVector(DDElementSet &Set) {
  DDElementVector Vec(Set.begin(), Set.end());
  return Vec;
}

std::vector<DDElementVector> Reduction::getCandidates(DDElementVector &Decls,
                                                      int ChunkSize) {
  if (Decls.size() == 1)
    return {Decls};
  std::vector<DDElementVector> Result;
  int Partitions = Decls.size() / ChunkSize;
  for (int Idx = 0; Idx < Partitions; Idx++) {
    DDElementVector Target;
    Target.insert(Target.end(), Decls.begin() + Idx * ChunkSize,
                  Decls.begin() + (Idx + 1) * ChunkSize);
    if (Target.size() > 0)
      Result.emplace_back(Target);
  }
  for (int Idx = 0; Idx < Partitions; Idx++) {
    DDElementVector Complement;
    Complement.insert(Complement.end(), Decls.begin(),
                      Decls.begin() + Idx * ChunkSize);
    Complement.insert(Complement.end(), Decls.begin() + (Idx + 1) * ChunkSize,
                      Decls.end());
    if (Complement.size() > 0)
      Result.emplace_back(Complement);
  }

  if (OptionManager::SkipLearning)
    return Result;
  else {
    arma::uvec ChunkOrder = TheModel.sortCandidates(Decls, Result);
    std::vector<DDElementVector> SortedResult;
    for (int I = 0; I < Result.size(); I++)
      if (ChunkOrder[I] != -1)
        SortedResult.emplace_back(Result[ChunkOrder[I]]);
    return SortedResult;
  }
}

DDElementSet Reduction::doDeltaDebugging(DDElementVector &Decls) {
  Cache.clear();
  DDElementSet Removed;
  DDElementVector DeclsCopy = Decls;

  TheModel.initialize(Decls);

  int ChunkSize = (DeclsCopy.size() + 1) / 2;
  int Iteration = 0;
  spdlog::get("Logger")->info("Running delta debugging - Size: {}",
                              DeclsCopy.size());

  while (DeclsCopy.size() > 0) {
    bool Success = false;
    TheModel.train(Iteration);
    auto Targets = getCandidates(DeclsCopy, ChunkSize);
    for (auto Target : Targets) {
      Iteration++;
      if (std::find(Cache.begin(), Cache.end(), Target) != Cache.end() ||
          isInvalidChunk(Target))
        continue;

      if (!OptionManager::NoCache)
        Cache.insert(Target);

      bool Status = test(Target);
      TheModel.addForTraining(Decls, Target, Status);
      if (Status) {
        auto TargetSet = toSet(Target);
        Removed.insert(TargetSet.begin(), TargetSet.end());
        for (auto T : Target)
          DeclsCopy.erase(std::remove(DeclsCopy.begin(), DeclsCopy.end(), T),
                          DeclsCopy.end());
        Success = true;
        break;
      }
    }
    if (Success) {
      spdlog::get("Logger")->info("Reduced - Size: {}", DeclsCopy.size());
      ChunkSize = (DeclsCopy.size() + 1) / 2;
    } else {
      if (ChunkSize == 1)
        break;
      ChunkSize = (ChunkSize + 1) / 2;
    }
  }
  return Removed;
}
