int main(int argc, char** argv) {
  int x;
  if (argc == 1) {
    goto case1;
  }
  if (argc == 9) {
    goto case9;
  }
  case1:
    x = 0;
    goto breaklabel;
  case9:
    x = 1;
    goto breaklabel;
  breaklabel:
    return x;
}
