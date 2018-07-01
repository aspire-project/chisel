#include <clang-c/Index.h>
#include <string.h>

#include <algorithm>
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <iterator>
#include <string>

#include "cursor_utils.h"
#include "stats.h"

int Stats::getTokenCount(const char *srcPath) {
  CXIndex indx = clang_createIndex(false, false);
  CXTranslationUnit tu = clang_parseTranslationUnit(
      indx, srcPath, nullptr, 0, 0, 0, CXTranslationUnit_None);

  unsigned int numTokens = 0;
  CXSourceRange extent =
      clang_getCursorExtent(clang_getTranslationUnitCursor(tu));
  clang_tokenize(tu, extent, nullptr, &numTokens);
  return numTokens;
}

int Stats::getWordCount(const char *srcPath) {
  std::ifstream ifs(srcPath);
  std::string word;
  int count = 0;
  while (ifs >> word) {
    count++;
  }
  return count;
}

int stmt = 0;
int funcs = 0;
std::vector<CXCursor> stmts;
CXChildVisitResult stmtVisitor(CXCursor cursor, CXCursor parent,
                               CXClientData clientData) {
  unsigned int curLevel = *(reinterpret_cast<unsigned int *>(clientData));
  unsigned int nextLevel = curLevel + 1;

  if ((clang_isStatement(cursor.kind) && cursor.kind != CXCursor_DeclStmt) ||
      (cursor.kind == CXCursor_CallExpr &&
       (parent.kind != CXCursor_ReturnStmt &&
        parent.kind != CXCursor_CallExpr &&
        parent.kind != CXCursor_BinaryOperator)) ||
      cursor.kind == CXCursor_BinaryOperator ||
      cursor.kind == CXCursor_UnaryOperator) {
    if (cursor.kind != CXCursor_CompoundStmt &&
        cursor.kind != CXCursor_LabelStmt) {
      if (!CursorUtils::isExtraSemicolon(cursor)) {
        stmts.emplace_back(cursor);
      }
    }
  }
  if (cursor.kind == CXCursor_FunctionDecl &&
      clang_equalCursors(clang_getCursorDefinition(cursor), cursor)) {
    funcs++;
  }

  clang_visitChildren(cursor, stmtVisitor, &nextLevel);

  return CXChildVisit_Continue;
}

std::vector<int> Stats::getStatementCount(const char *srcPath) {
  CXIndex indx = clang_createIndex(false, false);
  CXTranslationUnit tu = clang_parseTranslationUnit(
      indx, srcPath, nullptr, 0, 0, 0, CXTranslationUnit_None);
  CXCursor rootCursor = clang_getTranslationUnitCursor(tu);

  unsigned int treeLevel = 0;
  stmt = 0;
  funcs = 0;
  stmts.clear();
  clang_visitChildren(rootCursor, stmtVisitor, &treeLevel);
  for (auto cursor : stmts) {
    if (cursor.kind == CXCursor_BinaryOperator) {
      CXToken *tokens;
      unsigned numTokens;
      CXSourceRange range = clang_getCursorExtent(cursor);
      clang_tokenize(tu, range, &tokens, &numTokens);
      for (unsigned i = 0; i < numTokens; i++) {
        CXString s = clang_getTokenSpelling(tu, tokens[i]);
        const char *str = clang_getCString(s);
        if (strcmp(str, "=") == 0) {
          stmt++;
        }
        clang_disposeString(s);
      }
      clang_disposeTokens(tu, tokens, numTokens);
    } else if (cursor.kind == CXCursor_UnaryOperator) {
      CXToken *tokens;
      unsigned numTokens;
      CXSourceRange range = clang_getCursorExtent(cursor);
      clang_tokenize(tu, range, &tokens, &numTokens);
      for (unsigned i = 0; i < numTokens; i++) {
        CXString s = clang_getTokenSpelling(tu, tokens[i]);
        const char *str = clang_getCString(s);
        if (strcmp(str, "++") == 0 || strcmp(str, "--") == 0) {
          stmt++;
        }
        clang_disposeString(s);
      }
      clang_disposeTokens(tu, tokens, numTokens);

    } else {
      stmt++;
    }
  }
  clang_disposeTranslationUnit(tu);
  clang_disposeIndex(indx);
  return {stmt, funcs};
}
