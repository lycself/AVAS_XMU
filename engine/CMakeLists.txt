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
  src/core/errorset.cpp
  src/core/tools.cpp
  src/core/synchronous.cpp
  src/core/tracker.cpp
  src/core/engine.cpp
  src/core/dllmath.cpp
)

add_library(avas_engine SHARED ${AVAS_CORE_SOURCES})
target_include_directories(avas_engine PUBLIC include PRIVATE src ${CMAKE_CURRENT_SOURCE_DIR})
target_compile_definitions(avas_engine PRIVATE AVAS_ENGINE_BUILD)
target_compile_options(avas_engine PRIVATE ${AVAS_FP_FLAGS})
set_target_properties(avas_engine PROPERTIES CXX_VISIBILITY_PRESET hidden VISIBILITY_INLINES_HIDDEN ON)
if(MSVC)
  target_compile_options(avas_engine PRIVATE /W3 /utf-8 /EHsc)
  target_compile_definitions(avas_engine PRIVATE _CRT_SECURE_NO_WARNINGS)
  # The original DLL was built with /fp:fast; MSVC then evaluates sin and cos of the same
  # argument with its own static SSE2 sincos routine.  dllmath.cpp is compiled the same way so
  # that "compat=dll" calls exactly that routine (see dllmath.h).
  set_source_files_properties(src/core/dllmath.cpp PROPERTIES COMPILE_OPTIONS "/fp:fast")
  set_target_properties(avas_engine PROPERTIES OUTPUT_NAME avas_engine PREFIX "")
else()
  target_compile_options(avas_engine PRIVATE -Wall -Wextra -Wno-unused-parameter)
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
target_compile_options(avas_engine_static_for_tests PRIVATE ${AVAS_FP_FLAGS})
target_link_libraries(avas_engine_static_for_tests PUBLIC Threads::Threads)
if(MSVC)
  target_compile_options(avas_engine_static_for_tests PRIVATE /utf-8 /EHsc)
  target_compile_definitions(avas_engine_static_for_tests PRIVATE _CRT_SECURE_NO_WARNINGS)
endif()
add_test(NAME unit COMMAND avas_engine_unit)

# Physics checks of the space-charge solvers (engine/tests/physics/run_physics.py runs them).
add_executable(avas_engine_physics tests/physics/sc_ellipsoid.cpp)
target_include_directories(avas_engine_physics PRIVATE include src ${CMAKE_CURRENT_SOURCE_DIR})
target_compile_options(avas_engine_physics PRIVATE ${AVAS_FP_FLAGS})
target_link_libraries(avas_engine_physics PRIVATE avas_engine_static_for_tests Threads::Threads)
