#include "SourceManager.h"

#include "clang/Lex/Lexer.h"

bool SourceManager::IsInHeader(const clang::SourceManager &SM, clang::Decl *D) {
  return SM.getFilename(D->getLocation()).endswith(".h");
}

clang::SourceLocation
SourceManager::FindLocationAfterCond(const clang::SourceManager &SM,
                                     clang::Expr *E) {
  clang::SourceLocation L = E->getEndLoc();
  if (L.isMacroID())
    L = SM.getFileLoc(L);
  L = clang::Lexer::findLocationAfterToken(L, clang::tok::r_paren, SM,
                                           clang::LangOptions(), false);
  if (L.isMacroID())
    L = SM.getFileLoc(L);
  return L;
}

clang::SourceLocation
SourceManager::GetEndOfCond(const clang::SourceManager &SM, clang::Expr *E) {
  clang::SourceLocation L = FindLocationAfterCond(SM, E);
  if (L.isInvalid())
    return L;
  else
    return L.getLocWithOffset(-1);
}

int getOffsetUntil(const char *Buf, char Symbol) {
  int Offset = 0;
  while (*Buf != Symbol) {
    Buf++;
    if (*Buf == '\0')
      break;
    Offset++;
  }
  return Offset;
}

int SourceManager::GetStartingColumn(clang::SourceManager &SM, int Line) {
  clang::SourceLocation B = SM.translateLineCol(SM.getMainFileID(), Line, 1);
  return clang::Lexer::getIndentationForLine(B, SM).size() + 1;
}

clang::SourceLocation
SourceManager::GetEndLocationUntil(const clang::SourceManager &SM,
                                   clang::SourceLocation EndLoc,
                                   clang::tok::TokenKind Symbol) {
  if (EndLoc.isMacroID())
    EndLoc = SM.getFileLoc(EndLoc);

  if (EndLoc.isInvalid())
    return EndLoc;

  EndLoc = clang::Lexer::findLocationAfterToken(EndLoc, Symbol, SM,
                                                clang::LangOptions(), false);

  if (EndLoc.isMacroID())
    EndLoc = SM.getFileLoc(EndLoc);

  if (EndLoc.isInvalid())
    return EndLoc;

  return EndLoc.getLocWithOffset(-1);
}

clang::SourceLocation SourceManager::GetEndLocation(clang::ASTContext *Context,
                                                    clang::SourceLocation Loc) {
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
    End = GetEndLocationUntil(SM, Loc, clang::tok::semi);
  }
  return End;
}

clang::SourceLocation
SourceManager::GetRealLocation(clang::ASTContext *Context,
                               clang::SourceLocation Loc) {
  const clang::SourceManager &SM = Context->getSourceManager();
  if (Loc.isMacroID())
    return SM.getFileLoc(Loc);
  else
    return Loc;
}

clang::SourceLocation SourceManager::GetBeginOfStmt(clang::ASTContext *Context,
                                                    clang::Stmt *S) {
  const clang::SourceManager &SM = Context->getSourceManager();
  clang::SourceLocation Start = S->getSourceRange().getBegin();
  if (Start.isMacroID())
    return SM.getFileLoc(Start);
  else
    return Start;
}

clang::SourceLocation SourceManager::GetEndOfStmt(clang::ASTContext *Context,
                                                  clang::Stmt *S) {
  const clang::SourceManager &SM = Context->getSourceManager();
  if (clang::NullStmt *NS = llvm::dyn_cast<clang::NullStmt>(S))
    return NS->getSemiLoc();
  if (clang::CompoundStmt *CS = llvm::dyn_cast<clang::CompoundStmt>(S))
    return CS->getRBracLoc();
  if (clang::IfStmt *IS = llvm::dyn_cast<clang::IfStmt>(S))
    return GetEndLocation(Context, IS->getEndLoc());
  if (clang::WhileStmt *WS = llvm::dyn_cast<clang::WhileStmt>(S))
    return GetEndOfStmt(Context, WS->getBody());
  if (clang::DoStmt *DS = llvm::dyn_cast<clang::DoStmt>(S))
    return GetEndLocationUntil(SM, DS->getRParenLoc(), clang::tok::semi);
  if (clang::ForStmt *FS = llvm::dyn_cast<clang::ForStmt>(S))
    return GetEndOfStmt(Context, FS->getBody());
  if (clang::SwitchStmt *SS = llvm::dyn_cast<clang::SwitchStmt>(S))
    return GetEndOfStmt(Context, SS->getBody());
  if (clang::BinaryOperator *BO = llvm::dyn_cast<clang::BinaryOperator>(S))
    return GetEndLocationUntil(SM, BO->getEndLoc(), clang::tok::semi);
  if (clang::ReturnStmt *RS = llvm::dyn_cast<clang::ReturnStmt>(S))
    return GetEndLocationUntil(SM, RS->getEndLoc(), clang::tok::semi);
  if (clang::GotoStmt *GS = llvm::dyn_cast<clang::GotoStmt>(S))
    return GetEndLocationUntil(SM, GS->getEndLoc(), clang::tok::semi);
  if (clang::BreakStmt *BS = llvm::dyn_cast<clang::BreakStmt>(S))
    return GetEndLocationUntil(SM, BS->getEndLoc(), clang::tok::semi);
  if (clang::ContinueStmt *CS = llvm::dyn_cast<clang::ContinueStmt>(S))
    return GetEndLocationUntil(SM, CS->getEndLoc(), clang::tok::semi);
  if (clang::DeclStmt *DS = llvm::dyn_cast<clang::DeclStmt>(S))
    return DS->getEndLoc();
  if (clang::CallExpr *CE = llvm::dyn_cast<clang::CallExpr>(S))
    return GetEndLocationUntil(SM, CE->getEndLoc(), clang::tok::semi);
  if (clang::UnaryOperator *UO = llvm::dyn_cast<clang::UnaryOperator>(S))
    return GetEndLocationUntil(SM, UO->getEndLoc(), clang::tok::semi);
  if (clang::LabelStmt *LS = llvm::dyn_cast<clang::LabelStmt>(S))
    return GetEndOfStmt(Context, LS->getSubStmt());
  if (clang::ParenExpr *PE = llvm::dyn_cast<clang::ParenExpr>(S))
    return GetEndLocationUntil(SM, PE->getEndLoc(), clang::tok::semi);
  if (clang::SwitchCase *SC = llvm::dyn_cast<clang::SwitchCase>(S))
    return GetEndLocationUntil(SM, SC->getEndLoc(), clang::tok::semi);
  if (clang::CastExpr *CE = llvm::dyn_cast<clang::CastExpr>(S))
    return GetEndOfStmt(Context, CE->getSubExpr());
  if (clang::GCCAsmStmt *AS = llvm::dyn_cast<clang::GCCAsmStmt>(S))
    return GetEndLocationUntil(SM, AS->getEndLoc(), clang::tok::semi);

  return S->getEndLoc();
}

llvm::StringRef SourceManager::GetSourceText(const clang::SourceManager &SM,
                                             const clang::SourceLocation &B,
                                             const clang::SourceLocation &E) {
  return clang::Lexer::getSourceText(
      clang::CharSourceRange::getCharRange(B, E.getLocWithOffset(1)), SM,
      clang::LangOptions());
}

llvm::StringRef SourceManager::GetSourceText(const clang::SourceManager &SM,
                                             const clang::SourceRange &SR) {
  return GetSourceText(SM, SR.getBegin(), SR.getEnd());
}
