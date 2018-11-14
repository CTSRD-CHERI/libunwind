include(AddLLVM) # for add_lit_testsuite
macro(pythonize_bool var)
  if (${var})
    set(${var} True)
  else()
    set(${var} False)
  endif()
endmacro()

if (NOT DEFINED LIBCXX_ENABLE_SHARED)
  set(LIBCXX_ENABLE_SHARED ON)
endif()

pythonize_bool(LIBUNWIND_BUILD_32_BITS)
pythonize_bool(LIBCXX_ENABLE_SHARED)
pythonize_bool(LIBUNWIND_ENABLE_SHARED)
pythonize_bool(LIBUNWIND_ENABLE_THREADS)
pythonize_bool(LIBUNWIND_ENABLE_EXCEPTIONS)
pythonize_bool(LIBUNWIND_BUILD_EXTERNAL_THREAD_LIBRARY)
set(LIBUNWIND_TARGET_INFO "libcxx.test.target_info.LocalTI" CACHE STRING
    "TargetInfo to use when setting up test environment.")
set(LIBUNWIND_EXECUTOR "None" CACHE STRING
    "Executor to use when running tests.")

set(AUTO_GEN_COMMENT "## Autogenerated by libunwind configuration.\n# Do not edit!")
configure_file(
  ${CMAKE_CURRENT_SOURCE_DIR}/lit.site.cfg.in
  ${CMAKE_CURRENT_BINARY_DIR}/lit.site.cfg
  @ONLY)

add_lit_testsuite(check-unwind "Running libunwind tests"
  ${CMAKE_CURRENT_BINARY_DIR}
  DEPENDS ${LIBUNWIND_TEST_DEPS}
  )