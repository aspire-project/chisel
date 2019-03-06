#ifndef FILE_MANAGER_H
#define FILE_MANAGER_H

#include <string>
#include <vector>

/// \brief Wrapper for low-level file manipulations
class FileManager {
public:
  static void Initialize();
  static void Finalize();
  static FileManager *GetInstance();
  static std::string Readlink(std::string &Name);
  static std::string Dirname(std::string &Name);
  static std::string Basename(std::string &Name);

  std::string getTempFileName(std::string Suffix);
  void saveTemp(std::string Phase, bool Status);

private:
  FileManager() {}
  ~FileManager() {}

  static FileManager *Instance;

  int TempCounter = 0;
};

#endif // FILE_MANAGER_H
