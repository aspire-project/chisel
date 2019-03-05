#include "FileManager.h"

#include <libgen.h>
#include <unistd.h>

#include "llvm/Support/FileSystem.h"

#include "OptionManager.h"

FileManager *FileManager::Instance;

void FileManager::Initialize() {
  Instance = new FileManager();
  llvm::sys::fs::create_directory(OptionManager::OutputDir);
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

std::string FileManager::Readlink(std::string &Name) {
  std::string buffer(64, '\0');
  ssize_t len;
  while ((len = ::readlink(Name.c_str(), &buffer[0], buffer.size())) ==
         static_cast<ssize_t>(buffer.size())) {
    buffer.resize(buffer.size() * 2);
  }
  if (len == -1) {
    return Name;
  }
  buffer.resize(len);
  return buffer;
}

std::string FileManager::Dirname(std::string &Name) {
  char *Cstr = new char[Name.length() + 1];
  strcpy(Cstr, Name.c_str());
  ::dirname(Cstr);
  std::string Dir(Cstr);
  delete[] Cstr;
  return Dir;
}

void FileManager::Finalize() {
  delete Instance;
}
