extern int printf(const char*, int);
int ans;
int gcd(int a, int b) {
  if (b == 0) {
    return a;
  } else {
    return gcd(b, a % b);
  }
}
int main(void) {
  ans = gcd(9, 36) * gcd(6, 12);
  printf("%d", ans);
  return 0;
}