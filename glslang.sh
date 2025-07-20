#!/bin/bash

cd examples/multipass

glslang -V mesh.vert -o mesh.vert.spv
glslang -V mesh.frag -o mesh.frag.spv

glslang -V pattern.frag -o pattern.frag.spv

glslang -V integration.frag -o integration.frag.spv

glslang -V screen.vert -o screen.vert.spv
