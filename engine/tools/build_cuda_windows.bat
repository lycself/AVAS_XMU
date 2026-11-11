@echo off
rem Release build of the engine with the CUDA back end on Windows (Visual Studio 2022 + CUDA 12.x nvcc).
rem   build_cuda_windows.bat SOURCE_ENGINE_DIR BUILD_DIR [CUDA_PREFIX]
rem CUDA_PREFIX: a CUDA 12.x tool kit folder with bin\nvcc.exe -- an installed tool kit, NVIDIA's redistributable
rem archives cuda_nvcc + cuda_cudart + cuda_cccl unpacked into one folder (what .github/workflows/engine.yml does),
rem or the Library folder of a conda env of channel nvidia/label/cuda-12.4.1 with cuda-nvcc cuda-cudart-dev
rem cuda-cudart-static cuda-cccl.  Without it, nvcc is taken from PATH / CUDA_PATH.
rem Visual Studio 2022 (any edition, or the Build Tools) is found with vswhere; CUDA 12.4 does not accept the
rem MSVC of newer Visual Studio versions.  The generator is Ninja when vcvars puts it on PATH (Visual Studio
rem ships it), otherwise NMake (one file at a time).  More cmake arguments: set AVAS_CMAKE_ARGS=... first.
setlocal
set "VSWHERE=%ProgramFiles(x86)%\Microsoft Visual Studio\Installer\vswhere.exe"
set "VSDIR="
if exist "%VSWHERE%" for /f "usebackq delims=" %%i in (`"%VSWHERE%" -latest -products * -version [17.0^,18.0^) -requires Microsoft.VisualStudio.Component.VC.Tools.x86.x64 -property installationPath`) do set "VSDIR=%%i"
if not defined VSDIR set "VSDIR=%ProgramFiles(x86)%\Microsoft Visual Studio\2022\BuildTools"
if not exist "%VSDIR%\VC\Auxiliary\Build\vcvars64.bat" (
  echo Visual Studio 2022 with the C++ tools was not found
  exit /b 1
)
call "%VSDIR%\VC\Auxiliary\Build\vcvars64.bat" >nul
if errorlevel 1 exit /b 1
set "GEN=NMake Makefiles"
where ninja >nul 2>nul && set "GEN=Ninja"
set EXTRA=
if not "%~3"=="" set EXTRA=-DCMAKE_CUDA_COMPILER="%~3\bin\nvcc.exe" -DCUDAToolkit_ROOT="%~3"
if not "%~3"=="" set "PATH=%~3\bin;%PATH%"
echo Visual Studio: %VSDIR%
echo generator: %GEN%
cmake -S "%~1" -B "%~2" -G "%GEN%" -DCMAKE_BUILD_TYPE=Release -DAVAS_WITH_CUDA=ON %EXTRA% %AVAS_CMAKE_ARGS%
if errorlevel 1 exit /b 1
cmake --build "%~2"
if errorlevel 1 exit /b 1
endlocal
