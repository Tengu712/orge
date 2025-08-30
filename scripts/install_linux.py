import os
import shutil
import subprocess
import sys

def install(src, dst):
	if not os.path.isfile(src):
		print('error: ' + src + ' not found.')
		exit(1)
	print('Installing ' + src + ' to ' + dst)
	os.makedirs(os.path.dirname(dst), exist_ok=True)
	shutil.copy(src, dst)

srcdir = sys.argv[1]
dstdir = sys.argv[2]
default_library = sys.argv[3]

if default_library == 'static':
	install(srcdir + '/libSDL3.a', dstdir)
	install(srcdir + '/libyaml-cpp.a', dstdir)
