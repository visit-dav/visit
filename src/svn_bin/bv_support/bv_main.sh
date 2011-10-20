function initialize_build_visit()
{

# *************************************************************************** #
#                       Section 1, setting up inputs                          #
# --------------------------------------------------------------------------- #
# This section sets up the inputs to the VisIt script.  This is where you can #
# specify which compiler to use, which versions of the third party libraries, #
# etc.  Note that this script is really only known to work with gcc.          #
# *************************************************************************** #

# This env. variable is NOT to be overriden by user. It is intended to
# contain user's env. just prior to running build_visit.
export BUILD_VISIT_ENV=$(env | cut -d'=' -f1 | sort | uniq)

# Can cause problems in some build systems.
unset CDPATH

# Some systems tar command does not support the deep directory hierarchies
# used in Qt, such as AIX. Gnu tar is a good alternative.
### export TAR=/usr/local/bin/tar # Up and Purple
export TAR=tar

export OPSYS=${OPSYS:-$(uname -s)}
export PROC=${PROC:-$(uname -p)}
export REL=${REL:-$(uname -r)}
# Determine architecture
if [[ "$OPSYS" == "Darwin" ]]; then
   export ARCH=${ARCH:-"${PROC}-apple-darwin${REL%%.*}"}
#  export VISITARCH=${VISITARCH-${ARCH}}
   export SO_EXT="dylib"
   VER=$(uname -r)
# Check for Panther, because MACOSX_DEPLOYMENT_TARGET will default to 10.1
   if (( ${VER%%.*} < 8 )) ; then
      export MACOSX_DEPLOYMENT_TARGET=10.3
   elif [[ ${VER%%.*} == 8 ]] ; then
      export MACOSX_DEPLOYMENT_TARGET=10.4
   elif [[ ${VER%%.*} == 9 ]] ; then
      export MACOSX_DEPLOYMENT_TARGET=10.5
   elif [[ ${VER%%.*} == 10 ]] ; then
      export MACOSX_DEPLOYMENT_TARGET=10.6
   else
      export MACOSX_DEPLOYMENT_TARGET=10.6
   fi
   export C_COMPILER=${C_COMPILER:-"gcc"}
   export CXX_COMPILER=${CXX_COMPILER:-"g++"}
   # Disable Fortran on Darwin since it causes HDF5, H5Part, Silo, ADIOS builds to explode.
   export FC_COMPILER="" #${FC_COMPILER:-$(which gfortran | grep '^/')}
   export C_OPT_FLAGS=${C_OPT_FLAGS:-"-O2"}
   export CFLAGS=${CFLAGS:-"-fno-common -fexceptions"}
   export CXX_OPT_FLAGS=${CXX_OPT_FLAGS:-"-O2"}
   export CXXFLAGS=${CXXFLAGS:-"-fno-common -fexceptions"}
   export FCFLAGS=${FCFLAGS:-$CFLAGS}
   export MESA_TARGET=${MESA_TARGET:-"darwin"}
   export QT_PLATFORM=${QT_PLATFORM:-"macx-g++"}
elif [[ "$OPSYS" == "Linux" ]]; then
   export ARCH=${ARCH:-"linux-$(uname -m)"} # You can change this to say RHEL, SuSE, Fedora.
   export SO_EXT="so"
   if [[ "$(uname -m)" == "i386" ]] ; then
###   export MESA_TARGET=${MESA_TARGET:-"linux-x86"} # Mesa-6.x
      export MESA_TARGET=${MESA_TARGET:-"linux"}
   elif [[ "$(uname -m)" == "i686" ]] ; then
###   export MESA_TARGET=${MESA_TARGET:-"linux-x86"} # Mesa-6.x
      export MESA_TARGET=${MESA_TARGET:-"linux"}
   elif [[ "$(uname -m)" == "x86_64" ]] ; then
      CFLAGS="$CFLAGS -m64 -fPIC"
      FCFLAGS="$FCFLAGS -m64 -fPIC"
      if [[ "$C_COMPILER" == "gcc" || "$C_COMPILER" == "" ]]; then
          C_OPT_FLAGS="$C_OPT_FLAGS -O2"
      fi
      CXXFLAGS="$CXXFLAGS -m64 -fPIC"
      if [[ "$CXX_COMPILER" == "g++" || "$CXX_COMPILER" == "" ]]; then
          CXX_OPT_FLAGS="$CXX_OPT_FLAGS -O2"
      fi
###   export MESA_TARGET=${MESA_TARGET:-"linux-x86-64"} # Mesa-6.x
      export MESA_TARGET=${MESA_TARGET:-"linux"}
      QT_PLATFORM="linux-g++-64"
   elif [[ "$(uname -m)" == "ppc64" ]] ; then
      if [[ "$C_COMPILER" == "xlc" ]] ; then
          CFLAGS="$CFLAGS -qpic"
          FCFLAGS="$FCFLAGS -qpic"
          CXXFLAGS="$CXXFLAGS -qpic"
          export CXX_COMPILER=${CXX_COMPILER-"xlC"}
          export MESA_TARGET=${MESA_TARGET-"linux"}
          QT_PLATFORM="linux-xlc" #aix-xlc"
      else
          CFLAGS="$CFLAGS -fPIC"
          FCFLAGS="$FCFLAGS -fPIC"
          if [[ "$C_COMPILER" == "gcc" || "$C_COMPILER" == "" ]]; then
              C_OPT_FLAGS="$C_OPT_FLAGS -O2"
          fi
          CXXFLAGS="$CXXFLAGS -fPIC"
          if [[ "$CXX_COMPILER" == "g++" || "$CXX_COMPILER" == "" ]]; then
              CXX_OPT_FLAGS="$CXX_OPT_FLAGS -O2"
          fi
          export MESA_TARGET=${MESA_TARGET-"linux"}
          QT_PLATFORM="linux-g++"
      fi
   elif [[ "$(uname -m)" == "ia64" ]] ; then
      CFLAGS="$CFLAGS -fPIC"
      FCFLAGS="$FCFLAGS -fPIC"
      if [[ "$C_COMPILER" == "gcc" || "$C_COMPILER" == "" ]]; then
          C_OPT_FLAGS="$C_OPT_FLAGS -O2"
      fi
      CXXFLAGS="$CXXFLAGS -fPIC"
      if [[ "$CXX_COMPILER" == "g++" || "$CXX_COMPILER" == "" ]]; then
          CXX_OPT_FLAGS="$CXX_OPT_FLAGS -O2"
      fi
      QT_PLATFORM="linux-g++"
   fi
   export C_COMPILER=${C_COMPILER:-"gcc"}
   export CXX_COMPILER=${CXX_COMPILER:-"g++"}
   export FC_COMPILER=${FC_COMPILER:-$(which gfortran | grep '^/')}
   export C_OPT_FLAGS=${C_OPT_FLAGS:-"-O2"}
   export CXX_OPT_FLAGS=${CXX_OPT_FLAGS:-"-O2"}
   export MESA_TARGET=${MESA_TARGET:-"linux"}
   export QT_PLATFORM=${QT_PLATFORM:-"linux-g++"}
elif [[ "$OPSYS" == "AIX" ]]; then
   export ARCH="aix" # You can change this to say RHEL, SuSE, Fedora, etc.
   export SO_EXT="a"
   export C_COMPILER=${C_COMPILER:-"xlc"}
   export FC_COMPILER=${FC_COMPILER:-$(which xlf | grep '^/')}
   export CXX_COMPILER=${CXX_COMPILER:-"xlC"}
   export C_OPT_FLAGS=${C_OPT_FLAGS:-"-O2"}
   export CXX_OPT_FLAGS=${CXX_OPT_FLAGS:-"-O2"}
   export MAKE=${MAKE:-"gmake"}
   export MESA_TARGET=${MESA_TARGET:-"aix"}
   if [[ "$OBJECT_MODE" == 32 ]]; then
      export QT_PLATFORM=${QT_PLATFORM:-"aix-xlc"}
   else
      export QT_PLATFORM=${QT_PLATFORM:-"aix-xlc-64"}
   fi
elif [[ "$OPSYS" == "IRIX64" ]]; then
   export ARCH="irix64" # You can change this to say RHEL, SuSE, Fedora, etc.
   export SO_EXT="so"
   export C_COMPILER=${C_COMPILER:-"gcc"}
   export FC_COMPILER=${FC_COMPILER:-$(which gfortran | grep '^/')}
   export CXX_COMPILER=${CXX_COMPILER:-"g++"}
   export C_OPT_FLAGS=${C_OPT_FLAGS:-"-O2"}
   export CXX_OPT_FLAGS=${CXX_OPT_FLAGS:-"-O2"}
   export MAKE=${MAKE:-"gmake"}
   export MESA_TARGET=${MESA_TARGET:-"irix6-64-dso"}
elif [[ "$OPSYS" == "SunOS" ]]; then
   export ARCH=${ARCH:-"sunos5"}
   export SO_EXT="so"
   export C_COMPILER=${C_COMPILER:-"gcc"}
   export FC_COMPILER=${FC_COMPILER:-$(which gfortran | grep '^/')}
   export CXX_COMPILER=${CXX_COMPILER:-"g++"}
   export C_OPT_FLAGS=${C_OPT_FLAGS:-"-O2"}
   export CXX_OPT_FLAGS=${CXX_OPT_FLAGS:-"-O2"}
   export MAKE=${MAKE:-"make"}
   export MESA_TARGET=${MESA_TARGET:-"sunos5-gcc"}
   export QT_PLATFORM="solaris-g++"
else
   export ARCH=${ARCH:-"linux-$(uname -m)"} # You can change this to say RHEL, SuSE, Fedora.
   export SO_EXT="so"
   if [[ "$(uname -m)" == "x86_64" ]] ; then
      CFLAGS="$CFLAGS -m64 -fPIC"
      FCFLAGS="$FCFLAGS -m64 -fPIC"
      if [[ "$C_COMPILER" == "gcc" || "$C_COMPILER" == "" ]]; then
          C_OPT_FLAGS="$C_OPT_FLAGS -O2"
      fi
      CXXFLAGS="$CXXFLAGS -m64 -fPIC"
      if [[ "$CXX_COMPILER" == "g++" || "$CXX_COMPILER" == "" ]]; then
          CXX_OPT_FLAGS="$CXX_OPT_FLAGS -O2"
      fi
      QT_PLATFORM="linux-g++-64"
   fi
   if [[ "$(uname -m)" == "ia64" ]] ; then
      CFLAGS="$CFLAGS -fPIC"
      FCFLAGS="$FCFLAGS -fPIC"
      if [[ "$C_COMPILER" == "gcc" || "$C_COMPILER" == "" ]]; then
          C_OPT_FLAGS="$C_OPT_FLAGS -O2"
      fi
      CXXFLAGS="$CXXFLAGS -fPIC"
      if [[ "$CXX_COMPILER" == "g++" || "$CXX_COMPILER" == "" ]]; then
          CXX_OPT_FLAGS="$CXX_OPT_FLAGS -O2"
      fi
      QT_PLATFORM="linux-g++-64"
   fi
   export C_COMPILER=${C_COMPILER:-"gcc"}
   export FC_COMPILER=${FC_COMPILER:-$(which gfortran | grep '^/')}
   export CXX_COMPILER=${CXX_COMPILER:-"g++"}
   export C_OPT_FLAGS=${C_OPT_FLAGS:-"-O2"}
   export CXX_OPT_FLAGS=${CXX_OPT_FLAGS:-"-O2"}
   export QT_PLATFORM=${QT_PLATFORM:-"linux-g++"}
fi
export MAKE=${MAKE:-"make"}
export THIRD_PARTY_PATH=${THIRD_PARTY_PATH:-"./visit"}
export GROUP=${GROUP:-"visit"}
export LOG_FILE=${LOG_FILE:-"${0##*/}_log"}
export SVNREVISION=${SVNREVISION:-"HEAD"}
# Created a temporary value because the user can override most of 
# the components, which for the GUI happens at a later time.
# the tmp value is useful for user feedback.
if [[ $VISITARCH == "" ]] ; then
    export VISITARCHTMP=${ARCH}_${C_COMPILER}
    if [[ "$CXX_COMPILER" == "g++" ]] ; then
        VERSION=$(g++ -v 2>&1 | grep "gcc version" | cut -d' ' -f3 | cut -d'.' -f1-2)
        if [[ ${#VERSION} == 3 ]] ; then
            VISITARCHTMP=${VISITARCHTMP}-${VERSION}
        fi
    fi
else
# use environment variable value
    export VISITARCHTMP=$VISITARCH
fi

REDIRECT_ACTIVE="no"
ANY_ERRORS="no"
#
# OPTIONS
#
#initialize required libraries..
for (( i=0; i<${#reqlibs[*]}; ++i ))
do
    initializeFunc="bv_${reqlibs[$i]}_initialize"
    $initializeFunc
done

for (( i=0; i<${#optlibs[*]}; ++i ))
do
    initializeFunc="bv_${optlibs[$i]}_initialize"
    $initializeFunc
done

export DO_HOSTCONF="yes"
export ON_HOSTCONF="on"

export ON_ALLIO="off"

export DO_DEBUG="no"
export ON_DEBUG="off"
export ABS_PATH="yes"
export ON_ABS_PATH="on"
export DO_VISIT="yes"
export ON_VISIT="on"
export DO_REQUIRED_THIRD_PARTY="yes"
export ON_THIRD_PARTY="on"
export DO_GROUP="no"
export ON_GROUP="off"
export DO_LOG="no"
export ON_LOG="off"
parallel="no"
ON_parallel="off"
export DO_SVN="no"
export DO_SVN_ANON="no"
export ON_SVN="off"
export DO_REVISION="no"
export ON_REVISION="off"
USE_VISIT_FILE="no"
ON_USE_VISIT_FILE="off"
export DO_PATH="no"
export ON_PATH="off"
export DO_VERSION="no"
export ON_VERSION="off"
export DO_MODULE="no"
export ON_MODULE="off"
export DO_VERBOSE="no"
export ON_VERBOSE="off"
export DO_JAVA="no"
export ON_JAVA="off"
export DO_SLIVR="no"
export ON_SLIVR="off"
export PREVENT_ICET="no"
GRAPHICAL="yes"
ON_GRAPHICAL="on"
verify="no"
ON_verify="off"
export DO_OPTIONAL="yes"
export ON_OPTIONAL="on"
export DO_OPTIONAL2="no"
export ON_OPTIONAL2="off"
export DO_MORE="no"
export ON_MORE="off"
export DO_DBIO_ONLY="no"
export DO_STATIC_BUILD="no"
export USE_VISIBILITY_HIDDEN="no"
DOWNLOAD_ONLY="no"

if [[ "$CXX_COMPILER" == "g++" ]] ; then
    VERSION=$(g++ -v 2>&1 | grep "gcc version" | cut -d' ' -f3 | cut -d'.' -f1-1)
    if [[ ${VERSION} -ge 4 ]] ; then
        export USE_VISIBILITY_HIDDEN="yes"
    fi
fi


export SVN_ANON_ROOT_PATH="http://portal.nersc.gov/svn/visit"
# Setup svn path: use SVN_NERSC_NAME if set
if test -z "$SVN_NERSC_NAME" ; then
    export SVN_REPO_ROOT_PATH="svn+ssh://portal-auth.nersc.gov/project/projectdirs/visit/svn/visit"
else
    export SVN_REPO_ROOT_PATH="svn+ssh://$SVN_NERSC_NAME@portal-auth.nersc.gov/project/projectdirs/visit/svn/visit"
fi

############################################################################
# Path of VisIt src directory in svn.
# For trunk use "trunk/src"
# For versioned build_visit releases change this to point to the proper
# release tag.
############################################################################

export VISIT_VERSION=${VISIT_VERSION:-"2.3.2"}

####
# Trunk:
####
export SVN_SOURCE_PATH="trunk/src"

###
# Release:
###
#export SVN_SOURCE_PATH="tags/${VISIT_VERSION}/src"

if [[ "$OPSYS" != "Darwin" ]]; then
    WGET_MINOR_VERSION=$(wget --version| head -n 1|cut -d. -f 2)
    # version 1.7 pre-dates ssl integration
    if [[ "${WGET_MINOR_VERSION}" == "8" ]] ; then
       export WGET_OPTS=${WGET_OPTS=""}
    elif [[ "${WGET_MINOR_VERSION}" == "9" ]] ; then
       export WGET_OPTS=${WGET_OPTS:="--sslcheckcert=0"}
    else
       export WGET_OPTS=${WGET_OPTS:-"--no-check-certificate"}
    fi
fi

#
# TARBALL LOCATIONS AND VERSIONS
#
if [[ "$VISIT_FILE" != "" ]] ; then
  USE_VISIT_FILE="yes"
  ON_USE_VISIT_FILE="on"
fi
export VISIT_FILE=${VISIT_FILE:-"visit${VISIT_VERSION}.tar.gz"}

for (( i=0; i<${#reqlibs[*]}; ++i ))
do
    initializeFunc="bv_${reqlibs[$i]}_info"
    $initializeFunc
done

for (( i=0; i<${#optlibs[*]}; ++i ))
do
    initializeFunc="bv_${optlibs[$i]}_info"
    $initializeFunc
done


# Dialog-related variables.
DLG="dialog"
DLG_BACKTITLE="VisIt $VISIT_VERSION Build Process"
DLG_HEIGHT="5"
DLG_HEIGHT_TALL="25"
DLG_WIDTH="60"
DLG_WIDTH_WIDE="80"
WRITE_UNIFIED_FILE=""

}




# *************************************************************************** #
# Function: starts_with_quote                                                 #
#                                                                             #
# Purpose: Meant to be used in `if $(starts_with_quote "$var") ; then`        #
#          conditionals.                                                      #
#                                                                             #
# Programmer: Tom Fogal                                                       #
# Date: Thu Oct  9 15:24:04 MDT 2008                                          #
#                                                                             #
# *************************************************************************** #

function starts_with_quote
{
    if test "${1:0:1}" = "\""; then
        return 0
    fi
    if test "${1:0:1}" = "'" ; then
        return 0
    fi
    return 1
}

# *************************************************************************** #
# Function: ends_with_quote                                                   #
#                                                                             #
# Purpose: Meant to be used `if $(ends_with_quote "$var") ; then`             #
#          conditionals.                                                      #
#                                                                             #
# Programmer: Tom Fogal                                                       #
# Date: Thu Oct  9 15:24:13 MDT 2008                                          #
#                                                                             #
# *************************************************************************** #

function ends_with_quote
{
    if test "${1: -1:1}" = "\""; then
        return 0
    fi
    if test "${1: -1:1}" = "'"; then
        return 0
    fi
    return 1
}

# *************************************************************************** #
# Function: strip_quotes                                                      #
#                                                                             #
# Purpose: Removes all quotes from the given argument.  Meant to be used in   #
#          $(strip_quotes "$some_string") expressions.                        #
#                                                                             #
# Programmer: Tom Fogal                                                       #
# Date: Thu Oct  9 16:04:25 MDT 2008                                          #
#                                                                             #
# *************************************************************************** #

function strip_quotes
{
    local arg="$@"
    str=""
    while test -n "$arg" ; do
        if test "${arg:0:1}" != "\"" ; then
            str="${str}${arg:0:1}"
        fi
        arg="${arg:1}"
    done
    echo "${str}"
}


# *************************************************************************** #
#                          Function 9.1, build_visit                          #
# *************************************************************************** #

function build_visit
{
    if [[ "$DO_SVN" != "yes" || "$USE_VISIT_FILE" == "yes" ]] ; then
        #
        # Unzip the file, provided a gzipped file exists.
        #
        if [[ -f ${VISIT_FILE} ]] ; then
           info "Unzipping/untarring ${VISIT_FILE} . . ."
           uncompress_untar ${VISIT_FILE}
           if [[ $? != 0 ]] ; then
              warn \
"Unable to untar ${VISIT_FILE}.  Corrupted file or out of space on device?"
              return 1
           fi
        elif [[ -f ${VISIT_FILE%.*} ]] ; then
           info "Unzipping ${VISIT_FILE%.*} . . ."
           $TAR xf ${VISIT_FILE%.*}
           if [[ $? != 0 ]] ; then
              warn  \
"Unable to untar ${VISIT_FILE%.*}.  Corrupted file or out of space on device?"
              return 1
           fi
        fi
    fi

    #
    # Set up environment variables for the configure step.
    #
    PARFLAGS=""
    if [[ "$parallel" == "yes" ]] ; then
       PARFLAGS="--enable-parallel"
       CXXFLAGS="$CXXFLAGS $PAR_INCLUDE"
    fi

    #
    # Set up the config-site file, which gives configure the information it
    # needs about the third party libraries.
    #
    if [[ "$DO_SVN" == "yes" && "$USE_VISIT_FILE" == "no" ]] ; then
        cd src
    else
        VISIT_DIR=${VISIT_FILE%.tar*}/src
        cd "${VISIT_DIR}"
    fi
    cp $START_DIR/$(hostname).cmake config-site

    #
    # Call configure
    # 
    info "Configuring VisIt . . ."
    EXTRA_FEATURES=""
    if [[ "${DO_MODULE}" == "yes" ]] ; then
       EXTRA_FEATURES="${EXTRA_FEATURES} --enable-visitmodule"
    fi
    if [[ "${DO_JAVA}" == "yes" ]] ; then
       EXTRA_FEATURES="${EXTRA_FEATURES} --enable-java"
    fi
    if [[ "${DO_SLIVR}" == "yes" ]] ; then
       EXTRA_FEATURES="${EXTRA_FEATURES} --enable-slivr"
    fi
    # A dbio-only build disables pretty much everything else.
    if [[ "${DO_DBIO_ONLY}" == "yes" ]] ; then
       EXTRA_FEATURES="--enable-dbio-only"
    else
       EXTRA_FEATURES="${EXTRA_FEATURES} --enable-install-thirdparty"
    fi
    ./configure ${PARFLAGS} ${EXTRA_FEATURES}
    if [[ $? != 0 ]] ; then
       echo "VisIt configure failed.  Giving up"
       return 1
    fi

    if [[ "$OPSYS" == "Darwin" ]]; then
        # Check for version < 8.0.0 (MacOS 10.4, Tiger) for gcc < 4.x
        VER=$(uname -r)
        if (( ${VER%%.*} > 8 )) ; then
           cat databases/Shapefile/Makefile | \
              sed '/LDFLAGS/s/$/ -Wl,-dylib_file,\/System\/Library\/Frameworks\/OpenGL.framework\/Versions\/A\/Libraries\/libGLU.dylib:\/System\/Library\/Frameworks\/OpenGL.framework\/Versions\/A\/Libraries\/libGLU.dylib/' > Make.tmp
           mv -f databases/Shapefile/Makefile databases/Shapefile/Makefile.orig
           mv -f Make.tmp databases/Shapefile/Makefile
           if [[ "$DO_CCMIO" == "yes" ]] ; then
              cat databases/CCM/Makefile | \
                 sed '/LDFLAGS/s/$/ -Wl,-dylib_file,\/System\/Library\/Frameworks\/OpenGL.framework\/Versions\/A\/Libraries\/libGLU.dylib:\/System\/Library\/Frameworks\/OpenGL.framework\/Versions\/A\/Libraries\/libGLU.dylib/' > Make.tmp
              mv -f databases/CCM/Makefile databases/CCM/Makefile.orig
              mv -f Make.tmp databases/CCM/Makefile
           fi
        fi 
        if (( ${VER%%.*} < 8 )) ; then
           info "Patching VisIt . . ."
           cat databases/Fluent/Makefile | sed '/CXXFLAGS/s/$/ -O0/g' > Make.tmp
           mv -f databases/Fluent/Makefile databases/Fluent/Makefile.orig
           mv -f Make.tmp databases/Fluent/Makefile
           cat avt/Pipeline/Data/avtCommonDataFunctions.C | \
              sed '/isfinite/s/isfinite/__isfinited/g' > C.tmp
           mv -f avt/Pipeline/Data/avtCommonDataFunctions.C \
              avt/Pipeline/Data/avtCommonDataFunctions.C.orig
           mv -f C.tmp avt/Pipeline/Data/avtCommonDataFunctions.C
           cat avt/Expressions/Abstract/avtExpressionFilter.C | \
              sed '/isfinite/s/isfinite/__isfinited/g' > C.tmp
           mv -f avt/Expressions/Abstract/avtExpressionFilter.C \
              avt/Expressions/Abstract/avtExpressionFilter.C.orig
           mv -f C.tmp avt/Expressions/Abstract/avtExpressionFilter.C
        fi
        if (( ${VER%%.*} < 7 )) ; then
           cat third_party_builtin/mesa_stub/Makefile | \
              sed 's/glx.c glxext.c//' > Make.tmp
           mv -f third_party_builtin/mesa_stub/Makefile \
              third_party_builtin/mesa_stub/Makefile.orig
           mv -f Make.tmp third_party_builtin/mesa_stub/Makefile
        fi
        if (( ${VER%%.*} > 6 )) ; then
           cat databases/SimV1/Makefile | \
              sed '/LDFLAGS/s/$/ -Wl,-undefined,dynamic_lookup/g' > Make.tmp
           mv -f databases/SimV1/Makefile databases/SimV1/Makefile.orig
           mv -f Make.tmp databases/SimV1/Makefile
           cat databases/SimV1Writer/Makefile | \
              sed '/LDFLAGS/s/$/ -Wl,-undefined,dynamic_lookup/g' > Make.tmp
           mv -f databases/SimV1Writer/Makefile \
             databases/SimV1Writer/Makefile.orig
           mv -f Make.tmp databases/SimV1Writer/Makefile
           cat avt/Expressions/Makefile | \
              sed '/LDFLAGS/s/$/ -Wl,-undefined,dynamic_lookup/g' > Make.tmp
           mv -f avt/Expressions/Makefile \
             avt/Expressions/Makefile.orig
           mv -f Make.tmp avt/Expressions/Makefile
        else
           cat databases/SimV1/Makefile | \
              sed '/LDFLAGS/s/$/ -Wl,-flat_namespace,-undefined,suppress/g' > \
              Make.tmp
           mv -f databases/SimV1/Makefile databases/SimV1/Makefile.orig
           mv -f Make.tmp databases/SimV1/Makefile
           cat databases/SimV1Writer/Makefile | \
              sed '/LDFLAGS/s/$/ -Wl,-flat_namespace,-undefined,suppress/g' > \
              Make.tmp
           mv -f databases/SimV1Writer/Makefile \
             databases/SimV1Writer/Makefile.orig
           mv -f Make.tmp databases/SimV1Writer/Makefile
           cat avt/Expressions/Makefile | \
              sed '/LDFLAGS/s/$/ -Wl,-flat_namespace,-undefined,suppress/g' > \
              Make.tmp
           mv -f avt/Expressions/Makefile \
             avt/Expressions/Makefile.orig
           mv -f Make.tmp avt/Expressions/Makefile
        fi
    elif [[ "$OPSYS" == "SunOS" ]]; then
        # Some Solaris systems hang when compiling Fluent when optimizations
        # are on.  Turn optimizations off.
        info "Patching VisIt . . ."
        cat databases/Fluent/Makefile | sed '/CXXFLAGS/s/$/ -O0/g' > Make.tmp
        mv -f databases/Fluent/Makefile databases/Fluent/Makefile.orig
        mv -f Make.tmp databases/Fluent/Makefile
    fi

    #
    # Build VisIt
    #
    info "Building VisIt . . . (~50 minutes)"
    $MAKE $MAKE_OPT_FLAGS
    if [[ $? != 0 ]] ; then
       warn "VisIt build failed.  Giving up"
       return 1
    fi
    warn "All indications are that VisIt successfully built."

    #
    # Major hack here. Mark M. should really pull this total hack out of
    # this script. It is here to make the visitconvert tool be called
    # imeshio to satisfy needs of ITAPS SciDAC project.
    #
    if [[ "${DO_DBIO_ONLY}" == "yes" && "$0" == "build_imeshio" ]] ; then
        if [[ -e exe/visitconvert_ser_lite ]]; then
            cp exe/visitconvert_ser_lite exe/imeshioconvert
            cp bin/visitconvert bin/imeshioconvert
        fi
    fi
}


# *************************************************************************** #
#                       Section 2, building VisIt                             #
# --------------------------------------------------------------------------- #
# This section does some set up for building VisIt, and then calls the        #
# functions to build the third party libraries and VisIt itself.              #
# *************************************************************************** #
function run_build_visit()
{
# Fix the arguments: cram quoted strings into a single argument.
declare -a arguments
quoting="" # temp buffer for concatenating quoted args
state=0    # 0 is the default state, for grabbing std arguments.
           # 1 is for when we've seen a quote, and are currently "cramming"
for arg in $@ ; do
    case $state in
        0)
            if $(starts_with_quote "$arg") ; then
                state=1
                quoting="${arg}"
            else
                state=0
                tval="${arg}"
                arguments[${#arguments[@]}]=$tval
            fi
            ;;
        1)
            if $(starts_with_quote "$arg") ; then
                state=0
                arguments="${quoting}"
            elif $(ends_with_quote "$arg") ; then
                quoting="${quoting} ${arg}"
                tval="${quoting}"
                arguments[${#arguments[@]}]=$tval
                state=0
            else
                quoting="${quoting} ${arg}"
            fi
            ;;
        *) error "invalid state.";;
    esac
done

# Will be set if the next argument is an argument to an argument (I swear that
# makes sense).  Make sure to unset it after pulling the argument!
next_arg=""
# If the user gives any deprecated options, we'll append command line options
# we think they should use here.
deprecated=""
# A few options require us to perform some action before we start building
# things, but we'd like to finish option parsing first.  We'll set this
# variable in those cases, and test it when we finish parsing.
next_action=""

#handle dbio and allio first since they affect multiple libraries..
for arg in "${arguments[@]}" ; do
    case $arg in
        --all-io) DO_ALLIO="yes";;
        --dbio-only) DO_DBIO_ONLY="yes";;
    esac
done

#
# As a temporary convenience, for a dbio-only build, we turn on almost all
# of the 3rd party I/O libs used by database plugins.
#
if [[ "$DO_DBIO_ONLY" == "yes" || "$DO_ALLIO" == "yes" ]]; then
    for (( i=0; i<${#iolibs[*]}; ++i ))
    do
        initializeFunc="bv_${iolibs[$i]}_enable"
        $initializeFunc
    done
fi

    
for arg in "${arguments[@]}" ; do
    # Was the last option something that took an argument?
    if test -n "$next_arg" ; then
        # Yep.  Which option was it?
        case $next_arg in
            write-unified-file) WRITE_UNIFIED_FILE="$arg";;
            append-cflags) C_OPT_FLAGS="${C_OPT_FLAGS} ${arg}";;
            append-cxxflags) CXX_OPT_FLAGS="${CXX_OPT_FLAGS} ${arg}";;
            arch) VISITARCH="${arg}";;
            cflags) C_OPT_FLAGS=$(strip_quotes "${arg}");;
            cxxflags) CXX_OPT_FLAGS=$(strip_quotes "${arg}");;
            cc) C_COMPILER="${arg}";;
            cxx) CXX_COMPILER="${arg}";;
            flags-debug) C_OPT_FLAGS="${C_OPT_FLAGS} -g"
                         CXX_OPT_FLAGS="${CXX_OPT_FLAGS} -g";;
            makeflags) MAKE_OPT_FLAGS="${arg}";;
            group) GROUP="${arg}";;
            svn) SVNREVISION="${arg}";;
            tarball) VISIT_FILE="${arg}";;
            thirdparty-path) THIRD_PARTY_PATH="${arg}";;
            version) VISIT_VERSION="${arg}"
                     VISIT_FILE="visit${VISIT_VERSION}.tar.gz";;
            *) error "Unknown next_arg value '$next_arg'!"
        esac
        # Make sure we process the next option as an option and not an
        # argument to an option.
        next_arg=""
        continue
    fi

    if [[ ${#arg} -gt 2 ]] ; then #has --
       resolve_arg=${arg:2} #remove --
       declare -F "bv_${resolve_arg}_enable" &>/dev/null
       if [[ $? == 0 ]] ; then 
           echo "enabling ${resolve_arg}"
           initializeFunc="bv_${resolve_arg}_enable"
           $initializeFunc
           continue
       elif [[ ${#resolve_arg} -gt 3 ]] ; then #in case it is --no-
           resolve_arg_no_opt=${resolve_arg:3}
           #disable library if it does not exist..
           declare -F "bv_${resolve_arg_no_opt}_disable" &>/dev/null
           if [[ $? == 0 ]] ; then
               echo "disabling ${resolve_arg_no_opt}"
               initializeFunc="bv_${resolve_arg_no_opt}_disable"
               $initializeFunc
               continue
           fi
       fi
    fi
             
    case $arg in
        --write-unified-file) next_arg="write-unified-file";;
        --all-io) continue;; #do nothing now..
        --dbio-only) continue;; #do nothing now..
        --absolute) ABS_PATH="yes"; ON_ABS_PATH="on";;
        --arch) next_arg="arch";;
        --cflag) next_arg="append-cflags";;
        --cflags) next_arg="cflags";;
        --cxxflag) next_arg="append-cxxflags";;
        --cxxflags) next_arg="cxxflags";;
        --cc) next_arg="cc";;
        --cxx) next_arg="cxx";;
        --console) GRAPHICAL="no"; ON_GRAPHICAL="off";;
        --debug) set -vx;;
        --download-only) DOWNLOAD_ONLY="yes";;
        --flags-debug) next_arg="flags-debug";;
        --gdal) DO_GDAL="yes"; ON_GDAL="on";;
        --group) next_arg="group"; DO_GROUP="yes"; ON_GROUP="on";;
        -h|--help) next_action="help";;
        --java) DO_JAVA="yes"; ON_JAVA="on";;
        --makeflags) next_arg="makeflags";;
        --no-visit) DO_VISIT="no"; ON_VISIT="off";;
        --no-thirdparty) DO_REQUIRED_THIRD_PARTY="no"; ON_THIRD_PARTY="off";;
        --no-hostconf) DO_HOSTCONF="no"; ON_HOSTCONF="off";;
        --no-icet) PREVENT_ICET="yes";;
        --parallel) parallel="yes"; DO_ICET="yes"; ON_parallel="ON";;
        --print-vars) next_action="print-vars";;
        --python-module) DO_MODULE="yes"; ON_MODULE="on";;
        --relative) ABS_PATH="no"; ON_ABS_PATH="off";;
        --slivr) DO_SLIVR="yes"; ON_SLIVR="on";;
        --static) DO_STATIC_BUILD="yes";;
        --stdout) LOG_FILE="/dev/tty";;
        --svn) DO_SVN="yes"; export SVN_ROOT_PATH=$SVN_REPO_ROOT_PATH;;
        --svn-anon) DO_SVN="yes"; DO_SVN_ANON="yes" ; export SVN_ROOT_PATH=$SVN_ANON_ROOT_PATH ;;
        --svn-anonymous) DO_SVN="yes"; DO_SVN_ANON="yes" ; export SVN_ROOT_PATH=$SVN_ANON_ROOT_PATH ;;
        --svn-revision) next_arg="svn"; DO_SVN="yes"; DO_REVISION="yes"; DO_SVN_ANON="yes" ; export SVN_ROOT_PATH=$SVN_ANON_ROOT_PATH ;;
        --tarball) next_arg="tarball"
                   USE_VISIT_FILE="yes"
                   ON_USE_VISIT_FILE="on";;
        --thirdparty-path) next_arg="thirdparty-path"
                           ON_THIRD_PARTY_PATH="on";;
        --version) next_arg="version";;
        -4) deprecated="${deprecated} --hdf4";;
        -5) deprecated="${deprecated} --hdf5";;
        -c) deprecated="${deprecated} --cgns";;
        -C) deprecated="${deprecated} --ccmio";;
        -d) deprecated="${deprecated} --cflags '$C_OPT_FLAGS -g'";;
        -D) deprecated="${deprecated} --cflags '$C_OPT_FLAGS -g#'";;
        -E) deprecated="${deprecated} --print-vars";;
        -e) deprecated="${deprecated} --exodus";;
        -H) deprecated="${deprecated} --help";;
        -i) deprecated="${deprecated} --absolute";;
        -J) deprecated="${deprecated} --makeflags '-j <something>'";;
        -j) deprecated="${deprecated} --no-visit";;
        -m) deprecated="${deprecated} --mili";;
        -M) deprecated="${deprecated} --tcmalloc";;
        -r) deprecated="${deprecated} --h5part";;
        -R) deprecated="${deprecated} --svn <REVISION>";;
        -s) deprecated="${deprecated} --svn HEAD";;
        -S) deprecated="${deprecated} --slivr";;
        -t) deprecated="${deprecated} --tarball '<file>'";;
        -v) deprecated="${deprecated} --tarball 'visit<version>.tar.gz'";;
        -V) deprecated="${deprecated} --visus";;
        -b|-B) deprecated="${deprecated} --boxlib";;
        -f|-F) deprecated="${deprecated} --cfitsio";;
        -g|-G) deprecated="${deprecated} --gdal";;
        -k|-K) deprecated="${deprecated} --no-thirdparty";;
        -l|-L) deprecated="${deprecated} --group '<arg>'";;
        -n|-N) deprecated="${deprecated} --netcdf";;
        -o|-O) deprecated="${deprecated} --stdout";;
        -p|-P) deprecated="${deprecated} --parallel";;
        -u|-U) deprecated="${deprecated} --thirdparty-path <path>";;
        -w|-W) deprecated="${deprecated} --python";;
        -y|-Y) deprecated="${deprecated} --java";;
        -z|-Z) deprecated="${deprecated} --console";;
        *)
            echo "Unrecognized option '${arg}'."
            ANY_ERRORS="yes";;
    esac
