#include "FileManager.h"

#include "llvm/Support/FileSystem.h"

#include "OptionManager.h"

FileManager *FileManager::Instance;

void FileManager::Initialize() {
  Instance = new FileManager();
  llvm::sys::fs::create_directory(OptionManager::OutputDir);

  for (auto &File : OptionManager::InputFiles) {
    std::string Origin = OptionManager::OutputDir + "/" + File + ".origin.c";
    Instance->Origins.push_back(Origin);
    llvm::sys::fs::copy_file(File, Origin);
  }
}

FileManager *FileManager::GetInstance() {
  if (!Instance)
    Initialize();
  return Instance;
}

std::string FileManager::getTempFileName(std::string Suffix) {
  std::string Name = OptionManager::OutputDir + "/" + OptionManager::InputFile +
                     "." + std::to_string(TempCounter) + "." + Suffix;
  TempCounter++;
  return Name;
}

void FileManager::saveTemp(std::string Phase, bool Status) {
  if (OptionManager::SaveTemp) {
    std::string StatusString = Status ? ".success.c" : ".fail.c";
    llvm::sys::fs::copy_file(OptionManager::InputFile,
                             getTempFileName(Phase) + StatusString);
  }
}

void FileManager::Finalize() {
  for (auto &File : OptionManager::InputFiles) {
    std::string Origin = OptionManager::OutputDir + "/" + File + ".origin.c";
    llvm::sys::fs::copy_file(File, File + ".chisel.c");
    llvm::sys::fs::copy_file(Origin, File);
  }
  delete Instance;
}
