cmake_minimum_required(VERSION 3.18)

PROJECT(basix_pybind11)

find_package(Eigen3 REQUIRED)
message(STATUS "Found Eigen ${EIGEN3_INCLUDE_DIRS}")

find_package(Basix REQUIRED)
include(FetchContent)
FetchContent_Declare(
  basixcpp
  GIT_REPOSITORY https://github.com/FEniCS/basix.git
  GIT_TAG        cmake-improve
)
FetchContent_MakeAvailable(basixcpp)

# Set C++ standard before finding pybind11
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

find_package(pybind11 REQUIRED CONFIG HINTS ${PYBIND11_DIR} ${PYBIND11_ROOT}
  $ENV{PYBIND11_DIR} $ENV{PYBIND11_ROOT})

# Create the binding library
pybind11_add_module(_basixcpp SHARED wrapper.cpp)

target_include_directories(_basixcpp PRIVATE ${EIGEN3_INCLUDE_DIRS})

# Add srict compiler flags
include(CheckCXXCompilerFlag)
CHECK_CXX_COMPILER_FLAG("-Wall -Werror -Wno-comment -pedantic" HAVE_PEDANTIC)
if (HAVE_PEDANTIC)
  target_compile_options(_basixcpp PRIVATE -Wall;-Werror;-Wno-comment;-pedantic)
endif()

# In Debug mode override pybind11 symbols visibility
# Symbols must be visible to backtrace_symbols() to produce nice logs
if (CMAKE_BUILD_TYPE STREQUAL "Debug")
  target_compile_options(_basixcpp PRIVATE "-fvisibility=default")
endif()

# Add libraries and other config
target_link_libraries(_basixcpp PRIVATE pybind11::module)
target_link_libraries(_basixcpp PUBLIC Basix::basix)