done

if [[ $next_arg != "" ]] ; then
    echo "command line arguement are used incorrectly: argument $next_arg not fullfilled"
    error ""
fi

if test -n "${deprecated}" ; then
    summary="You are using some deprecated options to $0.  Please re-run"
    summary="${summary} $0 with a command line similar to:"
    echo "$summary"
    echo ""
    echo "$0 ${deprecated}"
    error ""
fi
if test -n "${next_action}" ; then
    case ${next_action} in
        print-vars) printvariables; exit 2;;
        help) usage; exit 2;;
    esac
fi

if [[ $WRITE_UNIFIED_FILE != "" ]] ; then
   bv_write_unified_file $WRITE_UNIFIED_FILE
   exit 2
fi

# make all VisIt related builds in its own directory..
mkdir -p visit_build_files
cd visit_build_files


#
# If we are AIX, make sure we are using GNU tar.
#
if [[ "$OPSYS" == "AIX" ]]; then
    TARVERSION=$($TAR --version >/dev/null 2>&1)
    if [[ $? != 0 ]] ; then
        echo "Error in build process. You are using the system tar on AIX."
        echo "Change the TAR variable in the script to the location of the"
        echo "GNU tar command."
        exit 1
    fi
fi

# Disable fortran support if a fortran compiler was not specified or found.
if [[ $FC_COMPILER == "" ]]; then
    export FC_COMPILER="no";
    warn "FC_COMPILER not set: Fortran support for thirdparty libraries disabled."
