cmake_minimum_required(VERSION 4.3)
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)


set(PROJECT "fluff")
set(LIB "fluff")
set(SRC_DIR "src")
set(TEST_DIR "test")
set(INCLUDE_DIR "include")

option(FLUFF_EXTENSION_MPFR "Enable MPFR/GMP support" OFF)

option(FLUFF_BUILD_TESTS "Build tests" OFF)

set(TEST_DIR ${CMAKE_CURRENT_SOURCE_DIR}/${TEST_DIR})

project(${PROJECT})

file(GLOB_RECURSE SRC_FILES CONFIGURE_DEPENDS ${SRC_DIR}/*.cpp)
file(GLOB_RECURSE HEADER_FILES CONFIGURE_DEPENDS ${INCLUDE_DIR}/*.hpp)


if(NOT USE_MPFR)
	list(FILTER SRC_FILES EXCLUDE REGEX "extensions/mpfr/")
	list(FILTER HEADER_FILES EXCLUDE REGEX "extensions/mpfr/")
	list(FILTER HEADER_FILES EXCLUDE REGEX "extensions/mpfr.hpp")
endif()

add_library(${LIB} STATIC ${SRC_FILES} ${HEADER_FILES})
add_library(${LIB}::${LIB} ALIAS ${LIB})
set_target_properties(${LIB} PROPERTIES LINKER_LANGUAGE CXX)
set_property(TARGET ${LIB} PROPERTY VERIFY_INTERFACE_HEADER_SETS ON)

target_include_directories(${LIB} SYSTEM PUBLIC ${INCLUDE_DIR})

target_compile_definitions(${LIB} PUBLIC $<$<CONFIG:Debug>:DEBUG>)

target_compile_options(${LIB} PRIVATE
	-Wall
	-Wextra
	-Wpedantic
	-Wswitch

	-O3

	# -lto
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

	target_link_libraries(${LIB}_test PRIVATE
		doctest
	)

	file(GLOB_RECURSE TEST_FILES CONFIGURE_DEPENDS ${TEST_DIR}/*.cpp)
	target_sources(${LIB}_test PRIVATE ${TEST_FILES})
endif (BUILD_TESTS)

# check for ODR violations
set(ODR_INCLUDES "")
foreach(hdr IN LISTS HEADER_FILES)
	string(APPEND ODR_INCLUDES "#include <${hdr}>\n")
endforeach()

include(CMakePrintHelpers)

set(ODR_DIR ${CMAKE_CURRENT_BINARY_DIR}/odr)
file(CONFIGURE OUTPUT ${ODR_DIR}/tu_a.cpp CONTENT "${ODR_INCLUDES}int main() {}\n" @ONLY)
file(CONFIGURE OUTPUT ${ODR_DIR}/tu_b.cpp CONTENT "${ODR_INCLUDES}" @ONLY)

add_executable(odr_check ${ODR_DIR}/tu_a.cpp ${ODR_DIR}/tu_b.cpp)
target_link_libraries(odr_check PRIVATE ${LIB})

# set(ODR_SOURCES "")
# foreach(hdr IN LISTS HEADER_FILES)
# 	string(MAKE_C_IDENTIFIER "${hdr}" id)
# 	foreach(copy a b)
# 		set(src ${CMAKE_CURRENT_BINARY_DIR}/odr/${id}_${copy}.cpp)
# 		file(CONFIGURE OUTPUT ${src} CONTENT "#include <${hdr}>\n" @ONLY)
# 		list(APPEND ODR_SOURCES ${src})
# 	endforeach()
# endforeach()
# 
# file(CONFIGURE OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/odr/main.cpp CONTENT "int main() {}\n" @ONLY)
# add_executable(odr_check ${ODR_SOURCES} ${CMAKE_CURRENT_BINARY_DIR}/odr/main.cpp)
# target_link_libraries(odr_check PRIVATE ${LIB})
