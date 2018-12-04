#include "Transformation.h"

#include "clang/Lex/Lexer.h"

void Transformation::Initialize(clang::ASTContext &C) {
  Context = &C;
  TheRewriter.setSourceMgr(Context->getSourceManager(), Context->getLangOpts());
}

void Transformation::writeToFile(std::string Filename) {
  std::error_code error_code;
  llvm::raw_fd_ostream outFile(Filename, error_code, llvm::sys::fs::F_None);
  TheRewriter.getEditBuffer(Context->getSourceManager().getMainFileID())
      .write(outFile);
  outFile.close();
}

clang::SourceLocation Transformation::getEndOfStmt(clang::Stmt *S) {
  if (clang::NullStmt *NS = llvm::dyn_cast<clang::NullStmt>(S))
    return NS->getSemiLoc().getLocWithOffset(1);
  if (clang::CompoundStmt *CS = llvm::dyn_cast<clang::CompoundStmt>(S))
    return CS->getRBracLoc().getLocWithOffset(1);
  if (clang::IfStmt *IS = llvm::dyn_cast<clang::IfStmt>(S))
    return getEndLocation(IS->getSourceRange().getEnd());
  if (clang::WhileStmt *WS = llvm::dyn_cast<clang::WhileStmt>(S))
    return getEndOfStmt(WS->getBody());
  if (clang::BinaryOperator *BO = llvm::dyn_cast<clang::BinaryOperator>(S))
    return getEndLocationAfter(BO->getSourceRange(), ';');
  if (clang::ReturnStmt *RS = llvm::dyn_cast<clang::ReturnStmt>(S))
    return getEndLocationAfter(RS->getSourceRange(), ';');
  if (clang::GotoStmt *GS = llvm::dyn_cast<clang::GotoStmt>(S))
    return getEndLocationAfter(GS->getSourceRange(), ';');
  if (clang::BreakStmt *BS = llvm::dyn_cast<clang::BreakStmt>(S))
    return getEndLocationAfter(BS->getSourceRange(), ';');
  if (clang::ContinueStmt *CS = llvm::dyn_cast<clang::ContinueStmt>(S))
    return getEndLocationAfter(CS->getSourceRange(), ';');
  if (clang::DeclStmt *DS = llvm::dyn_cast<clang::DeclStmt>(S))
    return DS->getSourceRange().getEnd().getLocWithOffset(1);
  if (clang::CallExpr *CE = llvm::dyn_cast<clang::CallExpr>(S))
    return getEndLocationAfter(CE->getSourceRange(), ';');
  if (clang::UnaryOperator *UO = llvm::dyn_cast<clang::UnaryOperator>(S))
    return getEndLocationAfter(UO->getSourceRange(), ';');
  if (clang::LabelStmt *LS = llvm::dyn_cast<clang::LabelStmt>(S))
    return getEndOfStmt(LS->getSubStmt());
  return S->getSourceRange().getEnd().getLocWithOffset(1);
}

int Transformation::getOffsetUntil(const char *Buf, char Symbol) {
  int Offset = 0;
  while (*Buf != Symbol) {
    Buf++;
    if (*Buf == '\0')
      break;
    Offset++;
  }
  return Offset;
}

clang::SourceLocation
Transformation::getEndLocation(clang::SourceLocation Loc) {
  const clang::SourceManager &SM = Context->getSourceManager();
  clang::Token Tok;

  clang::SourceLocation Beginning =
      clang::Lexer::GetBeginningOfToken(Loc, SM, Context->getLangOpts());
  clang::Lexer::getRawToken(Beginning, Tok, SM, Context->getLangOpts());

  clang::SourceLocation End;
  if (Tok.getKind() == clang::tok::semi ||
      Tok.getKind() == clang::tok::r_brace) {
    End = Loc.getLocWithOffset(1);
  } else {
    End = getEndLocationAfter(Loc, ';');
  }
  return End;
}

clang::SourceLocation
Transformation::getEndLocationAfter(clang::SourceRange Range, char Symbol) {
  clang::SourceLocation EndLoc = getEndLocationFromBegin(Range);
  if (EndLoc.isInvalid())
    return EndLoc;

  const char *EndBuf = Context->getSourceManager().getCharacterData(EndLoc);
  int Offset = getOffsetUntil(EndBuf, Symbol) + 1;
  return EndLoc.getLocWithOffset(Offset);
}

clang::SourceLocation
Transformation::getEndLocationUntil(clang::SourceRange Range, char Symbol) {
  clang::SourceLocation EndLoc = getEndLocationFromBegin(Range);
  if (EndLoc.isInvalid())
    return EndLoc;

  const char *EndBuf = Context->getSourceManager().getCharacterData(EndLoc);
  int Offset = getOffsetUntil(EndBuf, Symbol);
  return EndLoc.getLocWithOffset(Offset);
}

void Transformation::removeSourceText(const clang::SourceRange &SR) {
  std::string Text = getSourceText(SR);
  std::string Replacement = "";
  for (auto const &chr : Text) {
    if (chr == '\n')
      Replacement += '\n';
    else if (isprint(chr))
      Replacement += " ";
    else
      Replacement += chr;
  }
  TheRewriter.ReplaceText(SR, Replacement);
}

std::string Transformation::getSourceText(const clang::SourceRange &SR) {
  const clang::SourceManager *SM = &Context->getSourceManager();
  llvm::StringRef ref = clang::Lexer::getSourceText(
      clang::CharSourceRange::getCharRange(SR), *SM, clang::LangOptions());
  return ref.str();
}

clang::SourceLocation
Transformation::getEndLocationFromBegin(clang::SourceRange Range) {
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
