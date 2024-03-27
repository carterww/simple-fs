/* File for testing the primitive functions of the fs. 
 * These include the functions for dentry, vcb, and oft.
 */

#include <ctype.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "simple-fs.h"
#include "vcb.h"

enum test_what {
  TEST_ALL,
  TEST_DENTRY,
  TEST_VCB,
  TEST_OFT
};

static enum test_what test_what = TEST_ALL;

static size_t tests = 0;
static size_t passed_tests = 0;
void assert(int expr, char *msg) {
  ++tests;
  if (expr) {
    ++passed_tests;
    printf("[PASSED]: %s\n", msg);
  } else {
    printf("[FAILED]: %s\n", msg);
  }
}

void get_test(char *arg);
void test_vcb();
void test_oft();
void test_dentry();

void test_vcb() {
  char block[BLOCK_SIZE];
  struct vcb *vcb = (struct vcb *) block;
  vcb_init(vcb, BLOCK_SIZE);
  printf("VCB initialized\n");

  vcb_set_block_free(vcb, 0, 0);
  assert(vcb_get_block_free(vcb, 0) == 0, "VCB -- Superblock not free");

  assert(vcb_free_block_count(vcb) == BLOCK_COUNT - 1, "VCB -- Free block count updated");

  size_t blocks_to_set_used[] = { 5, 12, 16, 7, 511 };
  for (int i = 0; i < sizeof(blocks_to_set_used) / sizeof(size_t); ++i) {
    vcb_set_block_free(vcb, blocks_to_set_used[i], 0);
    char s[128];
    sprintf(s, "VCB -- Block %lu set to used", blocks_to_set_used[i]);
    assert(vcb_get_block_free(vcb, blocks_to_set_used[i]) <= 0, s);
  }
  vcb_set_block_free(vcb, BLOCK_COUNT, 0);
  int free = vcb_get_block_free(vcb, BLOCK_COUNT);
  assert(free == -1 || free == 0, "VCB -- Invalid block number");
}

void test_oft() {

}

void test_dentry() {}

int main(int argc, char *argv[]) {
  if (argc > 1) {
    get_test(argv[1]);
  }
  size_t dentry_tests = 0;
  size_t dentry_passed = 0;
  size_t vcb_tests = 0;
  size_t vcb_passed = 0;
  size_t oft_tests = 0;
  size_t oft_passed = 0;

  switch (test_what) {
    case TEST_ALL:
      test_dentry();
      dentry_tests = tests;
      dentry_passed = passed_tests;
      test_vcb();
      vcb_tests = tests - dentry_tests;
      vcb_passed = passed_tests - dentry_passed;
      test_oft();
      oft_tests = tests - vcb_tests - dentry_tests;
      oft_passed = passed_tests - vcb_passed - dentry_passed;
      break;
    case TEST_DENTRY:
      test_dentry();
      dentry_tests = tests;
      dentry_passed = passed_tests;
      break;
    case TEST_VCB:
      test_vcb();
      vcb_tests = tests;
      vcb_passed = passed_tests;
      break;
    case TEST_OFT:
      test_oft();
      oft_tests = tests;
      oft_passed = passed_tests;
      break;
  }

  printf("=== TEST RESULTS ===\n");
  printf("Dentry tests:    %lu/%lu\n", dentry_passed, dentry_tests);
  printf("VCB tests:       %lu/%lu\n", vcb_passed, vcb_tests);
  printf("OFT tests:       %lu/%lu\n", oft_passed, oft_tests);
  printf("Total tests:     %lu/%lu\n", passed_tests, tests);

  return 0;
}

void get_test(char *arg) {
  size_t len = strlen(arg);
  char *arg_lower = malloc(len + 1);

  for (size_t i = 0; i < len; i++) {
    arg_lower[i] = tolower(arg[i]);
  }
  arg_lower[len] = '\0';
  if(strstr(arg, "oft")) {
    test_what = TEST_OFT;
  } else if(strstr(arg, "vcb")) {
    test_what = TEST_VCB;
  } else if(strstr(arg, "dentry")) {
    test_what = TEST_DENTRY;
  } else {
    test_what = TEST_ALL;
  }
  free(arg_lower);
}
