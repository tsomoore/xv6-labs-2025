#include "kernel/types.h"
#include "kernel/fcntl.h"
#include "user/user.h"
#include "kernel/riscv.h"
int is_valid_char(char c) {
  if ((c >= 'a' && c <= 'z')||
      (c >= 'A' && c <= 'Z')||
      (c >= '0' && c <= '9')) {
      return 1;
  }
  return 0;
}
int
main(int argc, char *argv[])
{
  // Your code here.
  char *p = sbrk(PGSIZE * 10);
  char buffer[128];
  int buf_idx = 0;

  for (int i = 0; i < PGSIZE*10;i++) {
    char cur = p[i];
    if (is_valid_char(cur)) {
      if (buf_idx < sizeof(buffer) - 1) {
        buffer[buf_idx++] = cur;
      }
    } else {
      if (buf_idx > 1) {
        buffer[buf_idx] = '\0';
        printf("%s\n", buffer);
      }
      buf_idx = 0;
    }
  }
  exit(1);
}
