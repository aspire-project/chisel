#include <clang-c/Index.h>
#include <stdlib.h>

#include <algorithm>
#include <cstring>
#include <fstream>
#include <iostream>
#include <iterator>
#include <map>
#include <queue>
#include <vector>

#include "AST_manipulation.h"
#include "cursor_utils.h"
#include "delta_debugging.h"
#include "hierarchical_delta_debugging.h"
#include "options.h"
#include "rules.h"

void HierarchicalDeltaDebugging::clearQueue(std::queue<CXCursor> &q) {
  std::queue<CXCursor> empty;
  std::swap(q, empty);
}

void HierarchicalDeltaDebugging::addToQueue(std::vector<CXCursor> cursors) {
  for (auto cursor : cursors)
    q.push(cursor);
}

std::vector<CXCursor>
HierarchicalDeltaDebugging::getElementChildren(CXCursor cursor) {
  std::vector<CXCursor> immCh = dd.getImmediateChildren(cursor);
  std::vector<CXCursor> results;
  for (auto c : immCh)
    if (CursorUtils::contains(dd.getAllCursors(), c))
      results.emplace_back(c);
  return results;
}

void HierarchicalDeltaDebugging::reductionMessage(std::string kind,
                                                  unsigned int size) {
  std::cout << "Reducing " << kind << " [size: " << size << "]\n";
}

void HierarchicalDeltaDebugging::reduceIf(CXCursor cursor, const char *srcPath,
                                          const char *dstPath) {
  std::vector<CXCursor> immedChildren = getElementChildren(cursor);
  if (Option::verbose)
    reductionMessage("if", immedChildren.size());
  // remove the whole if statement
  ASTManipulation::removeLineRange(
      srcPath, dstPath, CursorUtils::getStartLine(cursor),
      CursorUtils::getStartCol(cursor), CursorUtils::getEndLine(cursor),
      CursorUtils::getEndCol(cursor));
  if (dd.test({}, dstPath, dstPath, LOCAL) == SUCCESS) {
    dd.removeFromAllCursors({cursor}, true);
  } else {
    if (immedChildren.size() == 1 ||
        (immedChildren.size() == 2 &&
         immedChildren[0].kind != CXCursor_CompoundStmt)) { // if
      CXCursor trueBranch = immedChildren.back();
      ASTManipulation::removeLineRange(
          srcPath, dstPath, CursorUtils::getStartLine(cursor),
          CursorUtils::getStartCol(cursor),
          CursorUtils::getStartLine(trueBranch),
          CursorUtils::getStartCol(trueBranch) - 1);
      if (dd.test({}, dstPath, dstPath, LOCAL) == SUCCESS) {
        // keep true branch
        dd.removeFromAllCursors({cursor}, false);
      }
      addToQueue({trueBranch});
    } else if ((immedChildren.size() == 2 &&
                immedChildren[0].kind == CXCursor_CompoundStmt &&
                immedChildren[1].kind == CXCursor_CompoundStmt) ||
               immedChildren.size() == 3) { // if-else
      CXCursor trueBranch = immedChildren[immedChildren.size() - 2];
      CXCursor falseBranch = immedChildren.back();
      ASTManipulation::removeLineRange(
          srcPath, dstPath, CursorUtils::getStartLine(cursor),
          CursorUtils::getStartCol(cursor),
          CursorUtils::getStartLine(trueBranch),
          CursorUtils::getStartCol(trueBranch) - 1);
      ASTManipulation::removeLineRange(dstPath, dstPath,
                                       CursorUtils::getEndLine(trueBranch),
                                       CursorUtils::getEndCol(trueBranch),
                                       CursorUtils::getEndLine(falseBranch),
                                       CursorUtils::getEndCol(falseBranch));
      if (dd.test({}, dstPath, dstPath, LOCAL) == SUCCESS) {
        // keep true branch
        dd.removeFromAllCursors({falseBranch}, true);
        dd.removeFromAllCursors({cursor}, false);
        addToQueue({trueBranch});
      } else {
        ASTManipulation::removeLineRange(
            srcPath, dstPath, CursorUtils::getStartLine(cursor),
            CursorUtils::getStartCol(cursor),
            CursorUtils::getStartLine(falseBranch),
            CursorUtils::getStartCol(falseBranch) - 1);
        if (dd.test({}, dstPath, dstPath, LOCAL) == SUCCESS) {
          // keep false branch
          dd.removeFromAllCursors({trueBranch}, true);
          dd.removeFromAllCursors({cursor}, false);
          addToQueue({falseBranch});
        } else {
          addToQueue({falseBranch, trueBranch});
        }
      }
    }
  }
}

