import shutil
import subprocess
import sys

def install(src, dst):
	print('Installing ' + src + ' to ' + dst)
	shutil.copy(src, dst)

srcdir = sys.argv[1]
dstdir = sys.argv[2]
default_library = sys.argv[3]
vulkan_version = sys.argv[4]

install(srcdir + '/libvulkan.' + vulkan_version + '.dylib', dstdir)

if default_library == 'static':
	install(srcdir + '/libSDL3.a', dstdir)
	install(srcdir + '/libyaml-cpp.a', dstdir)

if default_library == 'shared':
	print('Change liborge.dylib ID to @rpath/liborge.dylib')
	subprocess.run(['install_name_tool', '-id', '@rpath/liborge.dylib', dstdir + '/liborge.dylib'])
