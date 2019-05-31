#include "Visualization.h"

#include <stdlib.h>
#include <iostream>
#include <sstream>

#include "llvm/Support/Process.h"
#include "llvm/Support/Program.h"

#include "OptionManager.h"

void Visualization::generate(std::string Filename) {
  // Reformat both files
  llvm::ErrorOr<std::string> ClangFormat =
      llvm::sys::findProgramByName("clang-format");
  if (ClangFormat) {
    std::ofstream ClangConfig(".clang-format");
    ClangConfig
        << "AlwaysBreakAfterReturnType: All\nBreakBeforeBraces: Allman\n";
    ClangConfig.close();
    llvm::sys::ExecuteAndWait(
        ClangFormat.get(),
        {ClangFormat.get(), "-i", Filename, Filename + ".chisel.c"});
  } else
    llvm::errs() << "Cannot find clang-format. The visualization result might "
                    "not be precise.\n";

  // Call the oracle to record covered lines
  int Status = llvm::sys::ExecuteAndWait(OptionManager::OracleFile,
                                         {OptionManager::OracleFile, "1"});

  // Find and record the common line numbers
  std::string Diff =
      "diff --unchanged-line-format=$'\%dn\\n' --new-line-format='' "
      "--old-line-format='' --ignore-space-change " +
      Filename + " " + Filename + ".chisel.c";
  Save(ParseNumbers(getStdoutFromCommand(Diff)), Filename + ".common");

  // If the Generator exists, generate HTML cross-referencing
  llvm::ErrorOr<std::string> Generator =
      llvm::sys::findProgramByName("generator");
  if (Generator)
    llvm::sys::ExecuteAndWait(Generator.get(),
                              {Generator.get(), "-o", "xref-output", "-p",
                               Filename + ":.", Filename, "--",
                               "-I/opt/llvm/include"});
  else {
    llvm::errs()
        << "Cannot find generator. The visualization phase is incomplete.\n";
    return;
  }

  // Copy static data for HTML
  std::string StaticPath =
      Generator.get().substr(0, Generator.get().find_last_of("\\/")) +
      "/../../data";
  llvm::sys::ExecuteAndWait("/bin/cp",
                            {"/bin/cp", "-r", StaticPath, "xref-output/../"});
}

void Visualization::Save(std::vector<std::string> Numbers,
                         std::string Filename) {
  std::ofstream OutputFile(Filename);
  for (const auto &N : Numbers)
    OutputFile << N << "\n";
}

std::vector<std::string> Visualization::ParseNumbers(std::string String) {
  std::istringstream iss(String);
  std::string Token;
  std::vector<std::string> Numbers;

  while (std::getline(iss, Token, '$')) {
    if (Token.size() <= 2)
      continue;
    Numbers.emplace_back(Token.substr(0, Token.size() - 2));
  }
  return Numbers;
}

std::string Visualization::getStdoutFromCommand(std::string Command) {
  std::string Data;
  FILE *Stream;
  char Buffer[256];
  Command.append(" 2>&1");

  Stream = popen(Command.c_str(), "r");
  if (Stream) {
    while (!feof(Stream))
      if (fgets(Buffer, sizeof(Buffer), Stream) != NULL)
        Data.append(Buffer);
    pclose(Stream);
  }
  return Data;
}
