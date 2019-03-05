int z(int i, int j) {
  int p;
  p = i + j - 3;
  int k;
  k = i + j + 10;
  return p;
}

int main() {
  int i;
  i = z(6, 4);
  printf("%d\n", i);
  return 0;
}
