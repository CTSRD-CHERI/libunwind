/* ===-- assembly.h - libUnwind assembler support macros -------------------===
 *
 * Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
 * See https://llvm.org/LICENSE.txt for license information.
 * SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
 *
 * ===----------------------------------------------------------------------===
 *
 * This file defines macros for use in libUnwind assembler source.
 * This file is not part of the interface of this library.
 *
 * ===----------------------------------------------------------------------===
 */

#ifndef UNWIND_ASSEMBLY_H
#define UNWIND_ASSEMBLY_H

#if defined(__powerpc64__)
#define SEPARATOR ;
#define PPC64_OFFS_SRR0   0
#define PPC64_OFFS_CR     272
#define PPC64_OFFS_XER    280
#define PPC64_OFFS_LR     288
#define PPC64_OFFS_CTR    296
#define PPC64_OFFS_VRSAVE 304
#define PPC64_OFFS_FP     312
#define PPC64_OFFS_V      824
#ifdef _ARCH_PWR8
#define PPC64_HAS_VMX
#endif
#elif defined(__POWERPC__) || defined(__powerpc__) || defined(__ppc__)
#define SEPARATOR @
#elif defined(__arm64__)
#define SEPARATOR %%
#else
#define SEPARATOR ;
#endif

#define GLUE2(a, b) a ## b
#define GLUE(a, b) GLUE2(a, b)
#define SYMBOL_NAME(name) GLUE(__USER_LABEL_PREFIX__, name)

#if defined(__APPLE__)

#define SYMBOL_IS_FUNC(name)
#define EXPORT_SYMBOL(name)
#define HIDDEN_SYMBOL(name) .private_extern name
#define WEAK_SYMBOL(name) .weak_reference name
#define WEAK_ALIAS(name, aliasname)                                            \
  WEAK_SYMBOL(aliasname) SEPARATOR                                             \
  SYMBOL_NAME(aliasname) = SYMBOL_NAME(name)

#define NO_EXEC_STACK_DIRECTIVE

#elif defined(__ELF__)

#if defined(__arm__)
#define SYMBOL_IS_FUNC(name) .type name,%function
#else
#define SYMBOL_IS_FUNC(name) .type name,@function
#endif
#define EXPORT_SYMBOL(name)
#define HIDDEN_SYMBOL(name) .hidden name
#define WEAK_SYMBOL(name) .weak name
#define WEAK_ALIAS(name, aliasname)                                            \
  WEAK_SYMBOL(aliasname) SEPARATOR                                             \
  SYMBOL_NAME(aliasname) = SYMBOL_NAME(name)

#if defined(__GNU__) || defined(__FreeBSD__) || defined(__Fuchsia__) || \
    defined(__linux__)
#define NO_EXEC_STACK_DIRECTIVE .section .note.GNU-stack,"",%progbits
#else
#define NO_EXEC_STACK_DIRECTIVE
#endif

#elif defined(_WIN32)

#define SYMBOL_IS_FUNC(name)                                                   \
  .def name SEPARATOR                                                          \
    .scl 2 SEPARATOR                                                           \
    .type 32 SEPARATOR                                                         \
  .endef
#define EXPORT_SYMBOL2(name)                                                   \
  .section .drectve,"yn" SEPARATOR                                             \
  .ascii "-export:", #name, "\0" SEPARATOR                                     \
  .text
#if defined(_LIBUNWIND_DISABLE_VISIBILITY_ANNOTATIONS)
#define EXPORT_SYMBOL(name)
#else
#define EXPORT_SYMBOL(name) EXPORT_SYMBOL2(name)
#endif
#define HIDDEN_SYMBOL(name)

#if defined(__MINGW32__)
#define WEAK_ALIAS(name, aliasname)                                            \
  .globl SYMBOL_NAME(aliasname) SEPARATOR                                      \
  EXPORT_SYMBOL(aliasname) SEPARATOR                                           \
  SYMBOL_NAME(aliasname) = SYMBOL_NAME(name)
#else
#define WEAK_ALIAS3(name, aliasname)                                           \
  .section .drectve,"yn" SEPARATOR                                             \
  .ascii "-alternatename:", #aliasname, "=", #name, "\0" SEPARATOR             \
  .text
#define WEAK_ALIAS2(name, aliasname)                                           \
  WEAK_ALIAS3(name, aliasname)
#define WEAK_ALIAS(name, aliasname)                                            \
  EXPORT_SYMBOL(SYMBOL_NAME(aliasname)) SEPARATOR                              \
  WEAK_ALIAS2(SYMBOL_NAME(name), SYMBOL_NAME(aliasname))
#endif

#define NO_EXEC_STACK_DIRECTIVE

#elif defined(__sparc__)

#else

#error Unsupported target

#endif

#if defined(__mips__) && defined(__ELF__)
#define FUNCTION_ENTRY_DIRECTIVE(name) .ent name
#define FUNCTION_END_DIRECTIVE(name) .end name
#else
#define FUNCTION_ENTRY_DIRECTIVE(name)
#define FUNCTION_END_DIRECTIVE(name)
#endif

#define DEFINE_LIBUNWIND_FUNCTION(name)                                        \
  .globl SYMBOL_NAME(name) SEPARATOR                                           \
  FUNCTION_ENTRY_DIRECTIVE(SYMBOL_NAME(name)) SEPARATOR                        \
  HIDDEN_SYMBOL(SYMBOL_NAME(name)) SEPARATOR                                   \
  SYMBOL_IS_FUNC(SYMBOL_NAME(name)) SEPARATOR                                  \
  SYMBOL_NAME(name):

#define END_LIBUNWIND_FUNCTION(name)                                           \
  FUNCTION_END_DIRECTIVE(SYMBOL_NAME(name))

#if defined(__arm__)
#if !defined(__ARM_ARCH)
#define __ARM_ARCH 4
#endif

#if defined(__ARM_ARCH_4T__) || __ARM_ARCH >= 5
#define ARM_HAS_BX
#endif

#ifdef ARM_HAS_BX
#define JMP(r) bx r
#else
#define JMP(r) mov pc, r
#endif
#endif /* __arm__ */

#endif /* UNWIND_ASSEMBLY_H */
