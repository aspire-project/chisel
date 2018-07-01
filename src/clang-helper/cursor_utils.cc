#include <clang-c/Index.h>
#include <stdlib.h>

#include <algorithm>
#include <cstring>
#include <iostream>
#include <iterator>
#include <set>
#include <string>
#include <vector>

#include <boost/functional/hash.hpp>

#include "cursor_utils.h"

std::string CursorUtils::getCursorKindName(CXCursor cursor) {
  return clang_getCString(clang_getCursorKindSpelling(cursor.kind));
}

std::string CursorUtils::getCursorSpelling(CXCursor cursor) {
  return clang_getCString(clang_getCursorSpelling(cursor));
}

int CursorUtils::getStartLine(CXCursor c) {
  CXSourceRange extent = clang_getCursorExtent(c);
  unsigned int startLine = 0, startCol = 0;
  clang_getSpellingLocation(clang_getRangeStart(extent), nullptr, &startLine,
                            &startCol, nullptr);
  return startLine;
}

int CursorUtils::getStartCol(CXCursor c) {
  CXSourceRange extent = clang_getCursorExtent(c);
  unsigned int startLine = 0, startCol = 0;
  clang_getSpellingLocation(clang_getRangeStart(extent), nullptr, &startLine,
                            &startCol, nullptr);
  return startCol;
}

int CursorUtils::getEndLine(CXCursor c) {
  CXSourceRange extent = clang_getCursorExtent(c);
  unsigned int endLine = 0, endCol = 0;
  clang_getSpellingLocation(clang_getRangeEnd(extent), nullptr, &endLine,
                            &endCol, nullptr);
  return endLine;
}

int CursorUtils::getEndCol(CXCursor c) {
  CXSourceRange extent = clang_getCursorExtent(c);
  unsigned int endLine = 0, endCol = 0;
  clang_getSpellingLocation(clang_getRangeEnd(extent), nullptr, &endLine,
                            &endCol, nullptr);
  return endCol;
}

void CursorUtils::prettyPrintSet(std::vector<CXCursor> c) {
  for (auto i : c)
    std::cout << i << std::endl;
}

std::vector<std::vector<CXCursor>> CursorUtils::split(std::vector<CXCursor> vec,
                                                      int n) {
  std::vector<std::vector<CXCursor>> result;
  int length = static_cast<int>(vec.size()) / n;
  int remain = static_cast<int>(vec.size()) % n;

  int begin = 0, end = 0;
  for (int i = 0; i < std::min(n, static_cast<int>(vec.size())); ++i) {
    end += (remain > 0) ? (length + !!(remain--)) : length;
    result.emplace_back(
        std::vector<CXCursor>(vec.begin() + begin, vec.begin() + end));
    begin = end;
  }
  return result;
}

bool CursorUtils::contains(std::vector<CXCursor> vec, CXCursor cursor) {
  if (std::find(vec.begin(), vec.end(), cursor) != vec.end())
    return true;
  return false;
}

std::vector<CXCursor> CursorUtils::subtract(std::vector<CXCursor> a,
                                            std::vector<CXCursor> b) {
  // a - b
  if (b.size() == 0 || a.size() == 0)
    return a;
  std::vector<CXCursor> c;

  for (auto item : a) {
    if (!contains(b, item)) {
      c.emplace_back(item);
    }
  }
  return c;
}

bool CursorUtils::isLiteral(CXCursor cursor) {
  return (cursor.kind == CXCursor_ImaginaryLiteral) ||
         (cursor.kind == CXCursor_CharacterLiteral) ||
         (cursor.kind == CXCursor_FloatingLiteral) ||
         (cursor.kind == CXCursor_IntegerLiteral) ||
         (cursor.kind == CXCursor_StringLiteral);
}

bool CursorUtils::isExtraSemicolon(CXCursor cursor) {
  CXCursor parent = clang_getCursorSemanticParent(cursor);

  bool gl =
      CursorUtils::getEndLine(cursor) == CursorUtils::getStartLine(cursor) &&
      CursorUtils::getEndCol(cursor) - CursorUtils::getStartCol(cursor) == 1 &&
      cursor.kind == CXCursor_UnexposedDecl &&
      parent.kind == CXCursor_TranslationUnit;

  bool lc = cursor.kind == CXCursor_NullStmt;

  return gl || lc;
}

bool CursorUtils::isExpr(CXCursor cursor) {
  return (clang_isExpression(cursor.kind));
}

bool CursorUtils::isStmt(CXCursor cursor) {
  return (clang_isStatement(cursor.kind));
}

bool CursorUtils::isField(CXCursor cursor) {
  return (cursor.kind == CXCursor_FieldDecl ||
          cursor.kind == CXCursor_EnumConstantDecl);
}

bool CursorUtils::isUnexposed(CXCursor cursor) {
  return (clang_isUnexposed(cursor.kind));
}

bool operator<(CXCursor const &lhs, CXCursor const &rhs) {
  CXSourceRange lhsExtent = clang_getCursorExtent(lhs);
  CXSourceRange rhsExtent = clang_getCursorExtent(rhs);

  unsigned int lhsStartLine = 0;
  unsigned int lhsStartCol = 0;
  unsigned int rhsStartLine = 0;
  unsigned int rhsStartCol = 0;

  unsigned int lhsEndLine = 0;
  unsigned int lhsEndCol = 0;
  unsigned int rhsEndLine = 0;
  unsigned int rhsEndCol = 0;

  clang_getSpellingLocation(clang_getRangeStart(lhsExtent), nullptr,
                            &lhsStartLine, &lhsStartCol, nullptr);
  clang_getSpellingLocation(clang_getRangeEnd(lhsExtent), nullptr, &lhsEndLine,
                            &lhsEndCol, nullptr);
  clang_getSpellingLocation(clang_getRangeStart(rhsExtent), nullptr,
                            &rhsStartLine, &rhsStartCol, nullptr);
  clang_getSpellingLocation(clang_getRangeEnd(rhsExtent), nullptr, &rhsEndLine,
                            &rhsEndCol, nullptr);

  return (lhsStartLine < rhsStartLine) ||
         (lhsStartLine == rhsStartLine && lhsStartCol < rhsStartCol) ||
         (lhsStartLine == rhsStartLine && lhsStartCol == rhsStartCol &&
          lhsEndLine > rhsEndLine) ||
         (lhsStartLine == rhsStartLine && lhsStartCol == rhsStartCol &&
          lhsEndLine == rhsEndLine && lhsEndCol > rhsEndCol);
}

bool operator==(CXCursor const &lhs, CXCursor const &rhs) {
  return clang_equalCursors(lhs, rhs);
}

bool operator!=(CXCursor const &lhs, CXCursor const &rhs) {
  return !(lhs == rhs);
}

std::ostream &operator<<(std::ostream &stream, const CXCursor &c) {
  stream << CursorUtils::getCursorSpelling(c) << " : "
         << CursorUtils::getCursorKindName(c) << " @ "
         << CursorUtils::getStartLine(c) << "," << CursorUtils::getStartCol(c)
         << "-" << CursorUtils::getEndLine(c) << ","
         << CursorUtils::getEndCol(c) << std::endl;
  return stream;
}

bool CursorUtils::stmtConditions(CXCursor cursor) {
  return !(CursorUtils::isLiteral(cursor) || CursorUtils::isUnexposed(cursor) ||
           CursorUtils::isExtraSemicolon(cursor) ||
           CursorUtils::isField(cursor));
}
