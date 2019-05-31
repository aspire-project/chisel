#include "clang/AST/ASTContext.h"
#include "clang/Frontend/FrontendActions.h"
#include "clang/Tooling/JSONCompilationDatabase.h"
#include "clang/Tooling/Tooling.h"
#include "llvm/Support/CommandLine.h"

#include <clang/Frontend/CompilerInstance.h>
#include <llvm/ADT/StringSwitch.h>
#include <llvm/Support/Path.h>

#include <fstream>

class Visualization {
public:
  void generate(std::string Filename);

private:
  std::string getStdoutFromCommand(std::string Command);
  std::vector<std::string> ParseNumbers(std::string String);
  void Save(std::vector<std::string> Numbers, std::string Filename);
};
