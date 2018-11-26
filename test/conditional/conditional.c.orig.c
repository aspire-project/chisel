typedef unsigned int uuint;
struct x {
  int a;
  int b;
  int c;
};
int var1;
int f8(void) { return (0); }
int f0(void) { return (23); }
int f3(int i, int j, int x) { return (1); }
union a {
  int z;
};
int main(void) {
  struct x x_;
  int b;
  b = 12;
  printf("%d\n", f0());
  if (0) {
    f3(1, 1, 1);
  } else {
    f3(5, 5, 5);
    printf("%d\n", f0());
  }
  x_.a = 2;
  while (x_.a < 3) {
    x_.a++;
    printf("%d\n", f0());
    f3(8, 8, 8);
  }
  if (x_.a == 3) {
    printf("%d\n", f0());
    f3(0, 0, 0);
  } else {
    f3(8, 8, 8);
    f3(7, 7, 7);
  }

  goto this_label;
this_label:
  return f0();
  int z;
}
