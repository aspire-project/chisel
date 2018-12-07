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

std::vector<DDElementVector> Reduction::split(DDElementVector &Vec, int n) {
  std::vector<DDElementVector> Result;
  int Length = static_cast<int>(Vec.size()) / n;
  int Remain = static_cast<int>(Vec.size()) % n;

  int Begin = 0, End = 0;
  for (int i = 0; i < std::min(n, static_cast<int>(Vec.size())); ++i) {
    End += (Remain > 0) ? (Length + !!(Remain--)) : Length;
    Result.emplace_back(
        DDElementVector(Vec.begin() + Begin, Vec.begin() + End));
    Begin = End;
  }
  return Result;
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

DDElementSet Reduction::doDeltaDebugging(DDElementVector &Decls) {
  DDElementVector Target = Decls;
  DDElementSet Removed;

  int n = 2;
  std::string FormatStr =
      "%" + std::to_string(std::to_string(Target.size()).length()) + "d";
  while (Target.size() >= 1) {
    llvm::outs() << "\rRunning delta debugging - Size: "
                 << llvm::format(FormatStr.c_str(), Target.size());
    auto Chunks = split(Target, n);
    bool ComplementSucceeding = false;

    for (auto Chunk : Chunks) {
      if (isInvalidChunk(Chunk))
        continue;

      if (test(Chunk)) {
        auto TargetSet = toSet(Target);
        auto ChunkSet = toSet(Chunk);
        auto Diff = setDifference(TargetSet, ChunkSet);
        Removed.insert(ChunkSet.begin(), ChunkSet.end());
        Target = toVector(Diff);
        n = std::max(n - 1, 2);
        ComplementSucceeding = true;
        break;
      }
    }

    if (!ComplementSucceeding) {
      if (n == Target.size()) {
        break;
      }
      n = std::min(n * 2, static_cast<int>(Target.size()));
    }
  }
  return Removed;
}
