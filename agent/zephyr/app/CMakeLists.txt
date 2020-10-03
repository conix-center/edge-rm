# Copyright (C) 2019 Intel Corporation.  All rights reserved.
# SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

cmake_minimum_required(VERSION 3.8.2)

#set(CMAKE_C_COMPILER "/home/dafran/gcc-arm-none-eabi-9-2020-q2-update/bin/arm-none-eabi-gcc")
#set(CMAKE_CXX_COMPILER "/home/dafran/gcc-arm-none-eabi-9-2020-q2-update/bin/arm-none-eabi-g++")
#find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})
include($ENV{ZEPHYR_BASE}/cmake/app/boilerplate.cmake NO_POLICY_SCOPE)
project(NONE)

#include the agent libraries and source files
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../../c_lib)
FILE(GLOB agent_sources ../../c_lib/*.c)

#include nanopb
set(NANOPB_SRC_ROOT_FOLDER ${CMAKE_CURRENT_SOURCE_DIR}/../../support/nanopb)
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${NANOPB_SRC_ROOT_FOLDER}/extra)
find_package( Nanopb REQUIRED )
include_directories(${NANOPB_INCLUDE_DIRS})

NANOPB_GENERATE_CPP(PROTO_SRCS PROTO_HDRS RELPATH ../../../proto ../../../proto/messages.proto)

include_directories(${CMAKE_CURRENT_BINARY_DIR})

enable_language (ASM)

set (WAMR_BUILD_PLATFORM "zephyr")

# Build as X86_32 by default, change to "AARCH64[sub]", "ARM[sub]", "THUMB[sub]", "MIPS" or "XTENSA"
# if we want to support arm, thumb, mips or xtensa
if (NOT DEFINED WAMR_BUILD_TARGET)
  set (WAMR_BUILD_TARGET "X86_32")
endif ()

if (NOT DEFINED WAMR_BUILD_INTERP)
  # Enable Interpreter by default
  set (WAMR_BUILD_INTERP 1)
endif ()

if (NOT DEFINED WAMR_BUILD_AOT)
  # Enable AOT by default.
  set (WAMR_BUILD_AOT 1)
endif ()

if (NOT DEFINED WAMR_BUILD_LIBC_BUILTIN)
  # Enable libc builtin support by default
  set (WAMR_BUILD_LIBC_BUILTIN 1)
endif ()

if (NOT DEFINED WAMR_BUILD_LIBC_WASI)
  # Disable libc wasi support by default
  set (WAMR_BUILD_LIBC_WASI 0)
endif ()

set (WAMR_ROOT_DIR ${CMAKE_CURRENT_SOURCE_DIR}/wamr)

include (${WAMR_ROOT_DIR}/build-scripts/runtime_lib.cmake)

#add_executable (app src/main.c src/native_impl.c ${UNCOMMON_SHARED_SOURCE})
FILE(GLOB app_sources src/*.c)
target_sources(app PRIVATE
               ${WAMR_RUNTIME_LIB_SOURCE}
               ${app_sources}
               ${agent_sources}
               ${PROTO_SRCS}
               ${PROTO_HDRS})
target_include_directories(app PRIVATE ${ZEPHYR_BASE}/subsys/net/ip)
