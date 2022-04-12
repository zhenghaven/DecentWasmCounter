# Copyright (c) 2022 Haofan Zheng
# Use of this source code is governed by an MIT-style
# license that can be found in the LICENSE file or at
# https://opensource.org/licenses/MIT.

cmake_minimum_required(VERSION 3.18)

################################################################################
# Fetching dependencise
################################################################################

include(FetchContent)

FetchContent_Declare(
	git_googletest
	GIT_REPOSITORY https://github.com/google/googletest.git
	GIT_TAG        release-1.11.0
)
FetchContent_MakeAvailable(git_googletest)
set_property(TARGET gtest PROPERTY
	MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")

################################################################################
# Add targets
################################################################################

file(GLOB_RECURSE SOURCES ${CMAKE_CURRENT_LIST_DIR}/*.[ch]*)

add_executable(DecentWasmCounter_test ${SOURCES})

target_compile_options(DecentWasmCounter_test
	PRIVATE "$<$<CONFIG:Debug>:${DEBUG_OPTIONS}>"
			"$<$<CONFIG:Release>:${RELEASE_OPTIONS}>")
target_link_libraries(DecentWasmCounter_test DecentWasmCounter_untrusted gtest)

set_property(TARGET DecentWasmCounter_test PROPERTY
	MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
set_property(TARGET DecentWasmCounter_test PROPERTY CXX_STANDARD 17)

add_test(NAME DecentWasmCounter_test
	COMMAND DecentWasmCounter_test)
