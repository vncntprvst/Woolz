##
# \file         configure.in
# \author       Bill Hill
# \date         September 2007
# \version      $Id$
# \par
# Address:
#               MRC Human Genetics Unit,
#               MRC Institute of Genetics and Molecular Medicine,
#               University of Edinburgh,
#               Western General Hospital,
#               Edinburgh, EH4 2XU, UK.
# \par
# Copyright (C), [2012],
# The University Court of the University of Edinburgh,
# Old College, Edinburgh, UK.
# 
# This program is free software; you can redistribute it and/or
# modify it under the terms of the GNU General Public License
# as published by the Free Software Foundation; either version 2
# of the License, or (at your option) any later version.
#
# This program is distributed in the hope that it will be
# useful but WITHOUT ANY WARRANTY; without even the implied
# warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
# PURPOSE.  See the GNU General Public License for more
# details.
#
# You should have received a copy of the GNU General Public
# License along with this program; if not, write to the Free
# Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
# Boston, MA  02110-1301, USA.
# \brief        Process this file with autoconf to produce a configure script.
##

m4_define([WOOLZ_VERSION], [1.5.1])

AC_INIT(Woolz, WOOLZ_VERSION, ma-tech@hgu.mrc.ac.uk)
AC_CANONICAL_TARGET

AC_GNU_SOURCE

AM_INIT_AUTOMAKE

AC_CONFIG_SRCDIR([libWlz/Wlz.h])

# Set defaults
AC_PREFIX_DEFAULT(/opt/MouseAtlas)
AC_DISABLE_SHARED
CP="cp"
RM="rm -f"
RMRF="rm -rf"
CDEF_ARCH=""
USE_CTAGS=1
USE_ETAGS=0
ENABLE_EXTFF="no"
HAVE_JPEG="no"
JPEG_DIR="no"
HAVE_NIFTI="no"
NIFTI_DIR="no"
HAVE_TIFF="no"
TIFF_DIR="no"
ENABLE_JAVA="no"
HAVE_JNI="no"
JDK_DIR="no"
JAR="no"
JAVA="no"
JAVAC="no"
JAVACC="no"
ENABLE_TEST="no"
AC_SUBST(CP)
AC_SUBST(RM)
AC_SUBST(RMRF)
AC_SUBST(ENABLE_EXTFF)

# Check CPU type
echo "Checking target cpu/system type"
case ${target_cpu} in
  i386|i486|i586|i686)
    CDEF_ARCH="-D__x86"
    ;;
  x86_64|amd64)
    CDEF_ARCH="-D__x86"
    ;;
  ia64)
    CDEF_ARCH="-D__x86"
    ;;
  alpha)
    CDEF_ARCH="-D__alpha"
    ;;
  sparc|sparc64)
    CDEF_ARCH="-D__sparc"
    ;;
  ppc|powerpc)
    CDEF_ARCH="-D__ppc"
    ;;
  mips)
    CDEF_ARCH="-D__mpis"
    ;;
  *)
    echo "Warning: CPU type ${target_cpu} unknown."
    ;;
esac

# Check for programs.
AC_PROG_CC
AC_PROG_INSTALL
AC_PROG_LIBTOOL
AC_PROG_MKDIR_P
AC_PROG_RANLIB

# Check for system functions
AC_FUNC_MMAP

# Check for libraries.
AC_CHECK_LIB(m, pow)

# Check for header files.
AC_HEADER_STDC
AC_CHECK_HEADERS([	assert.h \
			ctype.h \
			errno.h \
			float.h \
			limits.h \
			stddef.h \
			stdlib.h \
			string.h \
			strings.h \
                  	sys/time.h \
                  	sys/types.h \
			time.h \
			unistd.h])

# Check for typedefs, structures, and compiler characteristics.
AC_C_CONST
AC_TYPE_SIZE_T
AC_HEADER_TIME
AC_STRUCT_TM