void HierarchicalDeltaDebugging::reduceSwitch(CXCursor cursor,
                                              const char *srcPath,
                                              const char *dstPath) {
  std::vector<CXCursor> immedChildren = getElementChildren(cursor);
  if (Option::verbose)
    reductionMessage("switch", immedChildren.size());
  ASTManipulation::removeLineRange(
      srcPath, dstPath, CursorUtils::getStartLine(cursor),
      CursorUtils::getStartCol(cursor), CursorUtils::getEndLine(cursor),
      CursorUtils::getEndCol(cursor));
  if (dd.test({}, dstPath, dstPath, LOCAL) == SUCCESS) {
    dd.removeFromAllCursors({cursor}, true);
  } else {
    addToQueue(immedChildren);
  }
}

void HierarchicalDeltaDebugging::reduceCaseOrDefault(CXCursor cursor,
                                                     const char *srcPath,
                                                     const char *dstPath) {
  std::vector<CXCursor> immedChildren = getElementChildren(cursor);
  if (Option::verbose)
    reductionMessage("case", immedChildren.size());
  ASTManipulation::removeLineRange(
      srcPath, dstPath, CursorUtils::getStartLine(cursor),
      CursorUtils::getStartCol(cursor), CursorUtils::getEndLine(cursor),
      CursorUtils::getEndCol(cursor));
  if (dd.test({}, dstPath, dstPath, LOCAL) == SUCCESS) {
    dd.removeFromAllCursors({cursor}, true);
  } else {
    addToQueue(immedChildren);
  }
}

void HierarchicalDeltaDebugging::reduceWhile(CXCursor cursor,
                                             const char *srcPath,
                                             const char *dstPath) {
  std::vector<CXCursor> immedChildren = getElementChildren(cursor);
  if (Option::verbose)
    reductionMessage("while", immedChildren.size());
  // remove the whole while statement
  ASTManipulation::removeLineRange(
      srcPath, dstPath, CursorUtils::getStartLine(cursor),
      CursorUtils::getStartCol(cursor), CursorUtils::getEndLine(cursor),
      CursorUtils::getEndCol(cursor));
  if (dd.test({}, dstPath, dstPath, LOCAL) == SUCCESS) {
    dd.removeFromAllCursors({cursor}, true);
  } else {
    CXCursor loopBody = immedChildren.back();
    ASTManipulation::removeLineRange(
        srcPath, dstPath, CursorUtils::getStartLine(cursor),
        CursorUtils::getStartCol(cursor), CursorUtils::getStartLine(loopBody),
        CursorUtils::getStartCol(loopBody) - 1);
    if (dd.test({}, dstPath, dstPath, LOCAL) == SUCCESS) {
      dd.removeFromAllCursors({cursor}, false);
      dd.removeFromAllCursors({immedChildren[0]}, true);
      addToQueue({loopBody});
    } else {
      addToQueue({loopBody});
    }
  }
}

