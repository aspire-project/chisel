int main() {
  char i = 'a';
  float x = 0.2;
  switch (i) {
  case 'a':
    if (x > 0.1)
      printf("%c\n", i);
    break;
  case 'b': {
    for (; !i;)
      printf("not this statement.\n");
  }
  }
  return 0;
}
