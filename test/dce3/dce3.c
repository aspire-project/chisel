void empty1() {
  {}
}

void empty2(int x) {
  {}
}

int main() {
  int x;
  {
    {
      {
        x = 0;
      }
    }
  }
  return x;
}
