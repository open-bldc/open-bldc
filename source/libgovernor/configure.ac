dnl libgovernor - Open-BLDC configuration and debug protocol library
dnl Copyright (C) 2010 by Piotr Esden-Tempski <piotr@esden.net>
dnl
dnl This program is free software; you can redistribute it and/or modify
dnl it under the terms of the GNU General Public License as published by
dnl the Free Software Foundation; either version 3 of the License, or
dnl (at your option) any later version.
dnl
dnl This program is distributed in the hope that it will be useful,
dnl but WITHOUT ANY WARRANTY; without even the implied warranty of
dnl MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
dnl GNU General Public License for more details.
dnl
dnl You should have received a copy of the GNU General Public License
dnl along with this program.  If not, see <http://www.gnu.org/licenses/>.

AC_PREREQ(2.50)

AC_INIT([libgovernor], [0.2], [open-bldc@open-bldc.org])

#===============================
# cross compilation
#===============================

case "$host" in
	arm-*)
		CFLAGS="$CFLAGS -mcpu=cortex-m3 -mthumb -mfix-cortex-m3-ldrd -ffunction-sections -fdata-sections -msoft-float -g"
		LDFLAGS="$LDFLAGS -T${srcdir}/../firmware/src/stm32.ld -nostartfiles -mthumb -march=armv7 -mfix-cortex-m3-ldrd -msoft-float"
	;;
esac

# =============================
# init libtool
# =============================
LT_INIT
dnl AC_CONFIG_SRCDIR([src/master.c])
AM_INIT_AUTOMAKE([libgovernor], [0.1])

AC_COPYRIGHT([Copyright (C) 2010-2011 Piotr Esden-Tempski])
AC_DEFINE([COPYRIGHT],["Copyright (C) 2010-2011 Piotr Esden-Tempski <piotr@esden.net>"],[Software package copyright])
AC_DEFINE([LICENSE],["License GPLv3+: GNU GPL version 3 or later <http://gnu.org/licenses/gpl.html>"],[Software package license])

AC_CANONICAL_HOST
AC_DEFINE_UNQUOTED([HOST],["$host"],[The canonical host])

# If the user didn't supplied a CFLAGS value,
# set it to the default of -O2 -g -Wall.
#test -z "$CFLAGS" && CFLAGS="-O2 -g -Wall"

AC_SUBST(EXTRACFLAGS, "-Wall -Wextra -Wshadow -I${srcdir}/include")
AC_SUBST(CHECK_EXTRACFLAGS, "-Wall -Wextra -Wshadow")
AC_SUBST(EXTRALDFLAGS, "")
AC_SUBST(CHECK_EXTRALDFLAGS, "")

# ===================
# Check for programs.
# ===================

dnl AC_PROG_CC
dnl AC_ISC_POSIX
dnl AC_PROG_CC_STDC
dnl AC_PROG_RANLIB
dnl AC_PROG_LN_S
AC_PROG_LIBTOOL

AC_CACHE_SAVE

# ========================
# Checks for header files.
# ========================

AC_HEADER_STDC
AC_HEADER_DIRENT
dnl AC_CHECK_HEADERS(
dnl [stdbool.h] dnl
dnl [stdout.h] dnl
dnl [string.h] dnl
dnl [stdlib.h] dnl
dnl [unistd.h] dnl
dnl )
AC_HEADER_TIME
AC_HEADER_STAT

# ==============================================================
# Checks for typedefs, structures, and compiler characteristics.
# ==============================================================

dnl AM_C_PROTOTYPES
AC_C_CONST
AC_TYPE_UID_T     dnl This checks for gid_t too.
dnl AC_CHECK_TYPES([ssize_t])

# =============================
# Checks for library functions.
# =============================

AC_CHECK_DECLS([malloc, free, memrchr, strnlen])
AC_CHECK_FUNCS([sleep usleep])

AC_FUNC_VPRINTF

AC_REPLACE_FUNCS([memrchr stpcpy strndup strcasecmp])

# for error.c
AC_CHECK_FUNCS([strerror])
AC_CHECK_DECLS([strerror])
AC_FUNC_STRERROR_R

#===============================
# Werror
#===============================
AC_ARG_ENABLE([werror],
   [AC_HELP_STRING([--disable-werror], [disable -Werror @<:@default=enabled@:>@])],
   [
    if test "x${enableval}" = "xyes" ; then
       werror="yes"
    else
       werror="no"
    fi
   ],
   [werror="yes"])

if test "x$werror" == "xyes";
then
	EXTRACFLAGS="$EXTRACFLAGS -Werror"
	CHECK_EXTRACFLAGS="$CHECK_EXTRACFLAGS -Werror"
fi

#==============================
# code coverage stuff
#==============================
AC_ARG_ENABLE(gcov, AC_HELP_STRING([--enable-gcov],[turn on code coverage analysis tools]))

AC_CHECK_PROGS(LCOV, lcov, false)
AC_CHECK_PROGS(GENHTML, genhtml, false)

dnl Check for --enable-gcov and add appropriate flags for gcc
dnl Note that these extra switches are NOT applied to the loader
AC_SUBST(GCOV_ENABLED, "")
if test "x$enable_gcov" = "xyes";
then
	GCOV_ENABLED="yes"
	EXTRACFLAGS="$EXTRACFLAGS -fprofile-arcs -ftest-coverage"
	dnl Turn off optimization so code coverage tool
	dnl can get accurate line numbers
	CFLAGS=`echo "$CFLAGS" | sed -e 's/-O[0-9]//g'`
	EXTRALDFLAGS="$EXTRALDFLAGS -lgcov"
fi

#===============================
# Doxygen support
#===============================
#DX_HTML_FEATURE(ON)
#DX_CHM_FEATURE(OFF)
#DX_CHI_FEATURE(OFF)
#DX_MAN_FEATURE(OFF)
#DX_RTF_FEATURE(OFF)
#DX_XML_FEATURE(OFF)
#DX_PDF_FEATURE(ON)
#DX_PS_FEATURE(OFF)
#
#DX_INIT_DOXYGEN(LG, doxygen.cfg, doc/doxygen)

# ===============================
# Test for check unittest library
# ===============================

AC_ARG_ENABLE(check, AC_HELP_STRING([--enable-check],[turn on check unittest library]))
if test "x$enable_check" = "xyes";
then
   PKG_CHECK_MODULES([CHECK], [check >= 0.9.4])
fi

# ============
# Output files
# ============

# Put config.h in the base directory because libintl expects it here,
# but keep src/config.h.in in the src/ directory to keep the base directory
# as clean as possible.
dnl Don't quote the following argument, it is parsed by automake.
AM_CONFIG_HEADER(include/config.h:include/config.h.in)

AC_CONFIG_FILES([
 Makefile
 src/Makefile
 include/Makefile
 tests/Makefile
 doc/Makefile
])
AC_OUTPUT

eval bindir="$bindir"
eval bindir="$bindir"
eval datadir="$datadir"
eval mandir="$mandir"

echo "
libgovernor configuration
--------------------
libgovernor version      : $VERSION
host                     : $host

binary install path      : $bindir
data install path        : ${datadir}/libgovernor
manual page install path : $mandir

compiler                 : $CC
general options
  CFLAGS                 : $CFLAGS $EXTRACFLAGS
  LIBS                   : $LDFLAGS $LIBS $EXTRALDFLAGS
  CHECK_CFLAGS           : $CFLAGS $CHECK_EXTRACFLAGS
  CHECK_LIBS             : $LDFLAGS $LIBS $CHECK_EXTRALDFLAGS"
