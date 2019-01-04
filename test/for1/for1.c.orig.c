int main(void) {
  int x = 1;
  int i;
  for (i = 2; i <= x + 2; i++) {
    printf("%d", i);
  }
  printf("\n");
  for (i = 4; i > 0; i--) x--;
  return 0;
}
