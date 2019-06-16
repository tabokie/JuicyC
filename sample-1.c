extern int printf(const char*, int);
int i;
int go(int a) {
  if (a == 1) {
    return 1;
  }
  if (a == 2) {
    return 1;
  }
  return go(a-1) + go(a-2);
}
int main(void) {
  i = go(10);
  printf("%d", i);
  return 0;
}