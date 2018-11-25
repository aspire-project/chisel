#include <string>
#include <vector>

class StringUtils {
public:
  static std::vector<std::string> splitBy(std::string str, char c);
  static std::string replaceFirst(std::string &s, const std::string &toReplace,
                                  const std::string &replaceWith);
  static std::string replaceLast(std::string &s, const std::string &toReplace,
                                 const std::string &replaceWith);
  static bool contains(std::string str, std::string what);
  static std::string placeholder(std::string str);
};