fi

# Show a splashscreen. This routine also determines if we have "dialog"
# or "whiptail", which we use to show dialogs. If we do not have either
# then proceed in non-graphical mode.
#

if [[ "$GRAPHICAL" == "yes" ]] ; then
    DLG=$(which dialog)
    
    # Guard against bad "which" implementations that return
    # "no <exe> in path1 path2 ..." (these implementations also
    # return 0 as the exit status).
    if [[ "${DLG#no }" != "${DLG}" ]] ; then
        DLG=""
    fi
    if [[ "$DLG" == "" ]] ; then
        warn "Could not find 'dialog'; looking for 'whiptail'."
        DLG=$(which whiptail)
        if [[ "${DLG#no }" != "${DLG}" ]] ; then
           DLG=""
        fi
        if [[ "$DLG" == "" ]] ; then
            GRAPHICAL="no"
            warn "'whiptail' not found."
            warn ""
            warn "Unable to find utility for graphical build..."
            warn "Continuing in command line mode..."
            warn ""
            test -t 1
            if test $? != 1 ; then
              warn "Please hit enter to continue."
              (read junkvar)
            fi
        else
            warn "Some versions of 'whiptail' fail to display anything with --infobox."
            warn "Continuing in command line mode..."
        fi
    fi
fi
if [[ "$GRAPHICAL" == "yes" ]] ; then
    $DLG --backtitle "$DLG_BACKTITLE" \
    --title "Build options" \
    --checklist \
