#include <clang-c/Index.h>
#include <time.h>

#include <algorithm>
#include <fstream>
#include <iostream>
#include <iterator>
#include <string>
#include <vector>

#include "AST_manipulation.h"
#include "options.h"

void ASTManipulation::removeElementsEfficient(const char *srcPath,
                                              const char *dstPath,
                                              std::vector<CXCursor> cursors) {
  std::ifstream ifs;
  std::ofstream ofs;

  ifs.open(srcPath);
  std::string tmp = Option::outputDir + "/tmp.txt";
  ofs.open(tmp.c_str(), std::ofstream::out);

  std::vector<std::string> content;
  content.reserve(500000);

  while (!ifs.eof()) {
    std::string line;
    std::getline(ifs, line);
    content.emplace_back(line);
  }
  content.pop_back();

  for (CXCursor cursor : cursors) {
    CXSourceRange extent = clang_getCursorExtent(cursor);

    CXSourceLocation startLocation = clang_getRangeStart(extent);
    CXSourceLocation endLocation = clang_getRangeEnd(extent);

    unsigned int startLine = 0, startCol = 0;
    unsigned int endLine = 0, endCol = 0;

    clang_getSpellingLocation(startLocation, nullptr, &startLine, &startCol,
                              nullptr);
    clang_getSpellingLocation(endLocation, nullptr, &endLine, &endCol, nullptr);

    for (int lineNo = 0; lineNo < content.size(); lineNo++) {
      for (int colNo = 0; colNo < content[lineNo].size(); colNo++) {
        bool cond = (lineNo < startLine - 1 || lineNo > endLine - 1) ||
                    (lineNo == startLine - 1 && colNo < startCol - 1) ||
                    (lineNo == endLine - 1 && colNo >= endCol - 1) ||
                    (static_cast<int>(content[lineNo][colNo]) == 10);
        if (!cond) {
          content[lineNo][colNo] = ' ';
        }
      }
    }
  }
  for (auto i : content)
    ofs << i << "\n";

  ofs.close();
  ifs.close();
  rename(tmp.c_str(), dstPath);
}

void ASTManipulation::removeLineRange(const char *srcPath, const char *dstPath,
                                      unsigned int startLine,
                                      unsigned int startCol,
                                      unsigned int endLine,
                                      unsigned int endCol) {
  std::ifstream ifs;
  std::ofstream ofs;

  ifs.open(srcPath);
  std::string tmp = Option::outputDir + "/tmp.txt";
  ofs.open(tmp.c_str(), std::ofstream::out);

  char c;
  int lineNo = 1, colNo = 1;

  while (ifs.get(c)) {
    if (lineNo < startLine || lineNo > endLine) {
      ofs << c;
    } else if (lineNo == startLine && colNo < startCol) {
      ofs << c;
    } else if (lineNo == endLine && colNo > endCol) {
      ofs << c;
    } else if (static_cast<int>(c) == 10) {
      ofs << char(10);
    } else {
      ofs << ' ';
    }

    if (static_cast<int>(c) == 10) {
      lineNo++;
      colNo = 1;
    } else {
      colNo++;
    }
  }

  ofs.close();
  ifs.close();
  rename(tmp.c_str(), dstPath);
}
