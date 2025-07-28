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

install(srcdir + '/libvulkan.so.1.*', dstdir + '/libvulkan.so')

if default_library == 'static':
	install(srcdir + '/libSDL3.a', dstdir)
	install(srcdir + '/libyaml-cpp.a', dstdir)

# TODO: change ID.
