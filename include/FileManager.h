#ifndef FILE_MANAGER_H
#define FILE_MANAGER_H

#include <string>

class FileManager {
public:
  static void Initialize();
  static void Finalize();
  static FileManager *GetInstance();

  std::string &getOriginFilePath() { return Origin; }
  std::string getTempFileName(std::string Suffix);
  void saveTemp(std::string Phase, bool Status);

private:
  FileManager() {}
  ~FileManager() {}

  static FileManager *Instance;

  std::string Origin;
  int TempCounter = 0;
};

#endif // FILE_MANAGER_H
