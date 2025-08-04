# =================================================================================================================== #
#     Project Settings                                                                                                #
# =================================================================================================================== #

project('orge', 'cpp',
  version : '0.1.0',
  default_options : [
    'cpp_std=c++20',
    'warning_level=3',
    'werror=true',
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

# macOSとLinuxではclangを強制
if host_system == 'darwin' or host_system == 'linux'
  if not cpp.get_id().contains('clang')
    error('macOS/Linux requires clang compiler. Current: ' + cpp.get_id())
  endif
endif

# macOSではarm64を強制
if host_system == 'darwin' and host_machine.cpu_family() != 'aarch64'
  error('macOS requires arm64 architecture. Current: ' + host_machine.cpu_family())
endif

# =================================================================================================================== #
#     Build Settings                                                                                                  #
# =================================================================================================================== #

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
    if get_option('buildtype') == 'release'
      cpp_args += ['/MT', '-DORGE_BUILD_STATIC']
    else
      cpp_args += ['/MTd', '-DORGE_BUILD_STATIC']
    endif
  else
    if get_option('buildtype') == 'release'
      cpp_args += ['/MD', '-DORGE_BUILD_SHARED']
    else
      cpp_args += ['/MDd', '-DORGE_BUILD_SHARED']
    endif
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
sdl3_dep = dependency('SDL3', method: 'cmake', static: true, required: true)
vulkan_dep = dependency('VulkanLoader', method: 'cmake', required: true)
yaml_cpp_dep = dependency('yaml-cpp', method: 'cmake', required: true)

# 追加の依存ライブラリを取得
#
# NOTE: Windowsにおいてdll作成時にSDL3が依存するライブラリ不足で失敗するので。
sdl3_ful_dep = sdl3_dep
if host_system == 'windows'
  deps = [
    cpp.find_library('winmm', required: true),
    cpp.find_library('setupapi', required: true),
    cpp.find_library('cfgmgr32', required: true),
    cpp.find_library('version', required: true),
    cpp.find_library('imm32', required: true)
  ]
  sdl3_ful_dep = declare_dependency(dependencies: [sdl3_dep] + deps)
endif

# prefixとsufixを指定
#
# NOTE: MesonではたといWindowsであってもlibxxx.aという命名でライブラリを作る。
#       ので、prefixとsufixを明示的に指定してxxx.libにする。
#       https://mesonbuild.com/FAQ.html#why-does-building-my-project-with-msvc-output-static-libraries-called-libfooa
prefix = []
suffix = []
if host_system == 'windows'
  prefix = ''
  if get_option('default_library') == 'static'
    suffix = 'lib'
  else
    suffix = 'dll'
  endif
endif

# ライブラリビルド
orge = library('orge',
  sources,
  cpp_args: cpp_args,
  dependencies : [sdl3_ful_dep, vulkan_dep, yaml_cpp_dep],
  pic: true,
  install: true,
  name_prefix: prefix,
  name_suffix: suffix
)

# =================================================================================================================== #
#     Install Settings                                                                                                #
# =================================================================================================================== #

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
    meson.current_source_dir() / 'scripts' / 'install_windows.py',
    vcpkg_libdir,
    vcpkg_bindir,
    get_option('prefix') / get_option('libdir'),
    get_option('prefix') / get_option('bindir'),
    get_option('default_library')
  )
elif host_system == 'darwin'
  meson.add_install_script(python,
    meson.current_source_dir() / 'scripts' / 'install_macos.py',
    vcpkg_libdir,
    get_option('prefix') / get_option('libdir'),
    get_option('default_library'),
    vulkan_dep.version()
  )
else
  meson.add_install_script(python,
    meson.current_source_dir() / 'scripts' / 'install_linux.py',
    vcpkg_libdir,
    get_option('prefix') / get_option('libdir'),
    get_option('default_library'),
    vulkan_dep.version()
  )
endif

# =================================================================================================================== #
#     Pkgconfig Settings                                                                                              #
# =================================================================================================================== #

# pkgconfig用の値を作成
libraries = ['-lorge']
libraries_private = ['-lyaml-cpp']
cflags = ['-std=c++20']
cflags_private = []
if host_system == 'windows'
  libraries += ['-lvulkan-1']
  libraries_private += ['-lSDL3-static']
  cflags += ['-DORGE_BUILD_SHARED']
  cflags_private += ['-UORGE_BUILD_SHARED', '-DORGE_BUILD_STATIC']
elif host_system == 'darwin'
  libraries += ['-lvulkan.' + vulkan_dep.version()]
  libraries_private += ['-lSDL3']
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
  libraries_private += ['-lSDL3']
endif

# pkgconfigファイルを生成
pc_conf = configuration_data()
pc_conf.set('name', 'orge')
pc_conf.set('description', 'Ore ore game engine.')
pc_conf.set('url', 'https://github.com/Tengu712/orge')
pc_conf.set('version', meson.project_version())
pc_conf.set('libraries', ' '.join(libraries))
pc_conf.set('libraries_private', ' '.join(libraries_private))
pc_conf.set('cflags', ' '.join(cflags))
pc_conf.set('cflags_private', ' '.join(cflags_private))
configure_file(
  input: meson.current_source_dir() / 'orge.pc.in',
  output: 'orge.pc',
  configuration: pc_conf,
  install: true,
  install_dir: get_option('libdir') / 'pkgconfig'
)
