int main() {

  int y = 10;
  // Compilers such as clang-7 raise warning for "int i" in for-statement.
  // DCE module should distinguish it from unsed-variable warnings
  for (int i = 0; i < 10; i++) {
    y--;
  }

  return y;
}
