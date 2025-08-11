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
vulkan_version = sys.argv[4]

vulkan_lib = 'libvulkan.so.' + vulkan_version

install(srcdir + '/' + vulkan_lib, dstdir + '/libvulkan.so')

if default_library == 'static':
	install(srcdir + '/libSDL3.a', dstdir)
	install(srcdir + '/libyaml-cpp.a', dstdir)
	install(srcdir + '/libssl.a', dstdir)

print('Change libvulkan.so ID to libvulkan.so')
subprocess.run(['patchelf', '--set-soname', 'libvulkan.so', dstdir + '/libvulkan.so'])

if default_library == 'shared':
	print('Change liborge.so dependence from ' + vulkan_lib + ' to libvulkan.so')
	subprocess.run(['patchelf', '--replace-needed', vulkan_lib, 'libvulkan.so', dstdir + '/liborge.so'])
