@echo off
rem Toolchain probe for Windows: needs VS 2022 Build Tools and CUDA 12.4 nvcc.
rem Optional %1 = prefix of a conda env from channel nvidia/label/cuda-12.4.1 with
rem cuda-nvcc cuda-cudart-dev cuda-cudart-static cuda-cccl.
setlocal
if not "%~1"=="" set PATH=%~1\bin;%~1\Library\bin;%PATH%
if not "%~1"=="" set EXTRA=-I"%~1\include" -L"%~1\lib"
call "C:\Program Files (x86)\Microsoft Visual Studio\2022\BuildTools\VC\Auxiliary\Build\vcvars64.bat" >nul
cd /d "%~dp0"
nvcc --version | findstr release
set G=-gencode arch=compute_61,code=sm_61 -gencode arch=compute_75,code=sm_75 -gencode arch=compute_80,code=sm_80 -gencode arch=compute_86,code=sm_86 -gencode arch=compute_89,code=sm_89 -gencode arch=compute_90,code=sm_90 -gencode arch=compute_90,code=compute_90
nvcc -O3 -std=c++17 %G% -cudart static -Xcompiler /openmp cu_probe.cu -o cu_probe.exe %EXTRA%
if exist cu_probe.exe cu_probe.exe
endlocal
