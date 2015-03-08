AC_PREREQ([2.69])
AC_INIT(rh_texture_packer, 0.1, chris.stones@gmail.com)
AC_CONFIG_AUX_DIR(config)
AC_CONFIG_SRCDIR(src/main.cpp)
AC_CONFIG_HEADERS(src/config.h)

AC_CONFIG_MACRO_DIR([m4])

AM_INIT_AUTOMAKE([subdir-objects])

AC_PROG_CC
AC_PROG_CXX
AC_PROG_INSTALL

AX_BOOST_BASE([1.57.0],[],AC_MSG_ERROR([Missing Boost >= 1.57.0]))
AX_BOOST_SYSTEM
AX_BOOST_FILESYSTEM

AC_SEARCH_LIBS(
  [imgAllocAndRead],[img],[],
  AC_MSG_ERROR([Unable to find libimg library]))  
AC_CHECK_HEADERS([libimg.h],[],[AC_MSG_ERROR([Missing libimg.h])])

AC_SEARCH_LIBS(
  [imguCopyImage],[imgutil],[],
  AC_MSG_ERROR([Unable to find libimgutil library]))  
AC_CHECK_HEADERS([libimgutil.h],[],[AC_MSG_ERROR([Missing libimgutil.h])])

AC_CONFIG_FILES([
	Makefile
	src/Makefile
])
AC_OUTPUT