# Check for library functions.
AC_FUNC_MKTIME
AC_FUNC_STAT
AC_FUNC_VPRINTF
AC_CHECK_FUNCS([	floor \
			getcwd \
			gethostname \
			gettimeofday \
		        malloc \
			memset \
			mkdir \
			modf \
			pow \
			realloc \
			regcmp \
			regcomp \
			sqrt \
			strcasecmp \
			strchr \
			strerror \
			strrchr \
                        strstr \
                        strtok \
                        strtok_r])


# Check for doxygen
AC_PATH_PROG(DOXYGEN, doxygen)
AC_CHECK_PROG(HAVE_DOXYGEN, doxygen, "yes", "no")
AM_CONDITIONAL(HAVE_DOXYGEN, test x$HAVE_DOXYGEN = xyes)

# Compilation and linking flags
CFLAGS=""
INCL_EXTFF=""
LIBS_EXTFF="-lz"
AC_SUBST(LIBS_EXTFF)
AC_SUBST(INCL_EXTFF)

AC_PROG_CC

AC_ARG_ENABLE(icc,
  [  --enable-icc          build using the Intel C compiler],
  GCC="no"
  CC="icc"
  CXX="icpc"
  LD="xild"
  AR="xiar"
  )

if test "$CC" = "gcc"
then
  CFLAGS="${CFLAGS} -pipe -Wall -Wno-deprecated"
fi
if test "$CC" = "icc"
then
  CFLAGS="${CFLAGS} -w2"
fi

CFLAGS="${CFLAGS} ${CDEF_ARCH}"

# Set target architecture flags
if test "$CC" = "gcc"
then
  case ${target_cpu} in
    i386|i486|i586)
      CFLAGS="${CFLAGS} -m32 -march=i586"
      ;;
    i686)
      CFLAGS="${CFLAGS} -m32 -march=i686"
      ;;
    x86_64|amd64)
      CFLAGS="${CFLAGS} -m64"
      ;;
    *)
      ;;
  esac
else
  #Intel CC
  if test "$CC" = "icc"
  then
    case ${target_cpu} in
      i386|i486|i586)
	CFLAGS="${CFLAGS} -m32 -xK"
	;;
      i686)
	CFLAGS="${CFLAGS}  -xN"
	;;
      x86_64|amd64)
	CFLAGS="${CFLAGS} -m64"
	;;
      *)
	;;
    esac
  fi
fi

CPP="cpp"

AC_ARG_ENABLE(debug,
  [  --enable-debug	build with debugging flags set],
    CFLAGS="${CFLAGS} -g -DDEBUG"
    LDFLAGS="${LDFLAGS} -g -DDEBUG")

AC_ARG_ENABLE(optimise,
  [  --enable-optimise	build with optimisation flags set],
  [
    # GNU CC
    if test "$CC" = "gcc"
    then
      case ${target_cpu} in
	i386|i486|i586|i686)
	  CFLAGS="${CFLAGS} -fPIC -O2 -fomit-frame-pointer"
	  ;;
	x86_64|amd64)
          CFLAGS="${CFLAGS} -fPIC -O3 -msse3"
	  ;;
        *)
          CFLAGS="${CFLAGS} -fPIC -O2"
	  ;;
      esac
    else
      #Intel CC
      if test "$CC" = "icc"
      then
	CFLAGS="${CFLAGS} -fPIC -O3 -ip"
      else
	# AIX CC
	if test "`uname`" = "AIX"
	then
	  CFLAGS="${CFLAGS} -O3"
	fi
	# SGI CC
	if test "`uname`" = "IRIX" || test "`uname`" = "IRIX64"
	then
	  CFLAGS="${CFLAGS} -Ofast"
	  LDDFLAGS="${LDFLAGS} -IPA"
	fi
	# HP-UX CC
	if test "`uname`" = "HP-UX"
	then
	  CFLAGS="${CFLAGS} -fast"
	fi
	# Sun CC
	if test "`uname`" = "SunOS"
	then
	  CFLAGS="${CFLAGS} -fast"
	fi
      fi
    fi
    ])

