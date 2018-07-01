#include <clang-c/Index.h>

#include <algorithm>
#include <cstring>
#include <iostream>
#include <iterator>
#include <map>
#include <vector>

#include "cursor_utils.h"
#include "rules.h"

bool Rule::isTypeDependencyBroken(std::vector<CXCursor> allCursors,
                                  std::vector<CXCursor> cursors) {
  // cursors: things to be removed from allCursors
  std::vector<CXCursor> remainingCursors =
      CursorUtils::subtract(allCursors, cursors);
  for (auto c : remainingCursors) {
    CXCursor ref = clang_getCursorReferenced(c);

    if (clang_equalCursors(c, ref))
      continue;
    if (clang_isInvalid(ref.kind))
      continue;

    if (CursorUtils::contains(cursors, ref))
      return true;
  }

  return false;
}

bool Rule::isUninitialized(std::vector<CXCursor> allCursors) {
  std::vector<CXCursor> defs;
  std::vector<CXCursor> uses;
  for (auto c : allCursors) {
    if (c.kind == CXCursor_DeclRefExpr) {
      auto d = clang_getCursorDefinition(c);
      if (d.kind == CXCursor_VarDecl)
        uses.emplace_back(d);
    } else if (c.kind == CXCursor_VarDecl) {
      defs.emplace_back(c);
    }
  }
  auto diff = CursorUtils::subtract(uses, defs);

  if (diff.size() == 0)
    return false;
  return true;
}

bool Rule::containsOneReturn(std::vector<CXCursor> subset, CXCursor func,
                             std::vector<CXCursor> allFunctionElems) {
  auto funcName = clang_getCursorSpelling(func);
  if (strcmp(clang_getCString(funcName), "main") == 0) {
    clang_disposeString(funcName);
    return true;
  }
  clang_disposeString(funcName);
  CXType f = clang_getCursorResultType(func);
  if (f.kind == CXType_Void)
    return true;
  auto remain = CursorUtils::subtract(allFunctionElems, subset);
  for (auto c : remain)
    if (clang_getCursorKind(c) == CXCursor_ReturnStmt)
      return true;
  return false;
}

bool Rule::mainExists(std::vector<CXCursor> cursors) {
  for (auto cursor : cursors) {
    if (cursor.kind == CXCursor_FunctionDecl) {
      auto funcName = clang_getCursorSpelling(cursor);
      if (strcmp(clang_getCString(funcName), "main") == 0) {
        clang_disposeString(funcName);
        return true;
      }
      clang_disposeString(funcName);
    }
  }
  return false;
}

bool isFuncBuiltin(CXCursor cursor) {
  if (clang_getCursorKind(cursor) == CXCursorKind::CXCursor_FunctionDecl)
    return false;

  std::string funcName = CursorUtils::getCursorSpelling(cursor);

  std::string implicitFunctions[] = {"__assert_fail",
                                     "__ctype_b_loc",
                                     "__ctype_get_mb_cur_max",
                                     "_exit",
                                     "__errno_location",
                                     "abort",
                                     "atexit",
                                     "bindtextdomain",
                                     "c_tolower",
                                     "calloc",
                                     "chmod",
                                     "chown",
                                     "clear_ungetc_buffer_preserving_position",
                                     "close",
                                     "closedir",
                                     "error",
                                     "exit",
                                     "fchmod",
                                     "fchown",
                                     "fclose",
                                     "fcntl",
                                     "ferror_unlocked",
                                     "feof_unlocked",
                                     "fflush",
                                     "fileno",
                                     "fprintf",
                                     "fputc",
                                     "fputc_unlocked",
                                     "fputs",
                                     "fputs_unlocked",
                                     "free",
                                     "fwrite",
                                     "fwrite_unlocked",
                                     "getc_unlocked",
                                     "getenv",
                                     "getpwuid",
                                     "gettext",
                                     "gettimeofday",
                                     "malloc",
                                     "memchr",
                                     "mbsinit",
                                     "mbrtowc",
                                     "memcmp",
                                     "memcpy",
                                     "memset",
                                     "mkdir",
                                     "nl_langinfo",
                                     "open",
                                     "printf",
                                     "putchar_unlocked",
                                     "quote_n",
                                     "quotearg_char",
                                     "quotearg_char_mem",
                                     "quotearg_n_options",
                                     "readdir",
                                     "realloc",
                                     "savewd_restore",
                                     "setlocale",
                                     "snprintf",
                                     "strchr",
                                     "strcmp",
                                     "strcpy",
                                     "strftime",
                                     "strlen",
                                     "strncmp",
                                     "strrchr",
                                     "strtoul",
                                     "textdomain",
                                     "tolower",
                                     "toupper",
                                     "towlower",
                                     "xnmalloc"};

  for (auto i : implicitFunctions) {
    if (strcmp(i.c_str(), funcName.c_str()) == 0) {
      return true;
    }
  }
  return false;
}
