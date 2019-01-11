typedef unsigned int uuint;
struct x {
  int a;
  int b;
  int c;
};

int f0(void) { return (23); }

union a {
  int z;
};
int main(void) {

  printf("%d\n", f0());

  return f0();
}
