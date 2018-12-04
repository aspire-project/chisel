#include "FileManager.h"

#include "llvm/Support/FileSystem.h"

#include "OptionManager.h"

FileManager *FileManager::Instance;

void FileManager::Initialize() {
  Instance = new FileManager();
  Instance->Origin = OptionManager::OutputDir + "/origin.c";

  llvm::sys::fs::create_directory(OptionManager::OutputDir);
  llvm::sys::fs::copy_file(OptionManager::InputFile, Instance->Origin);
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
  // TODO: implement clean up without using 'awk'
  /*std::string BestClean = OptionManager::OutputDir + "/best_clean.c";

  llvm::sys::fs::rename(BestClean, Best);*/
  llvm::sys::fs::copy_file(OptionManager::InputFile, OptionManager::OutputFile);
  llvm::sys::fs::copy_file(Instance->Origin, OptionManager::InputFile);

  delete Instance;
}
