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

  std::vector<std::string> &getOriginFilePaths() { return Origins; }
  std::string getTempFileName(std::string Suffix);
  void saveTemp(std::string Phase, bool Status);

private:
  FileManager() {}
  ~FileManager() {}

  static FileManager *Instance;

  std::vector<std::string> Origins;
  int TempCounter = 0;
};

#endif // FILE_MANAGER_H
