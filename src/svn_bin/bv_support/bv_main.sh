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

# Determine if gfortran is present. This overly complex coding is to prevent
# the "which" command from echoing failure to the user.
which gfortran >& /dev/null
if [[ $? == 0 ]]; then
    export GFORTRAN=`which gfortran | grep '^/'`
else
    export GFORTRAN=""
fi

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
   export FC_COMPILER=""
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
   export FC_COMPILER=${FC_COMPILER:-$GFORTRAN}
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
   export FC_COMPILER=${FC_COMPILER:-$GFORTRAN}
   export CXX_COMPILER=${CXX_COMPILER:-"g++"}
   export C_OPT_FLAGS=${C_OPT_FLAGS:-"-O2"}
   export CXX_OPT_FLAGS=${CXX_OPT_FLAGS:-"-O2"}
   export MAKE=${MAKE:-"gmake"}
   export MESA_TARGET=${MESA_TARGET:-"irix6-64-dso"}
elif [[ "$OPSYS" == "SunOS" ]]; then
   export ARCH=${ARCH:-"sunos5"}
   export SO_EXT="so"
   export C_COMPILER=${C_COMPILER:-"gcc"}
   export FC_COMPILER=${FC_COMPILER:-$GFORTRAN}
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
   export FC_COMPILER=${FC_COMPILER:-$GFORTRAN}
   export CXX_COMPILER=${CXX_COMPILER:-"g++"}
   export C_OPT_FLAGS=${C_OPT_FLAGS:-"-O2"}
   export CXX_OPT_FLAGS=${CXX_OPT_FLAGS:-"-O2"}
   export QT_PLATFORM=${QT_PLATFORM:-"linux-g++"}
fi
export MAKE=${MAKE:-"make"}
export THIRD_PARTY_PATH=${THIRD_PARTY_PATH:-"./visit"}
export GROUP=${GROUP:-"visit"}
#export LOG_FILE=${LOG_FILE:-"${0##*/}_log"}
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

#initialize VisIt
bv_visit_initialize

#
# OPTIONS
#
#initialize required libraries..

