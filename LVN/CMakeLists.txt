cmake_minimum_required(VERSION 3.20)
project(LVNPass)

# Set LLVM installation directory
set(LT_LLVM_INSTALL_DIR "/home/152402004/llvm-install" CACHE PATH "Path to custom LLVM installation")

# Add LLVM to CMake's search path
list(APPEND CMAKE_PREFIX_PATH "${LT_LLVM_INSTALL_DIR}/lib/cmake/llvm/")

# Find LLVM package
find_package(LLVM REQUIRED CONFIG)

# Ensure LLVM version is compatible
if("${LLVM_VERSION_MAJOR}" VERSION_LESS 15)
  message(FATAL_ERROR "Found LLVM ${LLVM_VERSION_MAJOR}, but need LLVM 15 or above")
endif()

# Include LLVM directories
include_directories(SYSTEM ${LLVM_INCLUDE_DIRS})
add_definitions(${LLVM_DEFINITIONS})

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Disable RTTI if LLVM does not enable it
if(NOT LLVM_ENABLE_RTTI)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fno-rtti")
endif()

# Define pass source file
add_library(LVNPass SHARED LVNPass.cpp)

# Link against LLVM libraries
target_link_libraries(LVNPass PRIVATE LLVMCore LLVMPasses LLVMAnalysis LLVMIRReader LLVMTransformUtils)

# Register the pass
target_compile_definitions(LVNPass PRIVATE LLVM_PLUGIN_API_VERSION=${LLVM_PLUGIN_API_VERSION})

