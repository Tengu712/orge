cpp_args = []
if host_system == 'darwin'
  cpp_args += ['-U_LIBCPP_ENABLE_ASSERTIONS']
  if get_option('buildtype') == 'debug'
    cpp_args += ['-D_LIBCPP_HARDENING_MODE=_LIBCPP_HARDENING_MODE_EXTENSIVE']
  else
    cpp_args += ['-D_LIBCPP_HARDENING_MODE=_LIBCPP_HARDENING_MODE_NONE']
  endif
endif

executable('mesher',
  'main.cpp',
  dependencies: [yaml_cpp_dep],
  cpp_args: cpp_args,
  install: true
)
