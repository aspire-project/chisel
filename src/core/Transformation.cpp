#include "Transformation.h"

#include <queue>

#include "clang/Lex/Lexer.h"

void Transformation::Initialize(clang::ASTContext &C) {
  Context = &C;
  TheRewriter.setSourceMgr(Context->getSourceManager(), Context->getLangOpts());
}

std::vector<clang::Stmt *> Transformation::getAllChildren(clang::Stmt *S) {
  std::queue<clang::Stmt *> ToVisit;
  std::vector<clang::Stmt *> AllChildren;
  ToVisit.push(S);
  while (!ToVisit.empty()) {
    auto C = ToVisit.front();
    ToVisit.pop();
    AllChildren.emplace_back(C);
    for (auto const &Child : C->children()) {
      if (Child != NULL)
        ToVisit.push(Child);
    }
  }
  return AllChildren;
}

clang::SourceLocation Transformation::getEndOfStmt(clang::Stmt *S) {
  if (clang::NullStmt *NS = llvm::dyn_cast<clang::NullStmt>(S))
    return NS->getSemiLoc();
  if (clang::CompoundStmt *CS = llvm::dyn_cast<clang::CompoundStmt>(S))
    return CS->getRBracLoc();
  if (clang::IfStmt *IS = llvm::dyn_cast<clang::IfStmt>(S))
    return getEndLocation(IS->getSourceRange().getEnd());
  if (clang::WhileStmt *WS = llvm::dyn_cast<clang::WhileStmt>(S))
    return getEndOfStmt(WS->getBody());
  if (clang::ForStmt *FS = llvm::dyn_cast<clang::ForStmt>(S))
    return getEndOfStmt(FS->getBody());
  if (clang::SwitchStmt *SS = llvm::dyn_cast<clang::SwitchStmt>(S))
    return getEndOfStmt(SS->getBody());
  if (clang::BinaryOperator *BO = llvm::dyn_cast<clang::BinaryOperator>(S))
    return getEndLocationUntil(BO->getSourceRange(), ';');
  if (clang::ReturnStmt *RS = llvm::dyn_cast<clang::ReturnStmt>(S))
    return getEndLocationUntil(RS->getSourceRange(), ';');
  if (clang::GotoStmt *GS = llvm::dyn_cast<clang::GotoStmt>(S))
    return getEndLocationUntil(GS->getSourceRange(), ';');
  if (clang::BreakStmt *BS = llvm::dyn_cast<clang::BreakStmt>(S))
    return getEndLocationUntil(BS->getSourceRange(), ';');
  if (clang::ContinueStmt *CS = llvm::dyn_cast<clang::ContinueStmt>(S))
    return getEndLocationUntil(CS->getSourceRange(), ';');
  if (clang::DeclStmt *DS = llvm::dyn_cast<clang::DeclStmt>(S))
    return DS->getSourceRange().getEnd();
  if (clang::CallExpr *CE = llvm::dyn_cast<clang::CallExpr>(S))
    return getEndLocationUntil(CE->getSourceRange(), ';');
  if (clang::UnaryOperator *UO = llvm::dyn_cast<clang::UnaryOperator>(S))
    return getEndLocationUntil(UO->getSourceRange(), ';');
  if (clang::LabelStmt *LS = llvm::dyn_cast<clang::LabelStmt>(S))
    return getEndOfStmt(LS->getSubStmt());
  if (clang::ParenExpr *PE = llvm::dyn_cast<clang::ParenExpr>(S))
    return getEndLocationUntil(PE->getSourceRange(), ';');
  return S->getSourceRange().getEnd();
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
    End = Loc;
  } else {
    End = getEndLocationUntil(Loc, ';');
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

void Transformation::removeSourceText(const clang::SourceLocation &B,
                                      const clang::SourceLocation &E) {
  llvm::StringRef Text = getSourceText(B, E);
  std::string Replacement = "";
  for (auto const &chr : Text) {
    if (chr == '\n')
      Replacement += '\n';
    else if (isprint(chr))
      Replacement += " ";
    else
      Replacement += chr;
  }
  TheRewriter.ReplaceText(clang::SourceRange(B, E), Replacement);
}

llvm::StringRef Transformation::getSourceText(const clang::SourceLocation &B,
                                              const clang::SourceLocation &E) {
  const clang::SourceManager *SM = &Context->getSourceManager();
  return clang::Lexer::getSourceText(
      clang::CharSourceRange::getCharRange(B, E.getLocWithOffset(1)), *SM,
      clang::LangOptions());
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
