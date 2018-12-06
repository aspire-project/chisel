

int f0(void) { return (23); }

int main(void) {

  printf("%d\n", f0());

this_label:
  return f0();
}
