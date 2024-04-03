#include "simple-fs.h"

#include <pthread.h>
#include <stdio.h>

void *p1_thread(void *arg) {
  // Create file 1 and write to it
  create("file1", 1);
  int fd = open("file1", 0);
  if (fd == -1) {
    printf("Failed to open file1\n");
    return NULL;
  }
  char buf[6] = "hello";
  write(fd, buf, sizeof(buf));
  close(fd);
  // Create file 2 and write to it
  create("file2", 1);
  fd = open("file2", 0);
  if (fd == -1) {
    printf("Failed to open file2\n");
    return NULL;
  }
  char buf2[6] = "world";
  write(fd, buf2, sizeof(buf2));
  close(fd);
  return NULL;
}

void *p2_and_p3_thread(void *arg) {
  char *file_name = (char *)arg;
  // Open file 1, read from it, print it, and close it
  int fd = open(file_name, 0);
  if (fd == -1) {
    printf("Failed to open %s\n", file_name);
    return NULL;
  }
  char buf[BLOCK_SIZE];
  read(fd, buf, sizeof(buf));
  printf("%s: %s\n", file_name, buf);
  close(fd);
  return NULL;
}

/* Example usage of the simple-fs implementation.
 * This example is based on the description of the
 * simulation as described in Part 2 of the rubric.
 */
int main(void) {
  // Initialize the file system
  init_fs();
  pthread_t p1, p2, p3;
  pthread_create(&p1, NULL, p1_thread, NULL);
  pthread_join(p1, NULL);
  char *p2_file = "file1";
  char *p3_file = "file2";
  pthread_create(&p2, NULL, p2_and_p3_thread, p2_file);
  pthread_create(&p3, NULL, p2_and_p3_thread, p3_file);
  pthread_join(p2, NULL);
  pthread_join(p3, NULL);
}
