cmake_minimum_required(VERSION 4.3)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)


set(PROJECT "fluff")
set(LIB "fluff")
set(SRC_DIR "src")
set(TEST_DIR "test")
set(INCLUDE_DIR "include")

option(FLUFF_BUILD_TESTS "Build tests" OFF)
option(FLUFF_USE_MPFR "Enable MPFR/GMP support" OFF)

set(TEST_DIR ${CMAKE_CURRENT_SOURCE_DIR}/${TEST_DIR})

project(${PROJECT})

file(GLOB_RECURSE SRC_FILES CONFIGURE_DEPENDS ${SRC_DIR}/*.cpp)
file(GLOB_RECURSE INCLUDE_FILES CONFIGURE_DEPENDS ${INCLUDE_DIR}/*.hpp)

set(PUBLIC_HEADERS
    ${INCLUDE_DIR}/fluff/benchmark
    ${INCLUDE_DIR}/fluff/containers
    ${INCLUDE_DIR}/fluff/hashing
    ${INCLUDE_DIR}/fluff/math
    ${INCLUDE_DIR}/fluff/memory
    ${INCLUDE_DIR}/fluff/random
    ${INCLUDE_DIR}/fluff/extensions/mpfr
)


if(NOT USE_MPFR)
	list(FILTER SRC_FILES EXCLUDE REGEX "extensions/mpfr/")
	list(FILTER INCLUDE_FILES EXCLUDE REGEX "extensions/mpfr/")
endif()

add_library(${LIB} STATIC ${SRC_FILES} ${INCLUDE_FILES} ${PUBLIC_HEADERS})
add_library(${LIB}::${LIB} ALIAS ${LIB})
set_target_properties(${LIB} PROPERTIES LINKER_LANGUAGE CXX)

target_include_directories(${LIB} SYSTEM PUBLIC ${INCLUDE_DIR})

target_compile_definitions(${LIB} PUBLIC $<$<CONFIG:Debug>:DEBUG>)

target_compile_options(${LIB} PRIVATE
	-Wall
	-Wextra
	-Wpedantic
	-O3
	# -march=native
	# -ffast-math

	# -fno-inline
	# -fno-omit-frame-pointer
	# -g
)


if (USE_MPFR)
	enable_language(C)
	add_subdirectory(third_party/gmp-6.3.0)
	add_subdirectory(third_party/mpfr-4.2.2)

	target_link_libraries(${LIB} PUBLIC
		GMP::GMPXX
		# GMP::GMP
		MPFR::MPFR
	)

	target_compile_definitions(${LIB} PUBLIC USE_MPFR)
endif()

if (BUILD_TESTS)
	add_executable(${LIB}_test ${TEST_MAIN})
	target_link_libraries(${LIB}_test PUBLIC ${LIB})

	target_compile_definitions(${LIB}_test PRIVATE DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN)

	add_subdirectory(third_party/doctest)
	add_subdirectory(third_party/gmp-6.3.0)

	target_link_libraries(${LIB}_test PUBLIC
		doctest
	)

	file(GLOB_RECURSE TEST_FILES CONFIGURE_DEPENDS ${TEST_DIR}/*.cpp)
	target_sources(${LIB}_test PRIVATE ${TEST_FILES})
endif (BUILD_TESTS)