"Welcome to the VisIt $VISIT_VERSION build process.\n\n"\
"This program will build VisIt and its required "\
"3rd party sources, downloading any missing source packages "\
"before building. The required and optional 3rd party libraries "\
"are built and installed before VisIt is built, so please be patient. "\
"Note that you can build a parallel version of VisIt by "\
"specifying the location of your MPI installation when prompted.\n\n"\
"Select the build options:" 0 0 0 \
           "Optional"    "select optional 3rd party libraries"  $ON_OPTIONAL\
           "AdvDownload" "use libs that cannot be downloaded from web"  $ON_OPTIONAL2\
           "SVN"        "get sources from SVN server"     $ON_SVN\
           "Tarball"    "specify VisIt tarball name"      $ON_USE_VISIT_FILE\
           "Parallel"   "specify parallel build flags"    $ON_parallel\
           "Python"     "enable VisIt python module"      $ON_MODULE\
           "Java"       "enable java client library"      $ON_JAVA\
           "SLIVR"      "enable SLIVR volume rendering library"  $ON_SLIVR\
           "EnvVars"     "specify build environment var values"   $ON_verify\
           "Advanced"   "display advanced options"        $ON_MORE  2> tmp$$
    retval=$?

    # Remove the extra quoting, new dialog has --single-quoted
    choice="$(cat tmp$$ | sed 's/\"//g' )"
    case $retval in
      0)
        DO_OPTIONAL="no"
        DO_OPTIONAL2="no"
        DO_SVN="no"
        USE_VISIT_FILE="no"
        parallel="no"
        DO_MODULE="no"
        DO_JAVA="no"
        DO_SLIVR="no"
        verify="no"
        DO_MORE="no"
        for OPTION in $choice
        do
            case $OPTION in
              Optional)
                 DO_OPTIONAL="yes";;
              AdvDownload)
                 DO_OPTIONAL2="yes";;
              SVN)
                 DO_SVN="yes";DO_SVN_ANON="yes";export SVN_ROOT_PATH=$SVN_ANON_ROOT_PATH ;;
              Tarball)
                 $DLG --backtitle "$DLG_BACKTITLE" \
                    --no-cancel --inputbox \
