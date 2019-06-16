extern int printf(const char*, int, int, int);
int func(void) { return 0; }
int func(int a) { return 1; }
int func(int a, float b) { return 2; }
int main(void) {
  int a = func();
  int b = func(1);
  int c = func(1,1);
  printf("%d, %d, %d", a, b, c);
  return 0;
}