for (( bv_i=0; bv_i<${#reqlibs[*]}; ++bv_i ))
do
    initializeFunc="bv_${reqlibs[$bv_i]}_initialize"
    $initializeFunc
done

for (( bv_i=0; bv_i<${#optlibs[*]}; ++bv_i ))
do
    initializeFunc="bv_${optlibs[$bv_i]}_initialize"
    $initializeFunc
done

export DO_HOSTCONF="yes"
export ON_HOSTCONF="on"

export DO_DEBUG="no"
export ON_DEBUG="off"
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
export DO_FORTRAN="no"
export ON_FORTRAN="off"
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
export DO_ENGINE_ONLY="no"
export DO_SERVER_COMPONENTS_ONLY="no"
export DO_STATIC_BUILD="no"
export USE_VISIBILITY_HIDDEN="no"
export VISIT_INSTALL_PREFIX=""
export VISIT_BUILD_MODE="Release"
DOWNLOAD_ONLY="no"

if [[ "$OPSYS" == "Darwin" ]]; then
    export DO_MESA="yes"
    export ON_MESA="on"
fi

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


#get visit information..
bv_visit_info

#
# TARBALL LOCATIONS AND VERSIONS
#
if [[ "$VISIT_FILE" != "" ]] ; then
  USE_VISIT_FILE="yes"
  ON_USE_VISIT_FILE="on"
fi
export VISIT_FILE=${VISIT_FILE:-"visit${VISIT_VERSION}.tar.gz"}


for (( bv_i=0; bv_i<${#reqlibs[*]}; ++bv_i ))
do
    initializeFunc="bv_${reqlibs[$bv_i]}_info"
    $initializeFunc
done

for (( bv_i=0; bv_i<${#optlibs[*]}; ++bv_i ))
do
    initializeFunc="bv_${optlibs[$bv_i]}_info"
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
VISIT_INSTALLATION_BUILD_DIR=""
VISIT_DRY_RUN=0
DO_SUPER_BUILD="no"
DO_MANGLED_LIBRARIES="no"
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
    if test "${1:0:1}" = "\""; then #"
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
    if test "${1: -1:1}" = "\""; then #"
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

function bv_enable_group 
{
    local name=${1/--}
    local match=0

    for (( bv_i=0; bv_i < ${#grouplibs_name[*]}; ++bv_i ))
    do
        #replace | with space
        group_flag=${grouplibs_name[$bv_i]}
        group_flag=${group_flag//\|/ }
        for group in `echo $group_flag`;
        do
            if [[ "$group" == "$name" ]]; then
                echo "executing group $name"
                match=1
                for group_dep in `echo ${grouplibs_deps[$bv_i]}`;
                do
                    if [[ "$group_dep" == no-* ]]; then
                        group_dep=${group_dep/no-}
                        #info "disabling $group_dep"
                        initializeFunc="bv_${group_dep}_disable"
                        $initializeFunc
                    else
                        #info "enabling $group_dep"
                        initializeFunc="bv_${group_dep}_enable"
                        $initializeFunc
                    fi
                done
            fi
        done
    done

    return $match
}

function enable_dependent_libraries
{
    local depends_on=""

    for (( bv_i=0; bv_i<${#reqlibs[*]}; ++bv_i ))
    do
        $"bv_${reqlibs[$bv_i]}_is_enabled"

        #if not enabled then skip
        if [[ $? == 0 ]]; then
            continue
        fi

        #enabled library, check dependencies..
        depends_on=$("bv_${reqlibs[$bv_i]}_depends_on")

        #replace commas with spaces if there are any..
        depends_on=${depends_on//,/ }

        for depend_lib in `echo $depends_on`;
        do
            $"bv_${depend_lib}_is_enabled"
            if [[ $? == 0 ]]; then
                echo "library ${depend_lib} was not set but another library depends on it, enabling it"
                $"bv_${depend_lib}_enable"
            fi
        done
    done

    for (( bv_i=0; bv_i<${#optlibs[*]}; ++bv_i ))
    do
        $"bv_${optlibs[$bv_i]}_is_enabled"

        #if not enabled then skip
        if [[ $? == 0 ]]; then
            continue
        fi

        #enabled library, check dependencies..
        depends_on=$("bv_${optlibs[$bv_i]}_depends_on")

        #replace commas with spaces if there are any..
        depends_on=${depends_on//,/ }

        for depend_lib in `echo $depends_on`;
        do
            $"bv_${depend_lib}_is_enabled"
            if [[ $? == 0 ]]; then
                echo "library ${depend_lib} was not set but another library depends on it, enabling it"
                $"bv_${depend_lib}_enable"
            fi
        done
    done
}

#TODO: enable this feature and remove this from ensure..
function initialize_module_variables
{
    info "initializing module variables"
    for (( bv_i=0; bv_i<${#reqlibs[*]}; ++bv_i ))
    do
        declare -F "bv_${reqlibs[$bv_i]}_initialize_vars" &>/dev/null 
        
        if [[ $? == 0 ]]; then
            info "initialize module variables for ${reqlibs[$bv_i]}"
            $"bv_${reqlibs[$bv_i]}_initialize_vars"
        fi
    done

    for (( bv_i=0; bv_i<${#optlibs[*]}; ++bv_i ))
    do
        declare -F "bv_${optlibs[$bv_i]}_initialize_vars" &>/dev/null
        
        if [[ $? == 0 ]]; then
            info "initialize module variables for ${optlibs[$bv_i]}"
            $"bv_${optlibs[$bv_i]}_initialize_vars"
        fi
    done
}


function build_library
{
    local build_lib=$1
    local depends_on=""

    #check if library is already installed..
    $"bv_${build_lib}_is_installed"
    
    if [[ $? == 1 ]]; then
        info "$build_lib is already installed, skipping"
        return
    fi    

    #Make sure that the recursive enable feature is working properly
    $"bv_${build_lib}_is_enabled"
    
    if [[ $? == 0 ]]; then
        error "$build_lib was disabled, but seems that another library requires it "
    fi

    depends_on=$("bv_${build_lib}_depends_on")
    
    if [[ $depends_on != "" ]]; then
        info "library $build_lib depends on $depends_on"
    fi

    #replace commas with spaces if there are any..
    depends_on=${depends_on//,/ }
    
    for depend_lib in `echo $depends_on`;
    do
        build_library $depend_lib
    done
    
    #build ..
    $"bv_${build_lib}_build"
}

function build_libraries_serial
{
    info "building required libraries"
    for (( bv_i=0; bv_i<${#reqlibs[*]}; ++bv_i ))
    do
        $"bv_${reqlibs[$bv_i]}_is_enabled"
     
        if [[ $? == 0 ]]; then
            continue
        fi

        $"bv_${reqlibs[$bv_i]}_is_installed"

        if [[ $? == 0 ]]; then
            cd "$START_DIR"
            build_library ${reqlibs[$bv_i]}
        else
            info "${reqlibs[$bv_i]} already installed, skipping"
        fi
    done

    info "building optional libraries"
    for (( bv_i=0; bv_i<${#optlibs[*]}; ++bv_i ))
    do
        $"bv_${optlibs[$bv_i]}_is_enabled"
    
        if [[ $? == 0 ]]; then
            continue
        fi

        $"bv_${optlibs[$bv_i]}_is_installed"

        if [[ $? == 0 ]]; then
            cd "$START_DIR"
            build_library ${optlibs[$bv_i]}
        else
            info "${optlibs[$bv_i]} already installed, skipping"
        fi  
    done
}

function build_libraries_parallel
{
    #launch all non dependent libraries in parallel..
    info "building parallel required libraries"
    for (( bv_i=0; bv_i<${#reqlibs[*]}; ++bv_i ))
    do
        $"bv_${reqlibs[$bv_i]}_is_enabled"
     
        if [[ $? == 0 ]]; then
            continue
        fi

        $"bv_${reqlibs[$bv_i]}_is_installed"
        if [[ $? == 0 ]]; then
        
            depends_on=$("bv_${reqlibs[$bv_i]}_depends_on")
            if [[ "$depends_on" == "" ]]; then
                (cd "$START_DIR" && build_library ${reqlibs[$bv_i]}) &
            fi
        else
            info "${reqlibs[$bv_i]} already installed, skipping"
        fi
    done

    wait

    #load the serial ones..
    for (( bv_i=0; bv_i<${#reqlibs[*]}; ++bv_i ))
    do
        $"bv_${reqlibs[$bv_i]}_is_enabled"

        if [[ $? == 0 ]]; then
            continue
        fi

        $"bv_${reqlibs[$bv_i]}_is_installed"

        if [[ $? == 0 ]]; then
            cd "$START_DIR"
            build_library ${reqlibs[$bv_i]}
        else
            info "${reqlibs[$bv_i]} already installed, skipping"
        fi
    done

    info "building parallel optional libraries"
    for (( bv_i=0; bv_i<${#optlibs[*]}; ++bv_i ))
    do
        $"bv_${optlibs[$bv_i]}_is_enabled"
    
        if [[ $? == 0 ]]; then
            continue
        fi
        $"bv_${optlibs[$bv_i]}_is_installed"
        if [[ $? == 0 ]]; then

            depends_on=$("bv_${optlibs[$bv_i]}_depends_on")
            if [[ "$depends_on" == "" ]]; then
                (cd "$START_DIR" && build_library ${optlibs[$bv_i]}) &
            fi
        else
            info "${optlibs[$bv_i]} already installed, skipping"
        fi  
    done

    wait

    #load serial
    for (( bv_i=0; bv_i<${#optlibs[*]}; ++bv_i ))
    do
        $"bv_${optlibs[$bv_i]}_is_enabled"

        if [[ $? == 0 ]]; then
            continue
        fi

        $"bv_${optlibs[$bv_i]}_is_installed"

        if [[ $? == 0 ]]; then
            cd "$START_DIR"
            build_library ${optlibs[$bv_i]}
        else
            info "${optlibs[$bv_i]} already installed, skipping"
        fi
    done

} 

# *************************************************************************** #
#                       Section 2, building VisIt                             #
# --------------------------------------------------------------------------- #
# This section does some set up for building VisIt, and then calls the        #
# functions to build the third party libraries and VisIt itself.              #
# *************************************************************************** #
function run_build_visit()
{
declare -a arguments

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

#handle groups first since they affect multiple libraries..
for arg in "$@" ; 
do
    bv_enable_group "$arg"
    #not part of a group, add to argument list..
    if [[ $? == 0 ]]; then
        xmlp_licenseMatch "$arg"
        #suppress licenses as well..
        if [[ $? == 0 ]]; then
            arguments[${#arguments[*]}]="$arg"
        fi
    fi
done

for arg in "${arguments[@]}" ; do

    # Was the last option something that took an argument?
    if test -n "$next_arg" ; then
        # Yep.  Which option was it?
        case $next_arg in
            extra_commandline_arg) $EXTRA_COMMANDLINE_ARG_CALL "$arg";;
            installation-build-dir) VISIT_INSTALLATION_BUILD_DIR="$arg";;
            write-unified-file) WRITE_UNIFIED_FILE="$arg";;
            append-cflags) C_OPT_FLAGS="${C_OPT_FLAGS} ${arg}";;
            append-cxxflags) CXX_OPT_FLAGS="${CXX_OPT_FLAGS} ${arg}";;
            arch) VISITARCH="${arg}";;
            build-mode) VISIT_BUILD_MODE="${arg}";;
            cflags) C_OPT_FLAGS="${arg}";;
            cxxflags) CXX_OPT_FLAGS="${arg}";;
            cc) C_COMPILER="${arg}";;
            cxx) CXX_COMPILER="${arg}";;
            makeflags) MAKE_OPT_FLAGS="${arg}";;
            prefix) VISIT_INSTALL_PREFIX="${arg}";;
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

        #one module at a time
        resolve_arg=${arg:2} #remove --
        declare -F "bv_${resolve_arg}_enable" &>/dev/null

        if [[ $? == 0 ]] ; then 
            #echo "enabling ${resolve_arg}"
            initializeFunc="bv_${resolve_arg}_enable"
            $initializeFunc
            continue
        elif [[ ${#resolve_arg} -gt 3 ]] ; then #in case it is --no-
            resolve_arg_no_opt=${resolve_arg:3}
            #disable library if it does not exist..
            declare -F "bv_${resolve_arg_no_opt}_disable" &>/dev/null
            if [[ $? == 0 ]] ; then
                #echo "disabling ${resolve_arg_no_opt}"
                initializeFunc="bv_${resolve_arg_no_opt}_disable"
                $initializeFunc
                #if disabling icet, prevent it as well
                if [[ ${resolve_arg_no_opt} == "icet" ]]; then
                    echo "preventing icet from starting"
                    PREVENT_ICET="yes"
                fi
                continue
           fi
        fi
        
        #command line arguments created by modules
        #checking to see if additional command line arguments were requested
        resolve_arg=${arg:2} #remove --
        local match=0
        for (( bv_i=0; bv_i<${#extra_commandline_args[*]}; bv_i += 5 ))
        do
            local module_name=${extra_commandline_args[$bv_i]} 
            local command=${extra_commandline_args[$bv_i+1]} 
            local args=${extra_commandline_args[$bv_i+2]} 
            local comment=${extra_commandline_args[$bv_i+3]} 
            local fp=${extra_commandline_args[$bv_i+4]} 
            if [[ "$command" == "$resolve_arg" ]]; then
                if [ $args -eq 0 ] ; then 
                  #call function immediately
                  $fp
                else 
                  #call function with next argument
                  next_arg="extra_commandline_arg"
                  EXTRA_COMMANDLINE_ARG_CALL="$fp"
                fi 
                match=1
                break;
            fi
        done

        #found a match in the modules..
        if [[ $match -eq 1 ]]; then
           continue
        fi
    fi

             
    case $arg in
        --installation-build-dir) next_arg="installation-build-dir";;
        --write-unified-file) next_arg="write-unified-file";;
        --parallel-build) DO_SUPER_BUILD="yes";;
        --mangle-libraries) DO_MANGLED_LIBRARIES="yes";;
        --dry-run) VISIT_DRY_RUN=1;;
        --arch) next_arg="arch";;
        --build-mode) next_arg="build-mode";;
        --cflag) next_arg="append-cflags";;
        --cflags) next_arg="cflags";;
        --cxxflag) next_arg="append-cxxflags";;
        --cxxflags) next_arg="cxxflags";;
        --cc) next_arg="cc";;
        --cxx) next_arg="cxx";;
        --console) GRAPHICAL="no"; ON_GRAPHICAL="off";;
        --debug) set -vx;;
        --download-only) DOWNLOAD_ONLY="yes";;
        --engine-only) DO_ENGINE_ONLY="yes";;
        --flags-debug) C_OPT_FLAGS="${C_OPT_FLAGS} -g"; CXX_OPT_FLAGS="${CXX_OPT_FLAGS} -g"; VISIT_BUILD_MODE="Debug";;
        --gdal) DO_GDAL="yes"; ON_GDAL="on";;
        --fortran) DO_FORTRAN="yes"; ON_FORTRAN="on";;
        --group) next_arg="group"; DO_GROUP="yes"; ON_GROUP="on";;
        -h|--help) next_action="help";;
        --java) DO_JAVA="yes"; ON_JAVA="on";;
        --makeflags) next_arg="makeflags";;
        --no-hostconf) DO_HOSTCONF="no"; ON_HOSTCONF="off";;
        --parallel) parallel="yes"; DO_ICET="yes"; ON_ICET="on"; DO_MESA="yes"; ON_MESA="on"; ON_parallel="on";;
        --prefix) next_arg="prefix";;
        --print-vars) next_action="print-vars";;
        --python-module) DO_MODULE="yes"; ON_MODULE="on";;
        --server-components-only) DO_SERVER_COMPONENTS_ONLY="yes";;
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

#error check to make sure that next arg is not left blank..
if [[ $next_arg != "" ]] ; then
    echo "command line arguments are used incorrectly: argument $next_arg not fullfilled"
    exit 0
fi

if test -n "${deprecated}" ; then
    summary="You are using some deprecated options to $0.  Please re-run"
    summary="${summary} $0 with a command line similar to:"
    echo "$summary"
    echo ""
    echo "$0 ${deprecated}"
    exit 0
fi

if test -n "${next_action}" ; then
    case ${next_action} in
        print-vars) printvariables; exit 2;;
        help) usage; exit 2;;
    esac
fi

#write a unified file
if [[ $WRITE_UNIFIED_FILE != "" ]] ; then
    bv_write_unified_file $WRITE_UNIFIED_FILE
    exit 0
fi

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

# Disable fortran support unless --fortran specified and a fortran compiler
# was specified or found.
if [[ $DO_FORTRAN == "no" || $FC_COMPILER == "" ]]; then
    export FC_COMPILER="no";
    warn "Fortran support for thirdparty libraries disabled."
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
    result=$($DLG --backtitle "$DLG_BACKTITLE" \
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
           "Groups"    "select 3rd party and advanced build"  $ON_OPTIONAL\
           "Custom"    "select custom flags provided by each module"  $ON_OPTIONAL2\
           "SVN"        "get sources from SVN server"     $ON_SVN\
           "Tarball"    "specify VisIt tarball name"      $ON_USE_VISIT_FILE\
           "Parallel"   "specify parallel build flags"    $ON_parallel\
           "Python"     "enable VisIt python module"      $ON_MODULE\
           "Java"       "enable java client library"      $ON_JAVA\
           "Fortran"    "enable fortran in third party libraries"  $ON_FORTRAN\
           "SLIVR"      "enable SLIVR volume rendering library"  $ON_SLIVR\
           "EnvVars"     "specify build environment var values"   $ON_verify\
           "Advanced"   "display advanced options"        $ON_MORE  3>&1 1>&2 2>&3)
    retval=$?

    # Remove the extra quoting, new dialog has --single-quoted
    choice="$(echo $result | sed 's/\"//g' )"

    case $retval in
      0)
        DO_OPTIONAL="no"
        DO_OPTIONAL2="no"
        DO_SVN="no"
        USE_VISIT_FILE="no"
        parallel="no"
        DO_MODULE="no"
        DO_JAVA="no"
        DO_FORTRAN="no"
        DO_SLIVR="no"
        verify="no"
        DO_MORE="no"
        for OPTION in $choice
        do
            case $OPTION in
              Groups)
                 DO_OPTIONAL="yes";;
              Custom)
                 DO_OPTIONAL2="yes";;
              SVN)
                 DO_SVN="yes";DO_SVN_ANON="yes";export SVN_ROOT_PATH=$SVN_ANON_ROOT_PATH ;;
              Tarball)
                 result=$($DLG --backtitle "$DLG_BACKTITLE" \
                    --nocancel --inputbox \
"Enter $OPTION value:" 0 $DLG_WIDTH_WIDE "$VISIT_FILE" 3>&1 1>&2 2>&3)
                 VISIT_FILE="$(echo $result)"
                 USE_VISIT_FILE="yes";;
              Parallel)
                 parallel="yes"; DO_ICET="yes"; ON_ICET="on"; DO_MESA="yes"; ON_MESA="on";;
              PythonModule)
                 DO_MODULE="yes";;
              Java)
                 DO_JAVA="yes";;
              Fortran)
                 DO_FORTRAN="yes";;
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

if [[ "$DO_OPTIONAL" == "yes" && "$GRAPHICAL" == "yes" ]] ; then

    local add_checklist_vars=""

    for (( bv_i=0; bv_i < ${#grouplibs_name[*]}; ++bv_i ))
    do
        name="${grouplibs_name[$bv_i]}"
        comment="${grouplibs_comment[$bv_i]}"
        output_str="$name \"$comment\" 0 \"  Customize($name)\" \"Customize selection of $name\" 0"    
        add_checklist_vars="$add_checklist_vars ${output_str}"
    done

    #calling whiptail directly is having issues delimiting by space , so hacking around it
    result=$(echo "$DLG --backtitle \"Group options\" --title \"Groups provided by build_visit\" --checklist \"select\" 0 0 0 $add_checklist_vars 3>&1 1>&2 2>&3 || echo \"Cancelled Operation\"" | awk '{system($0)}')

    if [[ $result == "Cancelled Operation" ]]; then
        warn "Operation was cancelled, exiting"
        exit 1
    fi

    # Remove the extra quoting, new dialog has --single-quoted
    choice="$(echo $result | sed 's/\"//g' )"
    for OPTION in $choice
    do
        #if customize operation
        if [[ "$OPTION" != Customize* ]]; then 
            #execute the entire group..
            bv_enable_group "$OPTION"
            continue
        fi
        
        #allow modification of predefined groups
        add_checklist_vars=""

        for (( bv_i=0; bv_i < ${#grouplibs_name[*]}; ++bv_i ))
        do
            name="${grouplibs_name[$bv_i]}"
            #dialog and whiptail treat parantheses differently
            if [[ "Customize($name)" == "$OPTION" || "Customize\\($name\\)" == "$OPTION" ]]; then 
                echo "Customizing $name"
                comment="${grouplibs_comment[$bv_i]}"

                for group_dep in `echo ${grouplibs_deps[$bv_i]}`
                do
                    output_str="${group_dep} \"Enable/Disable $group_dep library\" 1"
                    add_checklist_vars="$add_checklist_vars ${output_str}"
                done
                result=$(echo "$DLG --backtitle \"$comment\" \
                                    --title \"Custom options for $name\" \
                                    --checklist \"select\" 0 0 0 $add_checklist_vars \
                                    3>&1 1>&2 2>&3 || echo \"Cancelled Operation\"" \
                                    | awk '{system($0)}')

                if [[ $result == "Cancelled Operation" ]]; then
                    echo "Operation was cancelled"
                    exit 1
                fi
                choice_custom="$(echo $result | sed 's/\"//g' )"
                for OPTION_CUSTOM in $choice_custom
                do
                    #initialize all the ones
                    if [[ "$OPTION_CUSTOM" == no-* ]]; then
                        OPTION_CUSTOM=${OPTION_CUSTOM/no-}
                        #echo "disabling $OPTION_CUSTOM"
                        initializeFunc="bv_${OPTION_CUSTOM}_disable"
                        $initalizeFunc
                    else
                        #echo "enable $OPTION_CUSTOM"
                        initializeFunc="bv_${OPTION_CUSTOM}_enable"
                        $initializeFunc
                    fi
                done
            fi
        done
    done
fi

if [[ "$DO_OPTIONAL2" == "yes" && "$GRAPHICAL" == "yes" ]] ; then

    local needsRerun=1

    while [[ $needsRerun == 1 ]];
    do
        needsRerun=0
        local add_checklist_vars=""
        for (( bv_i=0; bv_i<${#extra_commandline_args[*]}; bv_i += 5 ))
        do
            local module_name=${extra_commandline_args[$bv_i]} 
            local command=${extra_commandline_args[$bv_i+1]} 
            #local args=${extra_commandline_args[$bv_i+2]} 
            local comment=${extra_commandline_args[$bv_i+3]} 
            output_str="\"$command\" \"$comment ($module_name)\" 0"    
            add_checklist_vars="$add_checklist_vars ${output_str}"
        done

        #calling whiptail directly is having issues delimiting by space , so hacking around it
        result=$(echo "$DLG --backtitle \"Custom options\" --title \"d by build_visit\" --checklist \"select\" 0 0 0 $add_checklist_vars 3>&1 1>&2 2>&3 || echo \"Cancelled Operation\"" | awk '{system($0)}')

        if [[ $result == "Cancelled Operation" ]]; then
            warn "Operation was cancelled, skipping"
            break
        fi

        # Remove the extra quoting, new dialog has --single-quoted
        choice="$(echo $result | sed 's/\"//g' )"
        for OPTION in $choice
        do
            for (( bv_i=0; bv_i<${#extra_commandline_args[*]}; bv_i += 5 ))
            do
                local module_name=${extra_commandline_args[$bv_i]} 
                local command=${extra_commandline_args[$bv_i+1]} 
                local args=${extra_commandline_args[$bv_i+2]} 
                local comment=${extra_commandline_args[$bv_i+3]} 
                local fp=${extra_commandline_args[$bv_i+4]} 
                if [[ "$command" == "$OPTION" ]]; then
                    if [[ $args -ne 0 ]]; then
                        result=$($DLG --backtitle "$DLG_BACKTITLE" \
                            --inputbox \
                            "Enter $OPTION value:" 0 $DLG_WIDTH_WIDE "" 3>&1 1>&2 2>&3)
                        retval=$?
                        if [[ $retval == 0 ]]; then
                            echo "executing $command"
                            $fp "$result"
                        else
                            needsRerun=1
                        fi
                    else
                        $fp
                    fi
                fi
            done
        done
    done
fi

# make all VisIt related builds in its own directory..
if [[ $VISIT_INSTALLATION_BUILD_DIR != "" ]] ; then
    if [[ -d $VISIT_INSTALLATION_BUILD_DIR ]]; then
        echo "Using already existing directory: $VISIT_INSTALLATION_BUILD_DIR"
    else
        mkdir -p $VISIT_INSTALLATION_BUILD_DIR
    fi

    if [[ ! -d $VISIT_INSTALLATION_BUILD_DIR ]]; then
        echo "Directory does not exist or I do not have permission to create it. Quitting"
        exit 0
    fi
    cd $VISIT_INSTALLATION_BUILD_DIR
fi

#
# See if the user needs to modify some variables
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
if [[ "$DO_ICET" == "yes" && "$PREVENT_ICET" != "yes" ]] ; then
    DO_CMAKE="yes"
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

#enabling any dependent libraries, handles both dependers and dependees..
#TODO: handle them seperately
info "enabling any dependent libraries"
enable_dependent_libraries

## At this point we are after the command line and the visual selection
#dry run, don't execute anything just run the enabled stuff..
#happens before any downloads have taken place..
if [[ $VISIT_DRY_RUN -eq 1 ]]; then
    for (( bv_i=0; bv_i<${#reqlibs[*]}; ++bv_i ))
    do
        initializeFunc="bv_${reqlibs[$bv_i]}_dry_run"
        $initializeFunc
    done

    for (( bv_i=0; bv_i<${#optlibs[*]}; ++bv_i ))
    do
        initializeFunc="bv_${optlibs[$bv_i]}_dry_run"
        $initializeFunc
    done

    bv_visit_dry_run
    exit 0
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
cd "$START_DIR"

#initialize module variables, since all of VisIt's variables should be set by now..
initialize_module_variables
#
# Later we will build Qt.  We are going to bypass their licensing agreement,
# so echo it here.
#
if [[ "$USE_SYSTEM_QT" != "yes" && "$DO_QT" == "yes" ]]; then

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
fi

if [[ $DO_MANGLED_LIBRARIES == "yes" ]]; then
    info "Mangling libraries while building"
    info "Any libraries that support mangling will do so"
fi

if [[ "$DO_SUPER_BUILD" == "yes" ]]; then
    build_libraries_parallel
else
    build_libraries_serial
fi

#
# Create the host.conf file
#

if [[ "$DO_HOSTCONF" == "yes" ]] ; then
    info "Creating host.conf"
    build_hostconf
fi

#build visit itself..
bv_visit_build
}

