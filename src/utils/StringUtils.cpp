#include "StringUtils.h"
#include <sstream>
#include <string>
#include <vector>

std::vector<std::string> StringUtils::splitBy(std::string str, char c) {
  std::stringstream strStream(str);
  std::string segment;
  std::vector<std::string> segList;
  while (std::getline(strStream, segment, c)) {
    segList.push_back(segment);
  }
  return segList;
}

std::string StringUtils::replaceFirst(std::string &s,
                                      const std::string &toReplace,
                                      const std::string &replaceWith) {
  std::size_t pos = s.find(toReplace);
  if (pos == std::string::npos)
    return s;
  return s.replace(pos, toReplace.length(), replaceWith);
}

std::string StringUtils::replaceLast(std::string &s,
                                     const std::string &toReplace,
                                     const std::string &replaceWith) {
  std::size_t pos = s.rfind(toReplace);
  if (pos == std::string::npos)
    return s;
  return s.replace(pos, toReplace.length(), replaceWith);
}

bool StringUtils::contains(std::string str, std::string what) {
  return (str.find(what) != std::string::npos);
}

std::string StringUtils::placeholder(std::string str) {
  std::string replacement = "";
  for (auto const &chr : str) {
    if (chr == '\n')
      replacement += '\n';
    else if (isprint(chr))
      replacement += " ";
    else
      replacement += chr;
  }
  return replacement;
}
