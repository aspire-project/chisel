#include "Reduction.h"

#include <algorithm>

#include "clang/Basic/SourceManager.h"
#include "llvm/Support/Program.h"

#include "OptionManager.h"
#include "Profiler.h"

void Reduction::Initialize(clang::ASTContext &C) {
  Transformation::Initialize(C);
}

bool Reduction::callOracle() {
  Profiler::GetInstance()->beginOracle();
  llvm::StringRef DevNull("/dev/null");
  llvm::Optional<llvm::StringRef> Redirects[] = {DevNull, DevNull, DevNull};
  int Status = llvm::sys::ExecuteAndWait(OptionManager::OracleFile,
                                         {OptionManager::OracleFile},
                                         llvm::None, Redirects);
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
  return Result;
}

DDElementSet Reduction::doDeltaDebugging(DDElementVector &Decls) {
  filterElements(Decls);

  DDElementSet Removed;
  DDElementVector DeclsCopy = Decls;

  int ChunkSize = (DeclsCopy.size() + 1) / 2;
  while (DeclsCopy.size() > 0) {
    std::string FormatStr =
        "%" + std::to_string(std::to_string(DeclsCopy.size()).length()) + "d";
    llvm::outs() << "\rRunning delta debugging - Size: "
                 << llvm::format(FormatStr.c_str(), DeclsCopy.size());

    bool Success = false;
    auto Targets = getCandidates(DeclsCopy, ChunkSize);
    for (auto Target : Targets) {
      if (std::find(Cache.begin(), Cache.end(), Target) != Cache.end() ||
          isInvalidChunk(Target))
        continue;

      if (!OptionManager::NoCache)
        Cache.insert(Target);

      bool Status = test(Target);
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
      ChunkSize = (DeclsCopy.size() + 1) / 2;
    } else {
      if (ChunkSize == 1)
        break;
      ChunkSize = (ChunkSize + 1) / 2;
    }
  }
  llvm::outs() << "\n";

  return Removed;
}
