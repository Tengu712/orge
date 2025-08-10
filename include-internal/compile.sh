#!/bin/bash -e

glslang -V text-shader.vert -o text-shader.vert.spv
glslang -V text-shader.frag -o text-shader.frag.spv

xxd -i -n text_shader_vert_spv text-shader.vert.spv > text-shader-vert-spv.cpp
xxd -i -n text_shader_frag_spv text-shader.frag.spv > text-shader-frag-spv.cpp
