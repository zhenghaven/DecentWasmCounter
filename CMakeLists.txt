# Copyright (c) 2022 Haofan Zheng
# Use of this source code is governed by an MIT-style
# license that can be found in the LICENSE file or at
# https://opensource.org/licenses/MIT.

cmake_minimum_required(VERSION 3.18)

project(DecentWasmCounter LANGUAGES CXX VERSION 0.0.1)

option(DECENT_WASM_COUNTER_ENABLE_ENCLAVE "Enable decent targets" ON)
option(DECENT_WASM_COUNTER_ENABLE_TEST    "Enable test targets"   OFF)

if(DEFINED DECENT_FRAMEWORK_ENABLE_ENCLAVE)
	set(DECENT_WASM_COUNTER_ENABLE_ENCLAVE ${DECENT_FRAMEWORK_ENABLE_ENCLAVE})
endif()

# Compile options

if(MSVC)
	set(COMMON_OPTIONS /we4239 /we4002 /we4700 /we4305 /EHsc /MP
		/GR /Zc:__cplusplus)
	set(DEBUG_OPTIONS /Od /Zi /DDEBUG)
	set(RELEASE_OPTIONS /Ox /Oi /Ob2 /fp:fast)# /DNDEBUG
#	set(CMAKE_EXE_LINKER_FLAGS_DEBUG ${CMAKE_EXE_LINKER_FLAGS_DEBUG} /DEBUG)
else()
	set(COMMON_OPTIONS -pthread)
	set(DEBUG_OPTIONS -O0 -g -DDEBUG)
	set(RELEASE_OPTIONS -O2) #-DNDEBUG defined by default
endif()

set(DEBUG_OPTIONS ${COMMON_OPTIONS} ${DEBUG_OPTIONS})
set(RELEASE_OPTIONS ${COMMON_OPTIONS} ${RELEASE_OPTIONS})

##################################################
# Setup CMake environment for enclave targets
##################################################

if(${DECENT_WASM_COUNTER_ENABLE_ENCLAVE})
	#Removed Basic Runtime Checks in MSVC
	if(MSVC)
		STRING (REGEX REPLACE "/RTC(su|[1su])" ""
			CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG}")
		STRING (REGEX REPLACE "/RTC(su|[1su])" ""
			CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE}")
		STRING (REGEX REPLACE "/RTC(su|[1su])" ""
			CMAKE_C_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG}")
		STRING (REGEX REPLACE "/RTC(su|[1su])" ""
			CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE}")
	endif()

	#Remove all standard libraries dependency here so that enclave DLL can be
	# compiled properly. And it will be added back later for non-enclave apps.
	set(UNTRUSTED_CXX_STANDARD_LIBRARIES ${CMAKE_CXX_STANDARD_LIBRARIES_INIT})
	set(UNTRUSTED_C_STANDARD_LIBRARIES ${CMAKE_C_STANDARD_LIBRARIES_INIT})
	set(CMAKE_CXX_STANDARD_LIBRARIES "")
	set(CMAKE_C_STANDARD_LIBRARIES "")

	# Add DebugSimulation to CMake configuration types
	set(CMAKE_CONFIGURATION_TYPES Release Debug DebugSimulation)
	set_property(GLOBAL PROPERTY DEBUG_CONFIGURATIONS Debug DebugSimulation)

	set(CMAKE_CXX_FLAGS_DEBUGSIMULATION           ${CMAKE_CXX_FLAGS_DEBUG})
	set(CMAKE_C_FLAGS_DEBUGSIMULATION             ${CMAKE_C_FLAGS_DEBUG})
	set(CMAKE_EXE_LINKER_FLAGS_DEBUGSIMULATION    ${CMAKE_EXE_LINKER_FLAGS_DEBUG})
	set(CMAKE_SHARED_LINKER_FLAGS_DEBUGSIMULATION ${CMAKE_SHARED_LINKER_FLAGS_DEBUG})
	set(CMAKE_STATIC_LINKER_FLAGS_DEBUGSIMULATION ${CMAKE_STATIC_LINKER_FLAGS_DEBUG})
endif()

##################################################
# Fetch external dependencies
##################################################

include(FetchContent)

if(${DECENT_WASM_COUNTER_ENABLE_TEST})
	# Setup WABT
	FetchContent_Declare(
		git_wabt_decent_sgx
		GIT_REPOSITORY https://github.com/zhenghaven/wabt.git
		GIT_TAG        origin/decent-enclave
		SOURCE_SUBDIR  decent/sgx
	)
	FetchContent_MakeAvailable(git_wabt_decent_sgx)
endif()

##################################################
# Add source directories
##################################################

set(DECENT_WASM_COUNTER_INCLUDE_DIR ${CMAKE_CURRENT_LIST_DIR}/include)

add_subdirectory(src)

if(${DECENT_WASM_COUNTER_ENABLE_TEST})
	enable_testing()
	add_subdirectory(test)
endif()
