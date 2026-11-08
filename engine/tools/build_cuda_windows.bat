@echo off
rem Release build of the engine with the CUDA back end on Windows (VS 2022 Build Tools + CUDA 12.x nvcc).
rem   build_cuda_windows.bat SOURCE_ENGINE_DIR BUILD_DIR [CUDA_PREFIX]
rem CUDA_PREFIX: a CUDA 12.x tool kit folder with bin\nvcc.exe (e.g. a conda env of channel
rem nvidia/label/cuda-12.4.1 with cuda-nvcc cuda-cudart-dev cuda-cudart-static cuda-cccl, its Library folder).
setlocal
call "C:\Program Files (x86)\Microsoft Visual Studio\2022\BuildTools\VC\Auxiliary\Build\vcvars64.bat" >nul
set CMAKE="C:\Program Files (x86)\Microsoft Visual Studio\2022\BuildTools\Common7\IDE\CommonExtensions\Microsoft\CMake\CMake\bin\cmake.exe"
set EXTRA=
if not "%~3"=="" set EXTRA=-DCMAKE_CUDA_COMPILER="%~3\bin\nvcc.exe" -DCUDAToolkit_ROOT="%~3"
if not "%~3"=="" set PATH=%~3\bin;%PATH%
%CMAKE% -S "%~1" -B "%~2" -G "NMake Makefiles" -DCMAKE_BUILD_TYPE=Release -DAVAS_WITH_CUDA=ON %EXTRA%
if errorlevel 1 exit /b 1
%CMAKE% --build "%~2"
endlocal
