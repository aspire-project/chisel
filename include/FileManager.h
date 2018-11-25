#ifndef FILE_MANAGER_H
#define FILE_MANAGER_H

#include <string>

class FileManager {
public:
  static void Initialize();
  static void Finalize();
  static FileManager *GetInstance();

  std::string getTempFileName(std::string Suffix);
  void updateBest();

private:
  FileManager() {}
  ~FileManager() {}

  static FileManager *Instance;

  std::string Best;
  std::string Origin;
  int TempCounter = 0;
};

#endif // FILE_MANAGER_H
