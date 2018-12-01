int main(int argc, char **argv) {
  int x = 0;
  int y = 0;

  if(argc > 2) {
    y = 1;
  } else {
    y = 2; /* comment to be removed */
  }

  if(argc > 2)
    y = 1  /* comment to be removed */   ;
  else
    y = 2  /* comment to be removed */      ;

  return x;
}
