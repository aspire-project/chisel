int main(int argc, char **argv) {
  int x = 1;
  int y = 3;
  int i;

  x--;

  for (i = 0; i < 3; i++) {
    y--;
  }
  return x + y;
}
