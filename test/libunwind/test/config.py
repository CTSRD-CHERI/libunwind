#===----------------------------------------------------------------------===##
#
# Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
# See https://llvm.org/LICENSE.txt for license information.
# SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
#
#===----------------------------------------------------------------------===##
import os
import sys

from libcxx.test.config import Configuration as LibcxxConfiguration


class Configuration(LibcxxConfiguration):
    # pylint: disable=redefined-outer-name
    def __init__(self, lit_config, config):
        super(Configuration, self).__init__(lit_config, config)
        self.libunwind_src_root = None
        self.libunwind_obj_root = None
        self.abi_library_path = None
        self.libcxx_src_root = None
        self.default_cxx_abi_library = None  # set later

    def configure_src_root(self):
        self.libunwind_src_root = (self.get_lit_conf('libunwind_src_root')
            or os.path.dirname(self.config.test_source_root))
        self.libcxx_src_root = (self.get_lit_conf('libcxx_src_root')
            or os.path.join(self.libunwind_src_root, '..', 'libcxx'))

    def configure_obj_root(self):
        self.libunwind_obj_root = self.get_lit_conf('libunwind_obj_root')
        super(Configuration, self).configure_obj_root()

    def has_cpp_feature(self, feature, required_value):
        return int(self.cxx.dumpMacros().get('__cpp_' + feature, 0)) >= required_value

    def configure_features(self):
        super(Configuration, self).configure_features()
        if self.get_lit_bool('arm_ehabi', False):
            self.config.available_features.add('libunwind-arm-ehabi')

    def configure_compile_flags(self):
        self.cxx.compile_flags += ['-DLIBUNWIND_NO_TIMER']
        # Stack unwinding tests need unwinding tables and these are not
        # generated by default on all Targets.
        self.cxx.compile_flags += ['-funwind-tables']
        if not self.get_lit_bool('enable_threads', True):
            self.cxx.compile_flags += ['-D_LIBUNWIND_HAS_NO_THREADS']
            self.config.available_features.add('libunwind-no-threads')
        super(Configuration, self).configure_compile_flags()

    def configure_cxx_stdlib_under_test(self):
        # We are always running libunwind against c++ library. Currently,
        # all the tests only use the C++ ABI library so if possible we avoid
        # linking against the full C++ standard library to avoid potentially
        # pulling in another copy of libunwind.
        self.cxx_stdlib_under_test = 'none'

    def configure_link_flags(self):
        # Ensure that the currently built libunwind is be the first library
        # in the search order. This is especially important for static linking.
        if self.link_shared:
            # dladdr needs libdl on Linux
            self.cxx.link_flags += ['-lunwind', '-ldl']
        else:
            libname = self.make_static_lib_name('unwind')
            abs_path = os.path.join(self.libunwind_obj_root, "lib", libname)
            assert os.path.exists(abs_path) and "static libunwind library does not exist", abs_path
            self.cxx.link_flags += [abs_path]

        super(Configuration, self).configure_link_flags()
        # Ensure that libunwind is always added to the linker flags
        # This should be the case for most TargetInfo classes anyway but some
        # of them don't add it.
        self.cxx.link_flags += ['-nodefaultlibs', '-lc']
        # If we are building a static binary, we also need to add the compiler
        # buitins library to the build since we are building with -nodefaultlibs.
        if self.force_static_executable:
            assert '-static' in self.cxx.link_flags  # should be set by super()
            rtlib = self.abi_library_root = self.get_lit_conf('runtime_library')
            if not rtlib:
                rtlib = self.cxx.getRtlibPath()
            if not os.path.exists(rtlib):
                self.lit_config.fatal("Cannot find compiler runtime library" +
                                      " at expected path " + rtlib + ". Set" +
                                      " -Druntime_library=.... to fix this.")
            self.cxx.link_flags += [rtlib]

        print("LINKER FLAGS:", self.cxx.link_flags)

    def configure_compile_flags_header_includes(self):
        self.configure_config_site_header()

        libunwind_headers = self.get_lit_conf(
            'libunwind_headers',
            os.path.join(self.libunwind_src_root, 'include'))
        if not os.path.isdir(libunwind_headers):
            self.lit_config.fatal("libunwind_headers='%s' is not a directory."
                                  % libunwind_headers)
        self.cxx.compile_flags += ['-I' + libunwind_headers]

    def configure_compile_flags_exceptions(self):
        pass

    def configure_compile_flags_rtti(self):
        pass

    def configure_link_flags_cxx_library(self):
        # libunwind tests should not link with libc++
        pass

    def configure_link_flags_abi_library(self):
        # libunwind tests should not link with libc++abi
        pass
