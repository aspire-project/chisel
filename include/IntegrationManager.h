#ifndef INTEGRATION_MANAGER_H
#define INTEGRATION_MANAGER_H

#include <map>
#include <string>
#include <vector>

/// \brief Manages build system integration
class IntegrationManager {
public:
  static void Initialize();
  static void Finalize();
  static IntegrationManager *GetInstance() { return Instance; }

  void capture();
  std::vector<std::string> &getOriginFilePaths() { return Origins; }
  std::vector<const char *> getCC1Args(std::string &FileName);

  std::map<std::string, std::vector<std::string>> CompilationDB;

private:
  static IntegrationManager *Instance;
  std::vector<std::string> Origins;
  std::string WrapperDir;

  void buildCompilationDB();

  std::string CaptureFilePath;
};

#endif // INTEGRATION_MANAGER_H
