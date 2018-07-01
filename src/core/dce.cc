#include <clang-c/Index.h>
#include <cstring>

#include <iostream>
#include <vector>

#include "AST_manipulation.h"
#include "cursor_utils.h"
#include "dce.h"
#include "stats.h"

std::vector<CXCursor> cursors;
CXChildVisitResult visitorAll(CXCursor cursor, CXCursor parent,
                              CXClientData clientData) {
  if (!(CursorUtils::isUnexposed(cursor) ||
        CursorUtils::isExtraSemicolon(cursor) || CursorUtils::isField(cursor)))
    cursors.emplace_back(cursor);
  clang_visitChildren(cursor, visitorAll, nullptr);
  return CXChildVisit_Continue;
}

CXCursor DCE::findMoreGeneralCursor(CXCursor cursor) {
  int lastCol = CursorUtils::getEndCol(cursor);
  int lastLine = CursorUtils::getEndLine(cursor);
  CXCursor genCursor = cursor;
  for (auto c : cursors) {
    if (CursorUtils::getStartLine(cursor) == CursorUtils::getStartLine(c) &&
        CursorUtils::getStartCol(cursor) == CursorUtils::getStartCol(c) &&
        CursorUtils::getEndCol(cursor) < CursorUtils::getEndCol(c) &&
        ((lastLine == CursorUtils::getEndLine(c) &&
          lastCol < CursorUtils::getEndCol(c)) ||
         (lastLine < CursorUtils::getEndLine(c)))) {
      genCursor = c;
      lastCol = CursorUtils::getEndCol(c);
      lastLine = CursorUtils::getEndLine(c);
    }
  }
  return genCursor;
}

std::vector<CXCursor> uselessElems;
CXChildVisitResult uselessVisitor(CXCursor cursor, CXCursor parent,
                                  CXClientData clientData) {
  if (clang_Location_isFromMainFile(clang_getCursorLocation(cursor)) == 0)
    return CXChildVisit_Continue;

  CXCursorKind cursorKind = cursor.kind;
  CXCursorKind parentKind = parent.kind;

  if ((cursorKind == CXCursor_UnexposedDecl &&
       parentKind == CXCursor_TranslationUnit) ||
      (cursorKind == CXCursor_NullStmt &&
       parentKind == CXCursor_CompoundStmt)) {
    uselessElems.emplace_back(cursor);
  }

  clang_visitChildren(cursor, uselessVisitor, nullptr);

  return CXChildVisit_Continue;
}

std::vector<CXCursor> DCE::getUselessStatements(CXCursor rootCursor) {
  uselessElems.clear();
  clang_visitChildren(rootCursor, uselessVisitor, nullptr);
  return uselessElems;
}

void DCE::removeVacuousElements(const char *inputPath) {
  CXIndex index = clang_createIndex(1, 0);
  CXTranslationUnit tu =
      clang_parseTranslationUnit(index, inputPath, nullptr, 0, 0, 0, 0);
  std::vector<CXCursor> toBeRemovedElems =
      getUselessStatements(clang_getTranslationUnitCursor(tu));
  ASTManipulation::removeElementsEfficient(inputPath, inputPath,
                                           toBeRemovedElems);
}

void DCE::removeDeadcode(const char *inputPath) {
  std::cout << "Removing unreachable code" << std::endl;
  cursors.clear();
  std::vector<CXCursor> toRemove;
  CXIndex index = clang_createIndex(1, 0);
  char *args[3];
  args[0] = "-Wunreachable-code";
  args[1] = "-Wunused-label";
  args[2] = "-Wunused-variable";

  CXTranslationUnit tu =
      clang_parseTranslationUnit(index, inputPath, args, 3, NULL, 0, 0);

  clang_visitChildren(clang_getTranslationUnitCursor(tu), visitorAll, nullptr);
  CXDiagnosticSet diagSet = clang_getDiagnosticSetFromTU(tu);

  unsigned numDiag = clang_getNumDiagnosticsInSet(diagSet);

  for (auto i = 0U; i < numDiag; i++) {
    CXDiagnostic diag = clang_getDiagnosticInSet(diagSet, i);

    CXString diagText = clang_getDiagnosticSpelling(diag);
    if (strstr(clang_getCString(diagText), "unused variable") != NULL ||
        strstr(clang_getCString(diagText), "unused label") != NULL ||
        strstr(clang_getCString(diagText), "code will never be executed") !=
            NULL) {

      clang_disposeString(diagText);

      CXSourceLocation location = clang_getDiagnosticLocation(diag);
      CXCursor cursor = clang_getCursor(tu, location);
      if (clang_isStatement(cursor.kind) || clang_isDeclaration(cursor.kind)) {
        if (cursor.kind != CXCursor_NullStmt)
          toRemove.emplace_back(cursor);
      } else if (cursor.kind == CXCursor_CallExpr) {
        cursor = DCE::findMoreGeneralCursor(cursor);
        if (cursor.kind != CXCursor_NullStmt)
          toRemove.emplace_back(cursor);
      }
      clang_disposeDiagnostic(diag);
    }
  }

  ASTManipulation::removeElementsEfficient(inputPath, inputPath, toRemove);
  clang_disposeDiagnosticSet(diagSet);
  clang_disposeTranslationUnit(tu);
  clang_disposeIndex(index);
}
