#include <algorithm>
#include <clang-c/Index.h>
#include <iostream>
#include <vector>

#include "cursor_utils.h"
#include "visitor.h"

std::vector<CXCursor> allCursors;
std::vector<CXCursor> globalElements, functionBodies;

void Visitor::clear() {
  allCursors.clear();
  globalElements.clear();
  functionBodies.clear();
}

std::vector<CXCursor> Visitor::getAllCursors() { return allCursors; }

std::vector<CXCursor> Visitor::getGlobalElements() { return globalElements; }

std::vector<CXCursor> Visitor::getFunctionBodies() { return functionBodies; }

void Visitor::setAllCursors(std::vector<CXCursor> newVec) {
  allCursors = newVec;
}

void Visitor::setGlobalElements(std::vector<CXCursor> newVec) {
  globalElements = newVec;
}

void Visitor::setFunctionBodies(std::vector<CXCursor> newVec) {
  functionBodies = newVec;
}

CXCursor Visitor::getFunction(std::vector<CXCursor> vecs, CXCursor cursor) {
  if (cursor.kind == CXCursor_FunctionDecl)
    return cursor;
  CXCursor func = cursor;
  int cursorInd = find(vecs.begin(), vecs.end(), cursor) - vecs.begin();

  while (true) {
    if (vecs[cursorInd].kind == CXCursor_FunctionDecl) {
      func = vecs[cursorInd];
      break;
    }
    cursorInd--;
    if (cursorInd < 0)
      break;
  }
  return func;
}

std::vector<CXCursor> manyLevelChildren;
CXChildVisitResult manyLevelVisitor(CXCursor cursor, CXCursor parent,
                                    CXClientData clientData) {
  if (CursorUtils::stmtConditions(cursor)) {
    manyLevelChildren.emplace_back(cursor);
  }

  clang_visitChildren(cursor, manyLevelVisitor, nullptr);
  return CXChildVisit_Continue;
}

std::vector<CXCursor> Visitor::getAllChildren(CXCursor rootCursor) {
  manyLevelChildren.clear();

  clang_visitChildren(rootCursor, manyLevelVisitor, nullptr);

  return manyLevelChildren;
}

std::vector<CXCursor> oneLevelChildren;
CXChildVisitResult oneLevelVisitor(CXCursor cursor, CXCursor parent,
                                   CXClientData clientData) {
  unsigned int curLevel = *(reinterpret_cast<unsigned int *>(clientData));
  unsigned int nextLevel = curLevel + 1;

  if (curLevel >= 1)
    return CXChildVisit_Break;

  if (CursorUtils::stmtConditions(cursor)) {
    oneLevelChildren.emplace_back(cursor);
  }

  clang_visitChildren(cursor, oneLevelVisitor, &nextLevel);
  return CXChildVisit_Continue;
}

std::vector<CXCursor> Visitor::getImmediateChildren(CXCursor root) {
  oneLevelChildren.clear();

  unsigned int treeLevel = 0;
  clang_visitChildren(root, oneLevelVisitor, &treeLevel);

  return oneLevelChildren;
}

CXChildVisitResult visitor(CXCursor cursor, CXCursor parent,
                           CXClientData clientData) {
  unsigned int curLevel = *(reinterpret_cast<unsigned int *>(clientData));
  unsigned int nextLevel = curLevel + 1;

  if (CursorUtils::stmtConditions(cursor)) {
    allCursors.emplace_back(cursor);
    if (curLevel == 0) {
      globalElements.emplace_back(cursor);
    } else if (curLevel == 1 && cursor.kind == CXCursor_CompoundStmt) {
      functionBodies.emplace_back(cursor);
    }
  }

  clang_visitChildren(cursor, visitor, &nextLevel);

  return CXChildVisit_Continue;
}

void Visitor::fin() {
  clang_disposeTranslationUnit(tu);
  clang_disposeIndex(indx);
}

bool firstTime = true;
void Visitor::init(const char *inputPath) {
  if (firstTime) {
    indx = clang_createIndex(false, false);
    firstTime = false;
  }
  tu = clang_parseTranslationUnit(indx, inputPath, nullptr, 0, 0, 0,
                                  CXTranslationUnit_None);
  CXCursor rootCursor = clang_getTranslationUnitCursor(tu);

  unsigned int treeLevel = 0;
  clang_visitChildren(rootCursor, visitor, &treeLevel);
}
