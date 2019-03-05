#define FUNCTION_CALL f();
#define X_EQUAL x=
#define ONE 1
#define FUNCTION_DEF void g() { return; }

int f() { return 1; }
FUNCTION_DEF

int main() {
  int x;
  x = FUNCTION_CALL
  X_EQUAL 1;
  return ONE - 1;
}
