import shutil
import sys

def install(src, dst):
	print('Installing ' + src + ' to ' + dst)
	shutil.copy(src, dst)

srclibdir = sys.argv[1]
srcbindir = sys.argv[2]
dstlibdir = sys.argv[3]
dstbindir = sys.argv[4]
default_library = sys.argv[5]

install(srclibdir + '/vulkan-1.lib', dstlibdir)
install(srcbindir + '/vulkan-1.dll', dstbindir)

if default_library == 'static':
	install(srclibdir + '/SDL3.lib', dstlibdir)
	install(srclibdir + '/yaml-cpp.lib', dstlibdir)
