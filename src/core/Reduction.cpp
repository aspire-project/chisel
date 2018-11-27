#include "Reduction.h"

#include "clang/Basic/SourceManager.h"
#include "clang/Lex/Lexer.h"
#include "llvm/Support/Program.h"

#include "OptionManager.h"
#include "Profiler.h"
#include "VectorUtils.h"

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

void Reduction::doDeltaDebugging(std::vector<DDElement> &Decls) {
  std::vector<DDElement> Target = std::move(Decls);
  int n = 2;
  while (Target.size() >= 1) {
    auto subsets = VectorUtils::split<DDElement>(Target, n);
    bool ComplementSucceeding = false;

    auto refinedSubsets = refineSubsets(subsets);

    for (auto subset : refinedSubsets) {
      if (test(subset)) {
        Target = std::move(VectorUtils::difference<DDElement>(Target, subset));
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
