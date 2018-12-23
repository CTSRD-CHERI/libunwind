#include <libunwind.h>
#include <stdio.h>
#include <stdlib.h>

#define fatal(...)                                                             \
  do {                                                                         \
    fprintf(stderr, __VA_ARGS__);                                              \
    abort();                                                                   \
  } while (0)

#ifdef __CHERI__
#define PRINT_PTR "%#p"
#else
#define PRINT_PTR "%p"
#endif

static void check_reg(unw_cursor_t *cursor, const char *name,
                      unw_regnum_t regnum, uintptr_t expected) {
  unw_word_t value;
  int err = unw_get_reg(cursor, regnum, &value);
  if (err != UNW_ESUCCESS) {
    fatal("Failed to get register %s: erro code %d\n", name, err);
  }
  if (value != expected) {
    fatal("Got wrong value for register %s: " PRINT_PTR " != " PRINT_PTR "\n",
          name, (void *)(uintptr_t)value, (void *)(uintptr_t)expected);
  }
  fprintf(stderr, "Register %s has expected value " PRINT_PTR "\n", name,
          (void *)(uintptr_t)expected);
}

#define CHECK_REG(num, expected) check_reg(&cursor, #num, num, expected)

int main() {
  unw_context_t context;
  unw_cursor_t cursor;

  // Call unw_getcontext() once to avoid registers being clobbered by lazy
  // binding resolvers in RTLD.
  int ret = unw_getcontext(&context);
  if (ret != UNW_ESUCCESS)
    fatal("unw_getcontext failed with error code %d", ret);

#ifdef __mips__
  // Fetch the values of hi + lo since they will certainly not be clobbered
  // between the asm volatile and the call to unw_init_local()
  size_t expected_hi = 0x12345678;
  size_t expected_lo = 0x87654321;
  // Setup some registers that we can compare to the values stored in the
  // unw_cursor
  __asm__ volatile("mthi %[hi_value]\n\t"
                   "mtlo %[lo_value]"
                   : // outputs
                   : [hi_value] "r"(expected_hi),
                     [lo_value] "r"(expected_lo) // inputs
                   : "lo", "hi"                  // clobbers
  );
  ret = unw_getcontext(&context);
  if (ret != UNW_ESUCCESS)
    fatal("unw_getcontext failed with error code %d", ret);
  ret = unw_init_local(&cursor, &context);
  if (ret != UNW_ESUCCESS)
    fatal("unw_init_local failed with error code %d", ret);

  CHECK_REG(UNW_MIPS_LO, expected_lo);
  CHECK_REG(UNW_MIPS_HI, expected_hi);
  // This should have been captured as the argument passed to unw_getcontext
#ifdef __CHERI_PURE_CAPABILITY__
  CHECK_REG(UNW_MIPS_C3, (uintptr_t)&context);
#ifdef __CHERI_CAPABILITY_TABLE__
  CHECK_REG(UNW_MIPS_DDC, (uintptr_t)NULL);
#endif
#else
  CHECK_REG(UNW_MIPS_R4, (uintptr_t)&context);
#endif

#elif defined(__x86_64__)
  // Fetch the values of hi + lo since they will certainly not be clobbered
  // between the asm volatile and the call to unw_init_local()
  size_t expected_r14 = 0x87654321;
  size_t expected_r15 = 0x12345678;
  // Setup some registers that we can compare to the values stored in the
  // unw_cursor
  __asm__ volatile("movq %[r14_value], %%r14\n\t"
                   "movq %[r15_value], %%r15\n\t"
                   : // outputs
                   : [r14_value] "X"(expected_r14),
                     [r15_value] "X"(expected_r15) // inputs
                   : "r14", "r15"                // clobbers
  );
  ret = unw_getcontext(&context);
  if (ret != UNW_ESUCCESS)
    fatal("unw_getcontext failed with error code %d", ret);
  ret = unw_init_local(&cursor, &context);
  if (ret != UNW_ESUCCESS)
    fatal("unw_init_local failed with error code %d", ret);
  CHECK_REG(UNW_X86_64_R14, expected_r14);
  CHECK_REG(UNW_X86_64_R15, expected_r15);

#else
#warning "Test not implemented for this architecture"
#endif

  fprintf(stderr, "Success!\n");
  return 0;
}
