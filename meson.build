project('orge', 'cpp',
  version : '0.1.0',
  default_options : [
    'cpp_std=c++20',
    'warning_level=3',
    'werror=true',
    'default_library=static',
    # NOTE: Linuxではlibではなくなるので。libに矯正。
    'libdir=lib'
  ]
)

cpp = meson.get_compiler('cpp')
host_system = host_machine.system()

# default_library=bothを禁止
if get_option('default_library') == 'both'
  error('default_library=both is not allowed. Use static or shared.')
endif

# buildtype=releaseとbuildtype=debug以外を禁止
if get_option('buildtype') != 'release' and get_option('buildtype') != 'debug'
  error('buildtype must be release or debug. Current: ' + get_option('buildtype'))
endif

# macOSとLinuxではclangを強制、Windowsではclang-clを強制
if host_system == 'darwin' or host_system == 'linux'
  if not cpp.get_id().contains('clang')
    error('macOS/Linux requires clang compiler. Current: ' + cpp.get_id())
  endif
elif host_system == 'windows'
  if cpp.get_id() != 'clang-cl'
    error('Windows requires clang-cl compiler. Current: ' + cpp.get_id())
  endif
else
  error('Unsupported platform: ' + host_system)
endif

# macOSではarm64を強制
if host_system == 'darwin' and host_machine.cpu_family() != 'aarch64'
  error('macOS requires arm64 architecture. Current: ' + host_machine.cpu_family())
endif

# ソースファイルを列挙
sources = [
  'src/config/config.cpp',
  'src/error/error.cpp',
  'src/graphics/descpool.cpp',
  'src/graphics/framebuffer.cpp',
  'src/graphics/graphics.cpp',
  'src/graphics/image.cpp',
  'src/graphics/pipeline.cpp',
  'src/graphics/renderer.cpp',
  'src/graphics/swapchain.cpp',
  'src/graphics/utils.cpp',
  'src/input/input.cpp',
  'src/orge.cpp'
]

# includeを追加
add_project_arguments('-I' + meson.current_source_dir() / 'include', language: 'cpp')

# 追加のコンパイラフラグを生成
cpp_args = []
if host_system == 'windows'
  if get_option('default_library') == 'static'
    cpp_args += ['/MT']
  else
    cpp_args += ['/MD']
  endif
endif

# macOSでは_LIBCPP_ENABLE_ASSERTIONSを無効化
#
# NOTE: Apple ClangとMesonの齟齬によるもの。
#       https://github.com/mesonbuild/meson/pull/14548 がマージされているので、そのうち治るはず。
if host_system == 'darwin'
  cpp_args += ['-U_LIBCPP_ENABLE_ASSERTIONS']
  if get_option('buildtype') == 'debug'
    cpp_args += ['-D_LIBCPP_HARDENING_MODE=_LIBCPP_HARDENING_MODE_EXTENSIVE']
  else
    cpp_args += ['-D_LIBCPP_HARDENING_MODE=_LIBCPP_HARDENING_MODE_NONE']
  endif
endif

# 依存ライブラリを取得
#
# NOTE: VulkanLoaderがWindowsでのみpcファイルを生成しないせいでpkgconfigを利用できないので、
#       cmakeで取得する。
sdl3_dep = dependency('SDL3', method: 'cmake', required: true, modules: ['SDL3::SDL3-static'])
vulkan_dep = dependency('VulkanLoader', method: 'cmake', required: true)
yaml_cpp_dep = dependency('yaml-cpp', method: 'cmake', required: true)

# 追加の依存ライブラリを取得
#
# NOTE: Windowsにおいてdll作成時にSDL3が依存するライブラリ不足で失敗するので。
sdl3_ful_dep = sdl3_dep
if host_system == 'windows'
  interface_libraries = sdl3_dep.get_variable(cmake: 'INTERFACE_LINK_LIBRARIES', default_value: '')
  interface_libraries = interface_libraries.split(';')
  interface_libraries_founds = []
  foreach n: interface_libraries
    if n.endswith('.lib')
      interface_libraries_founds += cpp.find_library(n.replace('.lib', ''), required: true)
    endif
  endforeach
  sdl3_ful_dep = declare_dependency(dependencies: [sdl3_dep] + interface_libraries_founds)
