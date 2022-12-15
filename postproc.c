#include <stdio.h>
#include <stdlib.h>
#include <inttypes.h>

int main()
{
    int64_t asd=0xff;
  union {
    char bytes[4];
    int val;
  } test;

  test.bytes[0] = 0;
  test.bytes[1] = 0;
  test.bytes[2] = 0;
  test.bytes[3] = asd;
    printf("%d\n",test.val==asd);
  return test.val == 0xff;
}