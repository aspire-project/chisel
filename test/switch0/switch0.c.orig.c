int main(int argc, char **argv) {
  int x = 0;
  int y = 3;
  switch(y) {
    case 0:
      y = 3;
      break;
    case 2:
      y = 1;
      break;
    case 3:
      y = 0;
      break;
    default:
      y = 10;
  }
  return x + y;
}
