#include "Reduction.h"

#include <algorithm>

#include "clang/Basic/SourceManager.h"
#include "clang/Lex/Lexer.h"
#include "llvm/Support/Program.h"

#include "OptionManager.h"
#include "Profiler.h"

void Reduction::Initialize(clang::ASTContext &C) {
  Context = &C;
  TheRewriter.setSourceMgr(Context->getSourceManager(), Context->getLangOpts());
}

void Reduction::writeToFile(std::string Filename) {
  std::error_code error_code;
  llvm::raw_fd_ostream outFile(Filename, error_code, llvm::sys::fs::F_None);
  TheRewriter.getEditBuffer(Context->getSourceManager().getMainFileID())
      .write(outFile);
  outFile.close();
}

bool Reduction::callOracle() {
  Profiler::GetInstance()->beginOracle();
  int Status = std::system(OptionManager::OracleFile.c_str());
  Profiler::GetInstance()->endOracle();
  return (Status == 0);
}

clang::SourceLocation
Reduction::getDeclGroupRefEndLoc(clang::DeclGroupRef DGR) {
  clang::Decl *LastD;

  if (DGR.isSingleDecl()) {
    LastD = DGR.getSingleDecl();
  } else {
    clang::DeclGroupRef::iterator E = DGR.end();
    --E;
    LastD = (*E);
  }

  clang::SourceRange Range = LastD->getSourceRange();
  clang::SourceLocation EndLoc = getEndLocationFromBegin(Range);
  return EndLoc;
}

clang::SourceLocation Reduction::getDeclStmtEndLoc(clang::DeclStmt *DS) {
  clang::DeclGroupRef DGR = DS->getDeclGroup();
  return getDeclGroupRefEndLoc(DGR);
}

clang::SourceLocation
Reduction::getEndLocationFromBegin(clang::SourceRange Range) {
  clang::SourceLocation StartLoc = Range.getBegin();
  clang::SourceLocation EndLoc = Range.getEnd();
  if (StartLoc.isInvalid())
    return StartLoc;
  if (EndLoc.isInvalid())
    return EndLoc;

  if (StartLoc.isMacroID())
    StartLoc = Context->getSourceManager().getFileLoc(StartLoc);
  if (EndLoc.isMacroID())
    EndLoc = Context->getSourceManager().getFileLoc(EndLoc);

  clang::SourceRange NewRange(StartLoc, EndLoc);
  int LocRangeSize = TheRewriter.getRangeSize(NewRange);
  if (LocRangeSize == -1)
    return NewRange.getEnd();

  return StartLoc.getLocWithOffset(LocRangeSize);
}

int Reduction::getOffsetUntil(const char *Buf, char Symbol) {
  int Offset = 0;
  while (*Buf != Symbol) {
    Buf++;
    if (*Buf == '\0')
      break;
    Offset++;
  }
  return Offset;
}

clang::SourceLocation Reduction::getEndLocationAfter(clang::SourceRange Range,
                                                     char Symbol) {
  clang::SourceLocation EndLoc = getEndLocationFromBegin(Range);
  if (EndLoc.isInvalid())
    return EndLoc;

  const char *EndBuf = Context->getSourceManager().getCharacterData(EndLoc);
  int Offset = getOffsetUntil(EndBuf, Symbol) + 1;
  return EndLoc.getLocWithOffset(Offset);
}

clang::SourceLocation Reduction::getEndLocationUntil(clang::SourceRange Range,
                                                     char Symbol) {
  clang::SourceLocation EndLoc = getEndLocationFromBegin(Range);
  if (EndLoc.isInvalid())
    return EndLoc;

  const char *EndBuf = Context->getSourceManager().getCharacterData(EndLoc);
  int Offset = getOffsetUntil(EndBuf, Symbol);
  return EndLoc.getLocWithOffset(Offset);
}

std::string Reduction::getSourceText(clang::SourceRange SR) {
  const clang::SourceManager *SM = &Context->getSourceManager();
  llvm::StringRef ref = clang::Lexer::getSourceText(
      clang::CharSourceRange::getCharRange(SR), *SM, clang::LangOptions());
  return ref.str();
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

void Reduction::doDeltaDebugging(DDElementVector &Decls) {
  DDElementVector Target = Decls;
  int n = 2;
  while (Target.size() >= 1) {
    auto Chunks = split(Target, n);
    bool ComplementSucceeding = false;

    auto RefinedChunks = refineChunks(Chunks);
    for (auto Chunk : RefinedChunks) {
      if (test(Chunk)) {
        auto TargetSet = toSet(Target);
        auto ChunkSet = toSet(Chunk);
        auto Diff = setDifference(TargetSet, ChunkSet);
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
}
