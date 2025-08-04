import os
import shutil
import sys

def install(src, dst):
	if not os.path.isfile(src):
		print('error: ' + src + ' not found.')
		exit(1)
	print('Installing ' + src + ' to ' + dst)
	os.makedirs(os.path.dirname(dst), exist_ok=True)
	shutil.copy(src, dst)

srclibdir = sys.argv[1]
srcbindir = sys.argv[2]
dstlibdir = sys.argv[3]
dstbindir = sys.argv[4]
default_library = sys.argv[5]

install(srclibdir + '/vulkan-1.lib', dstlibdir + '/vulkan-1.lib')
install(srcbindir + '/vulkan-1.dll', dstbindir + '/vulkan-1.dll')

if default_library == 'static':
	install(srclibdir + '/SDL3.lib', dstlibdir + '/SDL3.lib')
	install(srclibdir + '/yaml-cpp.lib', dstlibdir + '/yaml-cpp.lib')
