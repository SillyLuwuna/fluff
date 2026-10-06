include(ExternalProject)
include(ProcessorCount)
ProcessorCount(NPROC)

set(MPFR_VERSION 4.2.1)   # check mpfr.org for the current release
set(MPFR_PREFIX ${CMAKE_CURRENT_BINARY_DIR}/install)

if(GMP_BUILD_SHARED)
  set(_linkage --enable-shared --disable-static)
  set(_suffix ${CMAKE_SHARED_LIBRARY_SUFFIX})
  set(_type SHARED)
else()
  set(_linkage --disable-shared --enable-static --with-pic)
  set(_suffix ${CMAKE_STATIC_LIBRARY_SUFFIX})
  set(_type STATIC)
endif()

set(_mpfr_lib ${MPFR_PREFIX}/lib/libmpfr${_suffix})

ExternalProject_Add(mpfr_ext
  DEPENDS gmp_ext
  # URL https://ftpmirror.gnu.org/gnu/mpfr/mpfr-${MPFR_VERSION}.tar.xz
  SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}
  # URL_HASH SHA256=<checksum from mpfr.org or gnu.org>
  DOWNLOAD_EXTRACT_TIMESTAMP TRUE
  PATCH_COMMAND
  ${CMAKE_COMMAND} -E touch
    <SOURCE_DIR>/aclocal.m4
    <SOURCE_DIR>/configure
    <SOURCE_DIR>/config.h.in
    <SOURCE_DIR>/Makefile.in
  CONFIGURE_COMMAND
    <SOURCE_DIR>/configure
      "CC=${CMAKE_C_COMPILER}"
      "LDFLAGS=-Wl,-rpath,${GMP_PREFIX}/lib"
      --prefix=${MPFR_PREFIX}
      --with-gmp=${GMP_PREFIX}
      ${_linkage}
  BUILD_COMMAND make -j${NPROC}
  INSTALL_COMMAND make install
  BUILD_BYPRODUCTS ${_mpfr_lib}
)

file(MAKE_DIRECTORY ${MPFR_PREFIX}/include)

add_library(MPFR::MPFR ${_type} IMPORTED GLOBAL)
set_target_properties(MPFR::MPFR PROPERTIES
  IMPORTED_LOCATION ${_mpfr_lib}
  INTERFACE_INCLUDE_DIRECTORIES ${MPFR_PREFIX}/include
  INTERFACE_LINK_LIBRARIES GMP::GMP)
add_dependencies(MPFR::MPFR mpfr_ext)
