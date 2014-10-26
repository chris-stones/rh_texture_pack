#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ([2.69])
AC_INIT(rh_texture_loader, 0.1, chris.stones@gmail.com)
AC_CONFIG_AUX_DIR(config)
AC_CONFIG_SRCDIR(rh_texture_loader.c)
AC_CONFIG_HEADERS(config.h)

AC_CONFIG_MACRO_DIR([m4])

LT_INIT
AM_INIT_AUTOMAKE

AC_PROG_CXX
AC_PROG_CC
AC_PROG_INSTALL
#AC_FUNC_ALLOCA
#AC_FUNC_MALLOC
AC_CHECK_HEADERS([stdlib.h string.h])

AC_ARG_WITH([gl], AS_HELP_STRING([--with-gl], [Use OpenGL to create textures.]))
AC_ARG_WITH([gles], AS_HELP_STRING([--with-gles], [Use OpenGLES to create textures.]))

AC_ARG_ENABLE(
  [compressed-3d-textures],
  AS_HELP_STRING([--enable-compressed-3d-textures], [Enable 3D Compressed Textures.]),
  [AC_DEFINE(COMPRESSED_3D_TEXTURES, 1, [Enable loading 3d textures in a compressed format])],
  [AC_DEFINE(NCOMPRESSED_3D_TEXTURES, 1, [Enable loading 3d textures in a compressed format])]
)

dnl RASPBERY PI - Try to help the user as much as possible! 
AC_ARG_WITH(
  [raspberrypi],
  AS_HELP_STRING([--with-raspberrypi=SYSROOT_DIR], [Target RaspberryPi. Selects --with-gles]),
  [
    with_gles="yes"
    RPI_SYSROOT="$withval"
    CPPFLAGS="$CPPFLAGS -I$RPI_SYSROOT/opt/vc/include"
    LDFLAGS="$LDFLAGS -L$RPI_SYSROOT/opt/vc/lib -Wl,--rpath-link,$RPI_SYSROOT/opt/vc/lib,--rpath,/opt/vc/lib"
  ],
  []
)

AS_IF([test "x$with_gl" = "xyes"],
[
  AC_SEARCH_LIBS(
    [glClear],[GL],[],
    AC_MSG_ERROR([Unable to find OpenGL library]))
  AC_CHECK_HEADERS([GL/gl.h],[],[AC_MSG_ERROR([Missing GL/gl.h])])
  AC_CHECK_HEADERS(
    [GL/glext.h],[],
    [AC_MSG_ERROR([Missing GL/glext.h])],[#include<GL/gl.h>])
])

AS_IF([test "x$with_gles" = "xyes"],
[
  AC_SEARCH_LIBS(
    [glClear],[GLESv2],[],
    AC_MSG_ERROR([Unable to find OpenGLES library (libGLESv2.so)]))
  AC_CHECK_HEADERS([GLES2/gl2.h],[],[AC_MSG_ERROR([Missing GLES2/gl2.h])])
  AC_CHECK_HEADERS(
    [GLES2/gl2ext.h],[],
    [AC_MSG_ERROR([Missing GLES2/gl2ext.h])],[#include<GLES2/gl2.h>])
])

# Checks for typedefs, structures, and compiler characteristics.
AC_CHECK_HEADER_STDBOOL
AC_C_INLINE
AC_TYPE_OFF_T
AC_TYPE_SIZE_T

# Checks for library functions.
AC_CHECK_FUNCS([memset strchr strstr])

AC_CONFIG_FILES([Makefile])
AC_OUTPUT
