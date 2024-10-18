# ===========================================================================
# Copyright (C) 2021-2023 Infineon Technologies AG
# All rights reserved.
# ===========================================================================
#
# ===========================================================================
# This document contains proprietary information of Infineon Technologies AG.
# Passing on and copying of this document, and communication of its contents
# is not permitted without Infineon's prior written authorization.
# ===========================================================================

cmake_minimum_required(VERSION 3.10)

project(lib_avian_example)

###############################################################################
# Include External Modules
###############################################################################
# For sure libAvian is needed, because this example shows how to use libAvian.
# To connect to a Radar Baseboard MCU7 ("Hatvan") Strata is needed, too.
# Strata is added first, because otherwise CMake run for libAvian won't see the
# Strata target and excludes StrataPort from build.
set(STRATA_BUILD_WRAPPERS OFF CACHE BOOL "" FORCE)
set(STRATA_PACKAGES_AUTO_DETECT OFF CACHE BOOL "" FORCE)
set(STRATA_BUILD_WARNINGS_AS_ERRORS OFF CACHE BOOL "" FORCE)
add_subdirectory(${STRATA_DIR} strata EXCLUDE_FROM_ALL)

add_subdirectory(.. lib_avian EXCLUDE_FROM_ALL)

###############################################################################
# Source Files
###############################################################################
# There's only one source file in this example.
file(GLOB source_files
     main.cpp)

###############################################################################
# Target Executable
###############################################################################
# From the one source file, a binary is build and linked against libAvian and
# Strata.
add_executable(lib_avian_example ${source_files})
target_link_libraries(lib_avian_example
                      lib_avian_static
                      strata_static)
