#!/bin/bash
echo "yes" | ./build_visit3_5_0 \
--makeflags -j4 \
--skip-opengl-context-check \
--no-visit \
--parallel \
--mpich \
--optional \
--llvm \
--mesagl \
--anari \
--ospray \
--no-moab
