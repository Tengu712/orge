import glob
import shutil
import subprocess
import sys

def install(src, dst):
	for n in glob.glob(src):
		print('Installing ' + n + ' to ' + dst)
		shutil.copy(n, dst)

srcdir = sys.argv[1]
dstdir = sys.argv[2]
default_library = sys.argv[3]

install(srcdir + '/libvulkan.1.*.dylib', dstdir)

if default_library == 'static':
	install(srcdir + '/libSDL3.a', dstdir)
	install(srcdir + '/libyaml-cpp.a', dstdir)

if default_library == 'shared':
	print('Change liborge.dylib ID to @rpath/liborge.dylib')
	subprocess.run(['install_name_tool', '-id', '@rpath/liborge.dylib', dstdir + '/liborge.dylib'])