"Enter $OPTION value:" 0 $DLG_WIDTH_WIDE "$VISIT_FILE" 2> tmp$$
                 VISIT_FILE="$(cat tmp$$)"
                 USE_VISIT_FILE="yes";;
              Parallel)
                 parallel="yes"; DO_ICET="yes";;
              PythonModule)
                 DO_MODULE="yes";;
              Java)
                 DO_JAVA="yes";;
              SLIVR)
                 DO_SLIVR="yes";;
              EnvVars)
                 verify="yes";;
              Advanced)
                 DO_MORE="yes";;
            esac
        done
        ;;
      1)
        warn "Cancel pressed."
        exit 1;;
      255)
        warn "ESC pressed.";;
      *)
        warn "Unexpected return code: $retval";;
    esac
fi
if [[ -e "tmp$$" ]] ; then
    rm tmp$$
fi

if [[ "$DO_OPTIONAL" == "yes" && "$GRAPHICAL" == "yes" ]] ; then
    add_checklist_vars=""
    for (( i=0; i < ${#iolibs[*]}; ++i ))
    do
        initializeFunc="bv_${iolibs[$i]}_graphical"
        output_str="$($initializeFunc)"
        add_checklist_vars=${add_checklist_vars}" "${output_str}
    done


    $DLG --backtitle "$DLG_BACKTITLE" --title "Select 3rd party libraries" --checklist "Select the optional 3rd party libraries to be built and installed:" 0 0 0 $add_checklist_vars 2> tmp$$
    retval=$?

    # Remove the extra quoting, new dialog has --single-quoted
    choice="$(cat tmp$$ | sed 's/\"//g' )"
    case $retval in
      0)
        #disable all..
        for (( i=0; i < ${#iolibs[*]}; ++i ))
        do
          initializeFunc="bv_${iolibs[$i]}_disable"
          $initializeFunc
        done
        
    for OPTION in $choice
        do
            #lower case
            lower_option=`echo $OPTION | tr '[A-Z]' '[a-z]'`        
        declare -F "bv_${lower_option}_enable" &>/dev/null
            if [[ $? == 0 ]] ; then 
                initializeFunc="bv_${lower_option}_enable"
                $initializeFunc
            fi    
        done
        ;;
      1)
        warn "Cancel pressed."
        exit 1;;
      255)
        warn "ESC pressed.";;
      *)
        warn "Unexpected return code: $retval";;
    esac
fi
if [[ -e "tmp$$" ]] ; then
    rm tmp$$
fi

if [[ "$DO_OPTIONAL2" == "yes" && "$GRAPHICAL" == "yes" ]] ; then
    add_checklist_vars=""
    for (( i=0; i < ${#advancedlibs[*]}; ++i ))
    do
        initializeFunc="bv_${advancedlibs[$i]}_graphical"
        output_str="$($initializeFunc)"
        add_checklist_vars=${add_checklist_vars}" "${output_str}
    done
    $DLG --backtitle "$DLG_BACKTITLE" \
    --title "Select 3rd party libraries" \
    --checklist \
"Third party libs build_visit can't download "\
"(you must download these prior to running build_visit):" 0 0 0  \
           "AdvIO"    "$ADVIO_VERSION    $ADVIO_FILE"      $ON_ADVIO \
           "MDSplus"  "$MDSPLUS_VERSION  $MDSPLUS_FILE"    $ON_MDSPLUS \
           "Mili"     "$MILI_VERSION $MILI_FILE"      $ON_MILI \
           "TCMALLOC" "$TCMALLOC_VERSION   $TCMALLOC_FILE"      $ON_TCMALLOC \
            2> tmp$$
    retval=$?
    # Remove the extra quoting, new dialog has --single-quoted
    choice="$(cat tmp$$ | sed 's/\"//g' )"
    case $retval in
      0)
        DO_ADVIO="no"
        DO_MDSPLUS="no"
        DO_MILI="no"
        DO_TCMALLOC="no"
        for OPTION in $choice
        do
            case $OPTION in
              AdvIO)
                 DO_ADVIO="yes";;
              MDSplus)
                 DO_MDSPLUS="yes";;
              Mili)
                 DO_MILI="yes";;
              TCMALLOC)
                 DO_TCMALLOC="yes";;
            esac
        done
        ;;
      1)
        warn "Cancel pressed."
        exit 1;;
      255)
        warn "ESC pressed.";;
      *)
        warn "Unexpected return code: $retval";;
    esac
