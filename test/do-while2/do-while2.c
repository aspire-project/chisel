int main(void) {
  int a = 5;
  int x = 5;

  do {
    x--;
    do {
      x = 2;
    } while (x > 10);
    x = 0;
    printf("15\n");
  } while (x);

  return 0;
}
