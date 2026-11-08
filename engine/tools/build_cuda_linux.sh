#!/bin/bash
# Release build of the engine with the CUDA back end for Linux (glibc 2.17 and newer).
# Run inside the manylinux2014 + CUDA 12.4 image (docker/manylinux2014-cuda12.4.Dockerfile):
#   docker run --rm -v SRC:/src -v OUT:/out IMAGE bash /src/engine/tools/build_cuda_linux.sh [extra cmake args]
# Extra arguments go to cmake (e.g. -DAVAS_WITH_MPI=ON -DMPI_HOME=/opt/mpich).
set -e
SRC=${SRC:-/src/engine}
OUT=${OUT:-/out}
gcc --version | head -1
nvcc --version | tail -1
cmake --version | head -1
cmake -S "$SRC" -B "$OUT" -DCMAKE_BUILD_TYPE=Release -DAVAS_WITH_CUDA=ON \
      -DCMAKE_CUDA_COMPILER=/usr/local/cuda-12.4/bin/nvcc "$@"
cmake --build "$OUT" -j "${JOBS:-16}"
echo "--- libavas_engine.so needs:"
objdump -T "$OUT/libavas_engine.so" | grep -o 'GLIBC_[0-9.]*' | sort -uV | tail -1
objdump -T "$OUT/libavas_engine.so" | grep -o 'GLIBCXX_[0-9.]*' | sort -uV | tail -1
ldd "$OUT/libavas_engine.so" || true