AC_ARG_ENABLE(openmp,
  [  --enable-openmp	build with openmp flags set],
    # GNU CC
    if test "$CC" = "gcc"
    then
      CFLAGS="${CFLAGS} -fopenmp"
    fi
    # Intel CC
    if test "$CC" = "icc"
    then
      CFLAGS="${CFLAGS} -openmp"
    fi)

AC_ARG_ENABLE(profile,
  [  --enable-profile	build with profiling set],
    CFLAGS="${CFLAGS} -pg"
    LDFLAGS="${LDFLAGS} -pg")

AC_ARG_ENABLE(extff,
  [  --enable-extff	build with external file format support],
     ENABLE_EXTFF="yes",
     ENABLE_EXTFF="no")
AM_CONDITIONAL([BUILD_EXTFF], [test x"$ENABLE_EXTFF" = x"yes"])

AC_ARG_ENABLE(java,
  [  --enable-java	        build with java bindings],
     ENABLE_JAVA="yes",
     ENABLE_JAVA="no")
AM_CONDITIONAL([BUILD_JAVA], [test x"$ENABLE_JAVA" = x"yes"])

AC_ARG_ENABLE(test,
  [  --enable-test		build test programs],
     ENABLE_TEST="yes",
     ENABLE_TEST="no")
AM_CONDITIONAL([BUILD_TEST], [test x"$ENABLE_TEST" = x"yes"])

if test ${USE_CTAGS} = 1
then
  AC_CHECK_PROG(GNC_CTAGS_FILE, ctags, tags)
fi

AC_CONFIG_FILES([	Makefile \
			libAlc/Makefile \
			libAlg/Makefile \
			libWlz/Makefile \
			binWlz/Makefile \
			binAlgTst/Makefile \
			binWlzTst/Makefile ])

if test x${ENABLE_EXTFF} = "xyes"
then
  AC_ARG_WITH(jpeg, [  --with-jpeg=DIR         Directory containing the jpeg
                          lib and include directories.],
	      JPEG_DIR=$withval, JPEG_DIR=no)
  if test ${JPEG_DIR} = "no"
  then
    AC_CHECK_HEADER(jpeglib.h,
		    AC_CHECK_LIB(jpeg, jpeg_CreateCompress,
				 [
				   HAVE_JPEG=yes
				   LIBS_EXTFF="-ljpeg ${LIBS_EXTFF}"
				 ]))
  else
    HAVE_JPEG=yes
    INCL_EXTFF="${INCL_EXTFF} -I${JPEG_DIR}/include"
    LIBS_EXTFF="-L${JPEG_DIR}/lib -ljpeg ${LIBS_EXTFF}"
  fi

  AC_ARG_WITH(tiff, [  --with-tiff=DIR         Directory containing the tiff
                          lib and include directories.],
	      TIFF_DIR=$withval, TIFF_DIR=no)
  if test ${TIFF_DIR} = "no"
  then
    AC_CHECK_HEADER(tiff.h,
		    AC_CHECK_LIB(tiff, TIFFOpen,
				 [
				   HAVE_TIFF=yes
				   LIBS_EXTFF="-ltiff  ${LIBS_EXTFF} "
				 ]))
  else
    HAVE_TIFF=yes
    INCL_EXTFF="${INCL_EXTFF} -I${TIFF_DIR}/include"
    LIBS_EXTFF="-L${TIFF_DIR}/lib -ltiff ${LIBS_EXTFF}"
  fi

  AC_ARG_WITH(nifti, [  --with-nifti=DIR         Directory containing NIfTI
                          lib and include directories.],
	      NIFTI_DIR=$withval, NIFTI_DIR=no)
  if test ${NIFTI_DIR} = "no"
  then
    AC_CHECK_HEADER(nifti1_io.h,
		    AC_CHECK_LIB(nifti, nifti_simple_init_nim,
				 [
				   HAVE_NIFTI=yes
				   LIBS_EXTFF=" -lniftiio -lznz ${LIBS_EXTFF}"
				 ]))
  else
    HAVE_NIFTI=yes
    INCL_EXTFF="${INCL_EXTFF} -I${NIFTI_DIR}/include"
    LIBS_EXTFF="-L${NIFTI_DIR}/lib -lniftiio -lznz ${LIBS_EXTFF}"
  fi
  if test ${NIFTI_DIR} = "no"
  then
    CFLAGS="${CFLAGS} -DHAVE_NIFTI=0"
  else
    CFLAGS="${CFLAGS} -DHAVE_NIFTI=1"
  fi

  AC_CONFIG_FILES([	libbibfile/Makefile \
			libhguDlpList/Makefile \
			libReconstruct/Makefile \
  			libWlzExtFF/Makefile \
			binWlzApp/Makefile \
			binWlzExtFF/Makefile])
