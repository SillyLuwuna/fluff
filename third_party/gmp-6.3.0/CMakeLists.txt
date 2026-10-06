include(ExternalProject)
include(ProcessorCount)
ProcessorCount(NPROC)

option(GMP_BUILD_SHARED "Build GMP as a shared library" ON)

set(GMP_VERSION 6.3.0)
set(GMP_PREFIX ${CMAKE_CURRENT_BINARY_DIR}/install)
set(GMP_PREFIX ${GMP_PREFIX} PARENT_SCOPE)

if(GMP_BUILD_SHARED)
  set(_linkage --enable-shared --disable-static)
  set(_suffix ${CMAKE_SHARED_LIBRARY_SUFFIX})
  set(_type SHARED)
else()
  set(_linkage --disable-shared --enable-static --with-pic)
  set(_suffix ${CMAKE_STATIC_LIBRARY_SUFFIX})
  set(_type STATIC)
endif()

set(_gmp_lib   ${GMP_PREFIX}/lib/libgmp${_suffix})
set(_gmpxx_lib ${GMP_PREFIX}/lib/libgmpxx${_suffix})

ExternalProject_Add(gmp_ext
  # SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/third_party/gmp-${GMP_VERSION}
  SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}
  # URL https://ftpmirror.gnu.org/gnu/gmp/gmp-${GMP_VERSION}.tar.xz
  # URL_HASH SHA256=<checksum from gmplib.org or gnu.org>
  DOWNLOAD_EXTRACT_TIMESTAMP TRUE
  CONFIGURE_COMMAND
    <SOURCE_DIR>/configure
      "CC=${CMAKE_C_COMPILER} -std=gnu17"
      "CXX=${CMAKE_CXX_COMPILER}"
      --prefix=${GMP_PREFIX}
      --enable-cxx
      ${_linkage}  BUILD_COMMAND make -j${NPROC}
  INSTALL_COMMAND make install
  BUILD_BYPRODUCTS ${_gmp_lib} ${_gmpxx_lib}
)

file(MAKE_DIRECTORY ${GMP_PREFIX}/include)

add_library(GMP::GMP ${_type} IMPORTED GLOBAL)
set_target_properties(GMP::GMP PROPERTIES
  IMPORTED_LOCATION ${_gmp_lib}
  INTERFACE_INCLUDE_DIRECTORIES ${GMP_PREFIX}/include)
add_dependencies(GMP::GMP gmp_ext)

add_library(GMP::GMPXX ${_type} IMPORTED GLOBAL)
set_target_properties(GMP::GMPXX PROPERTIES
  IMPORTED_LOCATION ${_gmpxx_lib}
  INTERFACE_INCLUDE_DIRECTORIES ${GMP_PREFIX}/include
  INTERFACE_LINK_LIBRARIES GMP::GMP)
add_dependencies(GMP::GMPXX gmp_ext)
