#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <stddef.h>

#include <unistd.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>

bool aml_check(uint64_t const *buffer, size_t len);

int main(int argc, char *argv[]) {
  if (argc < 2) {
    printf("usage: %s <filename>\n", argv[0]);
    return 1;
  }  

  int const fd = open(argv[1], O_RDONLY);
  if (fd == -1) {
    perror("open");
    return 1;
  }

  struct stat sb;
  if (fstat(fd, &sb) == -1){
      perror("fstat");
      return 1;
  }

  void *addr = mmap(NULL, sb.st_size, PROT_READ, MAP_PRIVATE, fd, 0);
  if (addr == MAP_FAILED) {
    perror("mmap");
    return 1;
  }

  assert(((uintptr_t)addr) % sizeof(uint64_t) == 0);
  bool const result = aml_check((uint64_t const *) addr, sb.st_size / sizeof(uint64_t));
  printf("Result: %s\n", result? "true" : "false");
  munmap(addr, sb.st_size);
  close(fd);
  return 0;
}
