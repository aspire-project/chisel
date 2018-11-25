#include <vector>

class VectorUtils {
public:
  template <typename T>
  static std::vector<std::vector<T>> split(std::vector<T> &vec, int n) {
    std::vector<std::vector<T>> result;
    int length = static_cast<int>(vec.size()) / n;
    int remain = static_cast<int>(vec.size()) % n;

    int begin = 0, end = 0;
    for (int i = 0; i < std::min(n, static_cast<int>(vec.size())); ++i) {
      end += (remain > 0) ? (length + !!(remain--)) : length;
      result.emplace_back(
          std::vector<T>(vec.begin() + begin, vec.begin() + end));
      begin = end;
    }
    return result;
  }

  template <typename T> static bool contains(std::vector<T> &vec, T d) {
    for (auto const &v : vec)
      if (v == d)
        return true;
    return false;
  }

  template <typename T>
  static std::vector<T> difference(std::vector<T> &a, std::vector<T> &b) {
    // a - b
    std::vector<T> minus;
    if (a.size() == 0)
      return minus;
    if (b.size() == 0)
      return a;
    for (auto const &d : a) {
      if (!VectorUtils::contains<T>(b, d))
        minus.emplace_back(d);
    }
    return minus;
  }
};
