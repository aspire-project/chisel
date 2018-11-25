#include "FileManager.h"

#include "llvm/Support/FileSystem.h"

#include "OptionManager.h"

FileManager *FileManager::Instance;

void FileManager::Initialize() {
  Instance = new FileManager();
  Instance->Best = OptionManager::OutputDir + "/best.c";
  Instance->Origin = OptionManager::OutputDir + "/origin.c";

  llvm::sys::fs::create_directory(OptionManager::OutputDir);
  llvm::sys::fs::copy_file(OptionManager::InputFile, Instance->Best);
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
void FileManager::updateBest() {
  llvm::sys::fs::copy_file(OptionManager::InputFile, Best);
}

void FileManager::Finalize() {
  // TODO: implement clean up without using 'awk'
  /*std::string BestClean = OptionManager::OutputDir + "/best_clean.c";

  llvm::sys::fs::rename(BestClean, Best);*/
  llvm::sys::fs::copy_file(Instance->Best, OptionManager::OutputFile);
  llvm::sys::fs::copy_file(Instance->Origin, OptionManager::InputFile);

  delete Instance;
}