void HierarchicalDeltaDebugging::reduceBlock(CXCursor cursor,
                                             const char *srcPath,
                                             const char *dstPath) {
  std::vector<CXCursor> immedChildren = getElementChildren(cursor);
  if (Option::verbose)
    reductionMessage("block", immedChildren.size());
  CXCursorKind parentKind =
      clang_getCursorKind(clang_getCursorSemanticParent(cursor));
  if (parentKind != CXCursor_FunctionDecl && parentKind != CXCursor_WhileStmt &&
      parentKind != CXCursor_IfStmt) {
    ASTManipulation::removeLineRange(
        srcPath, dstPath, CursorUtils::getStartLine(cursor),
        CursorUtils::getStartCol(cursor), CursorUtils::getEndLine(cursor),
        CursorUtils::getEndCol(cursor));
    if (dd.test({}, dstPath, dstPath, LOCAL) == SUCCESS) {
      dd.removeFromAllCursors({cursor}, true);
    } else {
      dd.ddmin(immedChildren, srcPath, dstPath, LOCAL);
      addToQueue(immedChildren);
    }
  } else {
    dd.ddmin(immedChildren, srcPath, dstPath, LOCAL);
    addToQueue(immedChildren);
  }
}

void HierarchicalDeltaDebugging::reduceLabel(CXCursor cursor,
                                             const char *srcPath,
                                             const char *dstPath) {
  std::vector<CXCursor> immedChildren = getElementChildren(cursor);
  if (Option::verbose)
    reductionMessage("label", immedChildren.size());
  ASTManipulation::removeLineRange(
      srcPath, dstPath, CursorUtils::getStartLine(cursor),
      CursorUtils::getStartCol(cursor), CursorUtils::getEndLine(cursor),
      CursorUtils::getEndCol(cursor));
  if (dd.test({}, dstPath, dstPath, LOCAL) == SUCCESS) {
    dd.removeFromAllCursors({cursor}, true);
  } else {
    dd.ddmin(immedChildren, srcPath, dstPath, LOCAL);
    addToQueue(immedChildren);
  }
}

void HierarchicalDeltaDebugging::hierarchicalDD(CXCursor cursor,
                                                const char *srcPath,
                                                const char *dstPath) {
  if (!CursorUtils::contains(dd.getAllCursors(), cursor))
    return;

  switch (cursor.kind) {
  case CXCursor_IfStmt:
    reduceIf(cursor, srcPath, dstPath);
    break;
  case CXCursor_SwitchStmt:
    reduceSwitch(cursor, srcPath, dstPath);
    break;
  case CXCursor_CaseStmt:
  case CXCursor_DefaultStmt:
    reduceCaseOrDefault(cursor, srcPath, dstPath);
    break;
  case CXCursor_WhileStmt:
    reduceWhile(cursor, srcPath, dstPath);
    break;
  case CXCursor_CompoundStmt:
    reduceBlock(cursor, srcPath, dstPath);
    break;
  case CXCursor_LabelStmt:
    reduceLabel(cursor, srcPath, dstPath);
    break;
  }
}

bool firstTimeInit = true;
void HierarchicalDeltaDebugging::globalReduction(const char *srcPath,
                                                 const char *dstPath) {
  std::cout << "Reducing global elements" << std::endl;
  dd.ddmin(dd.getGlobalElements(), srcPath, dstPath, GLOBAL);
}

void HierarchicalDeltaDebugging::localReduction(const char *srcPath,
                                                const char *dstPath) {
  std::cout << "Reducing local elements" << std::endl;
  clearQueue(q);
  auto l1 = dd.getFunctionBodies();
  for (auto c : l1) {
    if (!CursorUtils::contains(dd.getAllCursors(), c))
      continue;
    addToQueue({c});
    while (!q.empty()) {
      CXCursor cursor = q.front();
      q.pop();
      hierarchicalDD(cursor, srcPath, dstPath);
    }
  }
}

void HierarchicalDeltaDebugging::fin() { dd.fin(); }

void HierarchicalDeltaDebugging::init(const char *inputPath) {
  dd.init(inputPath);
}