endif

# ライブラリビルド
orge = library('orge',
  sources,
  cpp_args : cpp_args,
  dependencies : [sdl3_ful_dep, vulkan_dep, yaml_cpp_dep],
  pic: true,
  install : true
)

# include/orge.hをインストール対象に追加
install_headers(['include/orge.h'])

# 依存ライブラリのビルドディレクトリを取得
#
# NOTE: VulkanLoaderがWindowsでのみpcファイルを生成しないせいでpkgconfigを利用できないので、
#       cmake_prefix_pathの設定を強制して直接構築。
cmake_prefix_path = get_option('cmake_prefix_path')
if cmake_prefix_path == []
  error('You must set cmake_prefix_path.')
endif
message('Use cmake_prefix_path: ' + cmake_prefix_path[0])
vcpkg_libdir = cmake_prefix_path[0]
vcpkg_bindir = cmake_prefix_path[0]
if get_option('buildtype') == 'release'
  vcpkg_libdir = vcpkg_libdir / 'lib'
  vcpkg_bindir = vcpkg_bindir / 'bin'
else
  vcpkg_libdir = vcpkg_libdir / 'debug' / 'lib'
  vcpkg_bindir = vcpkg_bindir / 'debug' / 'bin'
endif

# 依存ライブラリコピー用のインストールスクリプトを追加
python = find_program('python', 'python3')
if host_system == 'windows'
  meson.add_install_script(python,
    meson.current_source_dir() / 'meson' / 'install_windows.py',
    vcpkg_libdir,
    vcpkg_bindir,
    get_option('prefix') / get_option('libdir'),
    get_option('prefix') / get_option('bindir'),
    get_option('default_library')
  )
elif host_system == 'darwin'
  meson.add_install_script(python,
    meson.current_source_dir() / 'meson' / 'install_macos.py',
    vcpkg_libdir,
    get_option('prefix') / get_option('libdir'),
    get_option('default_library'),
    vulkan_dep.version()
  )
else
  meson.add_install_script(python,
    meson.current_source_dir() / 'meson' / 'install_linux.py',
    vcpkg_libdir,
    get_option('prefix') / get_option('libdir'),
    get_option('default_library'),
    vulkan_dep.version()
  )
endif

# pkgconfig用の値を作成
libraries = ['-lorge']
cflags = ['-std=c++20']
if host_system == 'windows'
  libraries += ['-lvulkan-1']
elif host_system == 'darwin'
  libraries += ['-lvulkan.' + vulkan_dep.version()]
  cflags += [
    '-Wl,-framework,CoreMedia',
    '-Wl,-framework,CoreVideo',
    '-Wl,-framework,Cocoa',
    '-Wl,-weak_framework,UniformTypeIdentifiers',
    '-Wl,-framework,IOKit',
    '-Wl,-framework,ForceFeedback',
    '-Wl,-framework,Carbon',
    '-Wl,-framework,CoreAudio',
    '-Wl,-framework,AudioToolbox',
    '-Wl,-framework,AVFoundation',
    '-Wl,-framework,Foundation',
    '-Wl,-framework,GameController',
    '-Wl,-framework,Metal',
    '-Wl,-framework,QuartzCore',
    '-Wl,-weak_framework,CoreHaptics',
    '-lpthread',
    '-lm'
  ]
else
  libraries += ['-lvulkan']
endif

# pkgconfigファイルを生成
pc_conf = configuration_data()
pc_conf.set('name', 'orge')
pc_conf.set('description', 'Ore ore game engine.')
pc_conf.set('url', 'https://github.com/Tengu712/orge')
pc_conf.set('version', meson.project_version())
pc_conf.set('libraries', ' '.join(libraries))
pc_conf.set('libraries_private', ' '.join(['-lSDL3', '-lyaml-cpp']))
pc_conf.set('cflags', ' '.join(cflags))
configure_file(
  input : meson.current_source_dir() / 'meson' / 'orge.pc.in',
  output : 'orge.pc',
  configuration : pc_conf,
  install : true,
  install_dir: get_option('libdir') / 'pkgconfig'
)
