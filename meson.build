project('orge', 'cpp',
  version : '0.1.0',
  default_options : [
    'cpp_std=c++20',
    'warning_level=3',
    'werror=true',
    'default_library=static'
  ]
)

cpp = meson.get_compiler('cpp')
host_system = host_machine.system()

# default_library=bothを禁止
if get_option('default_library') == 'both'
  error('default_library=both is not allowed. Use static or shared.')
endif

# clang系を強制
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
if host_system == 'darwin' or host_system == 'linux'
  cpp_args += ['-fPIC']
elif host_system == 'windows'
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
sdl3_dep = dependency('sdl3', required: true)
vulkan_dep = dependency('vulkan', required: true)
yaml_cpp_dep = dependency('yaml-cpp', required: true)

# ライブラリビルド
orge = library('orge',
  sources,
  cpp_args : cpp_args,
  dependencies : [sdl3_dep, vulkan_dep, yaml_cpp_dep],
  install : true
)

# include/orge.hをインストール対象に追加
install_headers(['include/orge.h'])

# 依存ライブラリのビルドディレクトリを取得
#
# NOTE: どのライブラリも同じディレクトリに存在するはずなので、vulkanから訊き出してそれを使う。
vcpkg_libdir = vulkan_dep.get_variable('libdir', default_value: '')
vcpkg_bindir = vulkan_dep.get_variable('bindir', default_value: '')

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
    get_option('default_library')
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

# pkgconfig用のextra_cflagsを作成
extra_cflags = ['-std=c++20']
if host_system == 'windows'
  extra_cflags += ['-lvulkan-1']
elif host_system == 'darwin'
  extra_cflags += [
    '-lvulkan.' + vulkan_dep.version(),
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
  extra_cflags += ['-lvulkan']
endif

# pkgconfigファイルを生成
pkg = import('pkgconfig')
pkg.generate(
  name: 'orge',
  description: 'Ore ore game engine.',
  url: 'https://github.com/Tengu712/orge',
  version: meson.project_version(),
  libraries: ['-L${libdir}', '-lorge'],
  libraries_private: ['-lSDL3', '-lyaml-cpp'],
  extra_cflags: extra_cflags
)
