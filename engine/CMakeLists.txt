cmake_minimum_required(VERSION 3.16)
project(avas_engine LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
  set(CMAKE_BUILD_TYPE Release)
endif()

# Floating point: the operation order in src/core follows the original kernel (AVAS.dll) term by
# term.  The compiler must not contract to FMA or re-associate, otherwise "compat=dll" results
# drift away from the reference in the last bits.
if(MSVC)
  set(AVAS_FP_FLAGS /fp:precise)
else()
  set(AVAS_FP_FLAGS -ffp-contract=off -fno-fast-math)
endif()

# Some macOS Command Line Tools installs leave a stale, almost empty usr/include/c++/v1 next to the
# compiler; clang then never looks at the SDK's libc++ headers and <string> is "not found".  Fall
# back to the SDK copy when that happens (reinstalling the Command Line Tools fixes it properly).
if(APPLE AND CMAKE_OSX_SYSROOT AND NOT MSVC)
  include(CheckCXXSourceCompiles)
  check_cxx_source_compiles("#include <string>\nint main(){std::string s;return (int)s.size();}" AVAS_LIBCXX_OK)
  if(NOT AVAS_LIBCXX_OK AND EXISTS "${CMAKE_OSX_SYSROOT}/usr/include/c++/v1/string")
    message(STATUS "avas_engine: using libc++ headers from ${CMAKE_OSX_SYSROOT}")
    add_compile_options(-nostdinc++ -isystem "${CMAKE_OSX_SYSROOT}/usr/include/c++/v1")
  endif()
endif()

option(AVAS_WITH_CUDA "Build the CUDA back end (NVIDIA GPUs)" OFF)
option(AVAS_WITH_MPI "Distribute the particles over MPI ranks (CUDA back end)" OFF)

set(AVAS_CORE_SOURCES
  src/capi.cpp
  src/core/options.cpp
  src/core/text.cpp
  src/core/format.cpp
  src/core/files.cpp
  src/core/params.cpp
  src/core/fieldmap.cpp
  src/core/lattice.cpp
  src/core/beam.cpp
  src/core/distribution.cpp
  src/core/spacecharge.cpp
  src/core/latview.cpp
  src/core/device.cpp
  src/core/device_emu.cpp
  src/core/errorset.cpp
  src/core/tools.cpp
  src/core/synchronous.cpp
  src/core/tracker.cpp
  src/core/engine.cpp
  src/core/dllmath.cpp
)

if(AVAS_WITH_CUDA)
  if(CMAKE_VERSION VERSION_LESS 3.18)
    message(FATAL_ERROR "AVAS_WITH_CUDA needs CMake 3.18 or newer")
  endif()
  # One CUDA 12.x tool kit builds sm_60 .. sm_90 (CUDA 13 dropped Pascal); the CUDA runtime is
  # linked statically.  --fmad=false: no FMA contraction, so that the fp64 particles are bit for bit
  # those of the CPU engine (corrected mode).
  if(NOT DEFINED CMAKE_CUDA_ARCHITECTURES)
    set(CMAKE_CUDA_ARCHITECTURES "60-real;61-real;70-real;75-real;80-real;86-real;89-real;90")
  endif()
  set(CMAKE_CUDA_RUNTIME_LIBRARY Static)
  enable_language(CUDA)
  set(CMAKE_CUDA_STANDARD 17)
  set(CMAKE_CUDA_STANDARD_REQUIRED ON)
  find_package(CUDAToolkit REQUIRED)
  list(APPEND AVAS_CORE_SOURCES src/cuda/cuda_backend.cu)
  # AVAS_CUDA_FMAD=ON only to measure what the bit-for-bit agreement costs (not for releases)
  option(AVAS_CUDA_FMAD "Measurement only: let nvcc contract to FMA (GPU fp64 no longer equals the CPU)" OFF)
  if(AVAS_CUDA_FMAD)
    set(AVAS_FMAD_FLAG --fmad=true)
  else()
    set(AVAS_FMAD_FLAG --fmad=false)
  endif()
  if(MSVC)
    set(AVAS_CUDA_FLAGS ${AVAS_FMAD_FLAG} -Xcompiler=/fp:precise,/utf-8,/EHsc)
  else()
    set(AVAS_CUDA_FLAGS ${AVAS_FMAD_FLAG} -Xcompiler=-ffp-contract=off,-fno-fast-math)
  endif()
  list(APPEND AVAS_CUDA_FLAGS --expt-relaxed-constexpr -Xcudafe=--diag_suppress=20012)
else()
  list(APPEND AVAS_CORE_SOURCES src/core/device_stub.cpp)
endif()
# Metal back end (macOS GPUs, M6c): the MSL source is the shared headers plus src/metal/kernels.metal,
# embedded into the library and compiled at run time (no offline Metal compiler is needed).
option(AVAS_WITH_METAL "Build the Metal back end (macOS GPUs)" OFF)
if(AVAS_WITH_METAL)
  if(NOT APPLE)
    message(FATAL_ERROR "AVAS_WITH_METAL needs macOS")
  endif()
  # The Objective-C++ part must be compiled by Apple's clang, and the C++ part by the same compiler
  # (one libc++, and the libc++ header fallback above then covers both).  Other clang builds, e.g.
  # conda's (CC / CXX set by an activated environment), cannot parse the SDK's Foundation headers.
  if(NOT CMAKE_CXX_COMPILER_ID STREQUAL "AppleClang")
    message(FATAL_ERROR "AVAS_WITH_METAL needs Apple's clang (found ${CMAKE_CXX_COMPILER_ID} ${CMAKE_CXX_COMPILER}); "
      "configure a new build directory with CC=/usr/bin/clang CXX=/usr/bin/clang++ (or with CC and CXX unset)")
  endif()
  if(NOT DEFINED CMAKE_OBJCXX_COMPILER)
    set(CMAKE_OBJCXX_COMPILER "${CMAKE_CXX_COMPILER}")
  endif()
  enable_language(OBJCXX)
  set(AVAS_MSL_PARTS src/core/hd.h src/core/mesh_kernels.h src/core/sc_kernels.h src/metal/kernels.metal)
  set(AVAS_MSL "#define double float\n")
  foreach(part ${AVAS_MSL_PARTS})
    file(READ ${CMAKE_CURRENT_SOURCE_DIR}/${part} txt)
    string(REGEX REPLACE "#include \"core/[a-z_]+\.h\"" "" txt "${txt}")
    string(REPLACE "#pragma once" "" txt "${txt}")
    if(part STREQUAL "src/core/hd.h")
      string(APPEND txt "\n#pragma METAL fp contract(off)\n")
    endif()
    string(APPEND AVAS_MSL "// ---- ${part} ----\n${txt}\n")
    set_property(DIRECTORY APPEND PROPERTY CMAKE_CONFIGURE_DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/${part})
  endforeach()
  string(REPLACE "\\n" "\n" AVAS_MSL "${AVAS_MSL}")
  file(WRITE ${CMAKE_CURRENT_BINARY_DIR}/generated/avas_msl_source.inc
       "// generated from ${AVAS_MSL_PARTS}\nstatic const char* const kAvasMslSource = R\"AVASMSL(${AVAS_MSL})AVASMSL\";\n")
  list(APPEND AVAS_CORE_SOURCES src/metal/metal_backend.mm)
else()
  list(APPEND AVAS_CORE_SOURCES src/core/metal_stub.cpp)
endif()
if(AVAS_WITH_MPI)
  if(NOT AVAS_WITH_CUDA)
    message(FATAL_ERROR "AVAS_WITH_MPI needs AVAS_WITH_CUDA")
  endif()
  # only the MPI C API is used; AVAS_MPI_INCLUDE + AVAS_MPI_LIBRARY bypass FindMPI (e.g. Intel MPI with
  # another host compiler)
  add_library(avas_mpi INTERFACE)
  if(AVAS_MPI_INCLUDE AND AVAS_MPI_LIBRARY)
    target_include_directories(avas_mpi INTERFACE ${AVAS_MPI_INCLUDE})
    target_link_libraries(avas_mpi INTERFACE ${AVAS_MPI_LIBRARY})
  else()
    find_package(MPI REQUIRED COMPONENTS C)
    target_link_libraries(avas_mpi INTERFACE MPI::MPI_C)
  endif()
endif()

add_library(avas_engine SHARED ${AVAS_CORE_SOURCES})
target_include_directories(avas_engine PUBLIC include PRIVATE src ${CMAKE_CURRENT_SOURCE_DIR})
target_compile_definitions(avas_engine PRIVATE AVAS_ENGINE_BUILD)
target_compile_options(avas_engine PRIVATE "$<$<COMPILE_LANGUAGE:CXX>:${AVAS_FP_FLAGS}>")
if(AVAS_WITH_CUDA)
  target_compile_options(avas_engine PRIVATE "$<$<COMPILE_LANGUAGE:CUDA>:${AVAS_CUDA_FLAGS}>")
  target_compile_definitions(avas_engine PRIVATE AVAS_WITH_CUDA=1)
  target_link_libraries(avas_engine PRIVATE CUDA::cudart_static)
endif()
if(AVAS_WITH_METAL)
  target_include_directories(avas_engine PRIVATE ${CMAKE_CURRENT_BINARY_DIR}/generated)
  target_compile_options(avas_engine PRIVATE "$<$<COMPILE_LANGUAGE:OBJCXX>:-fobjc-arc;${AVAS_FP_FLAGS}>")
  target_link_libraries(avas_engine PRIVATE "-framework Metal" "-framework Foundation")
endif()
if(AVAS_WITH_MPI)
  target_compile_definitions(avas_engine PRIVATE AVAS_WITH_MPI=1)
  target_link_libraries(avas_engine PRIVATE avas_mpi)
endif()
set_target_properties(avas_engine PROPERTIES CXX_VISIBILITY_PRESET hidden CUDA_VISIBILITY_PRESET hidden
  VISIBILITY_INLINES_HIDDEN ON)
if(CMAKE_SYSTEM_NAME STREQUAL "Linux")
  # Symbols of statically linked archives (libstdc++ with -static-libstdc++, cudart_static) must not
  # be exported: a Python process that already loaded another libstdc++ would otherwise bind to ours
  # and crash in std::ostream (seen on the CentOS 7 cluster, M6-notes section 6).
  # The version script hides the weak std:: template instances too, which -fvisibility cannot.
  target_link_options(avas_engine PRIVATE "LINKER:--exclude-libs,ALL"
    "LINKER:--version-script=${CMAKE_CURRENT_SOURCE_DIR}/src/exports.map")
  set_property(TARGET avas_engine APPEND PROPERTY LINK_DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/src/exports.map)
endif()
if(MSVC)
  target_compile_options(avas_engine PRIVATE "$<$<COMPILE_LANGUAGE:CXX>:/W3;/utf-8;/EHsc>")
  target_compile_definitions(avas_engine PRIVATE _CRT_SECURE_NO_WARNINGS)
  # The original DLL was built with /fp:fast; MSVC then evaluates sin and cos of the same
  # argument with its own static SSE2 sincos routine.  dllmath.cpp is compiled the same way so
  # that "compat=dll" calls exactly that routine (see dllmath.h).
  set_source_files_properties(src/core/dllmath.cpp PROPERTIES COMPILE_OPTIONS "/fp:fast")
  set_target_properties(avas_engine PROPERTIES OUTPUT_NAME avas_engine PREFIX "")
else()
  target_compile_options(avas_engine PRIVATE "$<$<COMPILE_LANGUAGE:CXX>:-Wall;-Wextra;-Wno-unused-parameter>")
endif()
find_package(Threads REQUIRED)
target_link_libraries(avas_engine PRIVATE Threads::Threads)

# Small command line driver (path + main_agent) used by the tests and for debugging.
add_executable(avas_engine_run tests/run_main.cpp)
target_link_libraries(avas_engine_run PRIVATE avas_engine)
target_include_directories(avas_engine_run PRIVATE include)

enable_testing()
add_executable(avas_engine_unit tests/unit_tests.cpp)
target_include_directories(avas_engine_unit PRIVATE include src ${CMAKE_CURRENT_SOURCE_DIR})
target_compile_options(avas_engine_unit PRIVATE ${AVAS_FP_FLAGS})
target_link_libraries(avas_engine_unit PRIVATE avas_engine_static_for_tests)
add_library(avas_engine_static_for_tests STATIC ${AVAS_CORE_SOURCES})
target_include_directories(avas_engine_static_for_tests PUBLIC include PRIVATE src ${CMAKE_CURRENT_SOURCE_DIR})
target_compile_definitions(avas_engine_static_for_tests PUBLIC AVAS_ENGINE_STATIC)
target_compile_options(avas_engine_static_for_tests PRIVATE "$<$<COMPILE_LANGUAGE:CXX>:${AVAS_FP_FLAGS}>")
target_link_libraries(avas_engine_static_for_tests PUBLIC Threads::Threads)
if(AVAS_WITH_CUDA)
  target_compile_options(avas_engine_static_for_tests PRIVATE "$<$<COMPILE_LANGUAGE:CUDA>:${AVAS_CUDA_FLAGS}>")
  target_compile_definitions(avas_engine_static_for_tests PRIVATE AVAS_WITH_CUDA=1)
  target_link_libraries(avas_engine_static_for_tests PUBLIC CUDA::cudart_static)
endif()
if(AVAS_WITH_METAL)
  target_include_directories(avas_engine_static_for_tests PRIVATE ${CMAKE_CURRENT_BINARY_DIR}/generated)
  target_compile_options(avas_engine_static_for_tests PRIVATE "$<$<COMPILE_LANGUAGE:OBJCXX>:-fobjc-arc;${AVAS_FP_FLAGS}>")
  target_link_libraries(avas_engine_static_for_tests PUBLIC "-framework Metal" "-framework Foundation")
endif()
if(AVAS_WITH_MPI)
  target_compile_definitions(avas_engine_static_for_tests PRIVATE AVAS_WITH_MPI=1)
  target_link_libraries(avas_engine_static_for_tests PUBLIC avas_mpi)
endif()
if(MSVC)
  target_compile_options(avas_engine_static_for_tests PRIVATE "$<$<COMPILE_LANGUAGE:CXX>:/utf-8;/EHsc>")
  target_compile_definitions(avas_engine_static_for_tests PRIVATE _CRT_SECURE_NO_WARNINGS)
endif()
add_test(NAME unit COMMAND avas_engine_unit)

# Physics checks of the space-charge solvers (engine/tests/physics/run_physics.py runs them).
add_executable(avas_engine_physics tests/physics/sc_ellipsoid.cpp)
target_include_directories(avas_engine_physics PRIVATE include src ${CMAKE_CURRENT_SOURCE_DIR})
target_compile_options(avas_engine_physics PRIVATE ${AVAS_FP_FLAGS})
target_link_libraries(avas_engine_physics PRIVATE avas_engine_static_for_tests Threads::Threads)
