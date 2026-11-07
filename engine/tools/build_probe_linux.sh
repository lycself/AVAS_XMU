#!/bin/bash
set -e
cd "$(dirname "$0")"
gcc --version | head -1; nvcc --version | tail -2; ldd --version | head -1
GENCODE=""
for a in 61 75 80 86 89 90; do GENCODE="$GENCODE -gencode arch=compute_$a,code=sm_$a"; done
GENCODE="$GENCODE -gencode arch=compute_90,code=compute_90"
nvcc -O3 -std=c++17 $GENCODE -cudart static -Xcompiler -static-libgcc,-static-libstdc++ \
     -Xcompiler -fopenmp -lgomp cu_probe.cu -o cu_probe
echo "--- built"; ls -la cu_probe; ldd cu_probe
echo "--- max GLIBC symbol needed:"; objdump -T cu_probe | grep -o 'GLIBC_[0-9.]*' | sort -uV | tail -1