fi
if [[ -e "tmp$$" ]] ; then
    rm tmp$$
fi

#
# See if the used needs to modify some variables
#
check_more_options
if [[ $? != 0 ]] ; then
   error "Stopping build because of bad variable option setting error."
fi

#
# See if the user wants to build a parallel version.
#
check_parallel
if [[ $? != 0 ]] ; then
   error "Stopping build because necessary parallel options are not set."
fi

#
# See if the user wants to modify variables
#
check_variables
if [[ $? != 0 ]] ; then
   error "Stopping build because of bad variable option setting error."
fi


if [[ $VISITARCH == "" ]] ; then
    export VISITARCH=${ARCH}_${C_COMPILER}
    if [[ "$CXX_COMPILER" == "g++" ]] ; then
       VERSION=$(g++ -v 2>&1 | grep "gcc version" | cut -d' ' -f3 | cut -d'.' -f1-2)
       if [[ ${#VERSION} == 3 ]] ; then
          VISITARCH=${VISITARCH}-${VERSION}
       fi
    fi
fi

START_DIR="$PWD"
if [[ "$DOWNLOAD_ONLY" == "no" ]] ; then
   if [[ ! -d "$THIRD_PARTY_PATH" ]] ; then
      if [[ "$THIRD_PARTY_PATH" == "./visit" ]] ; then
         mkdir "$THIRD_PARTY_PATH"
         if [[ $? != 0 ]] ; then
            error "Unable to write files to the third party library location." \
                  "Bailing out."
         fi
      else
         if [[ "$GRAPHICAL" == "yes" ]] ; then
             $DLG --backtitle "$DLG_BACKTITLE" --yesno "The third party library location does not exist. Create it?" 0 0
             if [[ $? == 1 ]] ; then
                 error "The third party library location does not exist." \
                       "Bailing out."
             else
                 mkdir "$THIRD_PARTY_PATH"
                 if [[ $? != 0 ]] ; then
                    error "Unable to write files to the third party library location." \
                          "Bailing out."
                 fi
             fi
         else
             info "The third party library location does not exist. Create it?"
             read RESPONSE
             if [[ "$RESPONSE" != "yes" ]] ; then
                 error "The third party library location does not exist." \
                       "Bailing out."
             else
                 mkdir "$THIRD_PARTY_PATH"
                 if [[ $? != 0 ]] ; then
                    error "Unable to write files to the third party library location." \
                          "Bailing out."
                 fi
             fi
         fi
      fi
   fi

   cd "$THIRD_PARTY_PATH"
   if [[ $? != 0 ]] ; then
      error "Unable to access the third party location. Bailing out."
   fi
fi
export VISITDIR=${VISITDIR:-$(pwd)}


if [[ "$DO_REQUIRED_THIRD_PARTY" == "yes" ]] ; then
    for (( i=0; i<${#thirdpartylibs[*]}; ++i ))
    do
        initializeFunc="bv_${thirdpartylibs[$i]}_enable"
        $initializeFunc
    done
    if [[ "$DO_DBIO_ONLY" == "yes" ]]; then
        #disable all non dbio libraries
        for (( i=0; i<${#nodbiolibs[*]}; ++i ))
        do
            initializeFunc="bv_${nodbiolibs[$i]}_disable"
            $initializeFunc
        done
    fi
fi

if [[ "$DO_ICET" == "yes" && "$PREVENT_ICET" != "yes" ]] ; then
    DO_CMAKE="yes"
fi


cd "$START_DIR"


#
# Later we will build Qt.  We are going to bypass their licensing agreement,
# so echo it here.
#

check_if_installed "qt" $QT_VERSION
if [[ $? == 0 ]] ; then
   DO_QT="no"
fi

if [[ "$DO_QT" == "yes" && "$DOWNLOAD_ONLY" == "no" ]] ; then
    qt_license_prompt
    if [[ $? != 0 ]] ;then
        error "Qt4 Open Source Edition License Declined. Bailing out."
    fi
fi

#
# Save stdout as stream 3, redirect stdout and stderr to the log file.
# After this maks sure to use the info/warn/error functions to display 
# messages to the user
#

if [[ "${LOG_FILE}" != "/dev/tty" ]] ; then
    exec 3>&1 >> ${LOG_FILE} 2>&1
    REDIRECT_ACTIVE="yes"
else
    exec 2>&1
fi

#
# Log build_visit invocation w/ arguments & the start time.
# Especially helpful if there are multiple starts dumped into the
# same log.
#
LINES="------------------------------------------------------------" 
log $LINES
log $0 $@
log "Started:" $(date)
log $LINES

if [[ "$DO_SVN" == "yes" ]] ; then
    check_svn_client
    if [[ $? != 0 ]]; then
        error "Fatal Error: SVN mode selected, but svn client is not available."
    fi
fi

#
# Now make sure that we have everything we need to build VisIt, so we can bail
# out early if we are headed for failure.
#
check_files
if [[ $? != 0 ]] ; then
   error "Stopping build because necessary files aren't available."
fi

#
# Exit if we were told to only download the files.
#
if [[ "$DOWNLOAD_ONLY" == "yes" ]] ; then
    info "Successfully downloaded the specified files."
    exit 0
fi

#
# We are now ready to build.
#

for (( i=0; i<${#reqlibs[*]}; ++i ))
do
    cd "$START_DIR"
    echo "building required libraries"
    initializeFunc="bv_${reqlibs[$i]}_build"
    $initializeFunc
done

for (( i=0; i<${#optlibs[*]}; ++i ))
do
    cd "$START_DIR"
    echo "building optional libraries"
    initializeFunc="bv_${optlibs[$i]}_build"
    $initializeFunc
done

#
# Create the host.conf file
#

if [[ "$DO_HOSTCONF" == "yes" ]] ; then
    info "Creating host.conf"
    build_hostconf
fi

#
# Build the actual VisIt code
#

if [[ "$DO_VISIT" == "yes" ]] ; then
    cd "$START_DIR"
    info "Building VisIt (~50 minutes)"
    build_visit
    if [[ $? != 0 ]] ; then
        error "Unable to build or install VisIt.  Bailing out."
    fi

    if [[ "$DO_SVN" == "yes" && "$USE_VISIT_FILE" == "no" ]] ; then
        VISIT_SRC_DIR="src"
    else
        VISIT_SRC_DIR="${VISIT_FILE%.tar*}/src"
    fi

FINISHED_MSG="Finished building VisIt.\n\n\
You many now try to run VisIt by cd'ing into the \
$VISIT_SRC_DIR/bin directory and invoking \"visit\".\n\n\
To create a binary distribution tarball from this build, cd to \
${START_DIR}/${VISIT_SRC_DIR}\n\
then enter: \"make package\"\n\n\
This will produce a tarball called visitVERSION.ARCH.tar.gz, where \
VERSION is the version number, and ARCH is the OS architecure.\n\n\
To install the above tarball in a directory called \"INSTALL_DIR_PATH\",\
enter: svn_bin/visit-install VERSION ARCH INSTALL_DIR_PATH\n\n\
If you run into problems, contact visit-users@ornl.gov."

    #
    # Output the message indicating that we are finished. I couldn't get
    # the imbedded newlines in the FINISHED_MSG to display properly so I
    # am treating the graphical and console interfaces differently.
    #
    if test "${GRAPHICAL}" = "yes" ; then
        if [[ "$REDIRECT_ACTIVE" == "yes" ]] ; then
            info_box_large "$FINISHED_MSG" 1>&3
        else
            info_box_large "$FINISHED_MSG"
        fi
        log "Finished building VisIt."
        log
        log "You many now try to run VisIt by cd'ing into the"
        log "$VISIT_SRC_DIR/bin directory and invoking \"visit\""
        log
        log "To create a binary distribution tarball from this build, cd to"
        log "${START_DIR}/${VISIT_SRC_DIR}"
        log "then enter: \"make package\""
        log
        log "This will produce a tarball called visitVERSION.ARCH.tar.gz, where"
        log "VERSION is the version number, and ARCH is the OS architecure."
        log
        log "To install the above tarball in a directory called \"INSTALL_DIR_PATH\""
        log "enter: svn_bin/visit-install VERSION ARCH INSTALL_DIR_PATH"
        log
        log "If you run into problems, contact visit-users@ornl.gov."
    else
        info "Finished building VisIt."
        info
        info "You many now try to run VisIt by cd'ing into the"
        info "$VISIT_SRC_DIR/bin directory and invoking \"visit\""
        info
        info "To create a binary distribution tarball from this build, cd to"
        info "${START_DIR}/${VISIT_SRC_DIR}"
        info "then enter: \"make package\""
        info
        info "This will produce a tarball called visitVERSION.ARCH.tar.gz, where"
        info "VERSION is the version number, and ARCH is the OS architecure."
        info
        info "To install the above tarball in a directory called \"INSTALL_DIR_PATH\""
        info "enter: svn_bin/visit-install VERSION ARCH INSTALL_DIR_PATH"
        info
        info "If you run into problems, contact visit-users@ornl.gov."
    fi
else
    if [[ $ANY_ERRORS == "no" ]] ; then
        info "Finished!"
    else
        info "Finished with Errors"
    fi
fi
}

