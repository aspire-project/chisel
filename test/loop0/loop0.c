int main(int argc, char **argv) {
  int x = 0;
  int y = 0;

  while(argc > 2) {
    y = 2; /* comment to be removed */
  }

  while(argc > 2)
    y = 1  /* comment to be removed */   ;

  return x;
}
