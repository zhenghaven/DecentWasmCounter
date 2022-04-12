# Copyright (c) 2022 Haofan Zheng
# Use of this source code is governed by an MIT-style
# license that can be found in the LICENSE file or at
# https://opensource.org/licenses/MIT.

cmake_minimum_required(VERSION 3.18)

if (NOT DEFINED WABT_SOURCES_ROOT_DIR)
	message(FATAL_ERROR "Failed to find WABT source directory")
endif()

if(${DECENT_WASM_COUNTER_ENABLE_ENCLAVE})
endif()

add_library(DecentWasmCounter_untrusted STATIC DecentWasmCounter.cpp)

target_link_libraries(DecentWasmCounter_untrusted DecentWasmWat_untrusted)

target_compile_options(DecentWasmCounter_untrusted
	PRIVATE "$<$<CONFIG:Debug>:${DEBUG_OPTIONS}>"
			"$<$<CONFIG:Release>:${RELEASE_OPTIONS}>")

target_include_directories(DecentWasmCounter_untrusted
	PUBLIC ${DECENT_WASM_COUNTER_INCLUDE_DIR}
	PRIVATE ${WABT_SOURCES_ROOT_DIR})

set_property(TARGET DecentWasmCounter_untrusted PROPERTY
	MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
set_property(TARGET DecentWasmCounter_untrusted PROPERTY CXX_STANDARD 17)