fi

if test x${ENABLE_JAVA} = "xyes"
then
  AC_ARG_WITH(jdk, [  --with-jdk=DIR          Directory with Java development
                          kit defaults to the value of the JAVA_HOME
			  environment variable.],
		     JDK_DIR=$withval, JDK_DIR=no)
  if test ${JDK_DIR} = "no"
  then
      # no --with-jdk argument given
      if test x${JAVA_HOME} = "x"
      then
	AC_MSG_ERROR([JDK base directory required, use --with-jdk=DIR.])
      else
        JDK_DIR=${JAVA_HOME}
      fi
  fi
  if test ${JDK_DIR} != "no"
  then
    JAR=${JDK_DIR}/bin/jar
    JAVA=${JDK_DIR}/bin/java
    JAVAC=${JDK_DIR}/bin/javac
  fi
  AC_ARG_WITH(javacc, [  --with-javacc=PATH      Full path of the javacc
                          executable, by default the standard paths are
			  searched.],
		     JAVACC=$withval, JAVACC=no)
  if test ${JAVACC} = "no"
  then
    AC_PATH_PROG(JAVACC, javacc)
    if test x${JAVACC} = "x"
    then
      AC_MSG_ERROR([Program javacc required, use --with-javacc=PATH.])
    fi
  fi
  AC_SUBST(JAVA)
  AC_SUBST(JAVAC)
  AC_SUBST(JAVACC)
  AC_CONFIG_FILES([     libWlzBnd/Makefile \
                        java/uk/ac/mrc/hgu/Wlz/C2Java/Makefile \
                        java/uk/ac/mrc/hgu/Wlz/C/Makefile \
			java/uk/ac/mrc/hgu/Wlz/Java/Makefile ])

  if test x${ENABLE_TEST} = "xyes"
  then
    AC_CONFIG_FILES([   java/uk/ac/mrc/hgu/Wlz/Test/Makefile ])
  fi
fi

case $host in
  *mingw*)
    LIBS="-lregex $LIBS"
    ;;
  *cygwin*)
    LIBS="-lregex $LIBS"
    ;;
esac

AC_OUTPUT

AC_MSG_RESULT([
Woolz core has been configured on ${host} with

  Source directory:                 ${srcdir}
  Installation directory:           ${prefix}
  Target CPU:                       ${target_cpu}
  C compiler:                       ${CC}
  C++ compiler:                     ${CXX}
  Linker:                           ${LD}
  Archiver:                         ${AR}
  CFLAGS:                         ${CFLAGS}
  LIBS needed for configure:       ${LIBS_EXTFF} ${LIBS}
  Woolz external file format code:  ${ENABLE_EXTFF}
  Java Woolz binding:               ${ENABLE_JAVA}
  JDK directory:                    ${JDK_DIR}
  jar:                              ${JAR}
  java:                             ${JAVA}
  javac:                            ${JAVAC}
  javacc:                           ${JAVACC}
  Test programs:                    ${ENABLE_TEST}

To build Woolz:                     make
To install Woolz:                   make install
To build Woolz documentation:       make doc
To install Woolz documentation:     make install-doc
])

