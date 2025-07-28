import shutil
import subprocess
import sys

def install(src, dst):
	print('Installing ' + n + ' to ' + dst)
	shutil.copy(n, dst)

srcdir = sys.argv[1]
dstdir = sys.argv[2]
default_library = sys.argv[3]
vulkan_version = sys.argv[4]

vulkan_lib = 'libvulkan.so.' + vulkan_version

install(srcdir + '/' + vulkan_lib, dstdir + '/libvulkan.so')

if default_library == 'static':
	install(srcdir + '/libSDL3.a', dstdir)
	install(srcdir + '/libyaml-cpp.a', dstdir)

print('Change libvulkan.so ID to libvulkan.so')
subprocess.run(['patchelf', '--set-soname', 'libvulkan.so', dstdir + '/libvulkan.so'])

if default_library == 'shared':
	print('Change liborge.so dependence from ' + vulkan_lib + ' to libvulkan.so')
	subprocess.run(['patchelf', '--replace-needed', vulkan_lib, 'libvulkan.so', dstdir + '/liborge.so'])
