extern int printf(const char*, int, int);
int f;
int k;
int go(int b, int a) {
  int ret;
  int fk;
  float t;
  if (a > 0) {
    ret = a * go(b, a-1);
  } else {
    ret = 1;
  }
  b += ret;
  k += ret;
  return ret;
}
int main(void) {
  k = 0;
  f = go(k, 5);
  printf("%d, %d", f, k);
  return 0;
}