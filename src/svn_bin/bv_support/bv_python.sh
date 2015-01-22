function bv_python_initialize
{
export DO_PYTHON="yes"
export ON_PYTHON="on"
export FORCE_PYTHON="no"
export USE_SYSTEM_PYTHON="no"
export VISIT_PYTHON_DIR=${VISIT_PYTHON_DIR:-""}
add_extra_commandline_args "python" "system-python" 0 "Using system python"
add_extra_commandline_args "python" "alt-python-dir" 1 "Using alternate python directory"
}

function bv_python_enable
{
DO_PYTHON="yes"
ON_PYTHON="on"
FORCE_PYTHON="yes"
}

function bv_python_disable
{
DO_PYTHON="no"
ON_PYTHON="off"
FORCE_PYTHON="no"
}

function bv_python_force
{
  if [[ "$FORCE_PYTHON" == "yes" ]]; then
     return 0;
  fi
  return 1;
}

function python_set_vars_helper
{
  VISIT_PYTHON_DIR=`"$PYTHON_CONFIG_COMMAND" --prefix`
  PYTHON_BUILD_DIR=`"$PYTHON_CONFIG_COMMAND" --prefix`
  PYTHON_VER=`"$PYTHON_COMMAND" --version 2>&1`
  PYTHON_VERSION=${PYTHON_VER#"Python "}
  PYTHON_COMPATIBILITY_VERSION=${PYTHON_VERSION%.*}
  ########################
  PYTHON_INCLUDE_PATH=`"$PYTHON_CONFIG_COMMAND" --includes`
  #remove -I from first include
  PYTHON_INCLUDE_PATH="${PYTHON_INCLUDE_PATH:2}"
  #remove any extra includes
  PYTHON_INCLUDE_PATH="${PYTHON_INCLUDE_PATH%%-I*}"
  PYTHON_INCLUDE_DIR="$PYTHON_INCLUDE_PATH"
  PYTHON_LIBRARY=`"$PYTHON_CONFIG_COMMAND" --libs`
  #remove all other libraries except for python..
  PYTHON_LIBRARY="${PYTHON_LIBRARY##-l*-l}"

  if [[ "$DO_STATIC_BUILD" == "yes" ]]; then
       PYTHON_LIBRARY="lib${PYTHON_LIBRARY}.a"
  else
      if [[ "$OPSYS" == "Darwin" ]]; then
           PYTHON_LIBRARY="lib${PYTHON_LIBRARY}.dylib"
      else
           PYTHON_LIBRARY="lib${PYTHON_LIBRARY}.so"
      fi
  fi
  PYTHON_LIBRARY_DIR="${VISIT_PYTHON_DIR}/lib"
  if [ ! -e "${PYTHON_LIBRARY_DIR}/${PYTHON_LIBRARY}" ]
  then
      # some systems eg fedora use lib64...
      PYTHON_LIBRARY_DIR="${VISIT_PYTHON_DIR}/lib64"
      if [ ! -e "${PYTHON_LIBRARY_DIR}/${PYTHON_LIBRARY}" ]
      then
          error "python library was not found, cannot configure python"
      fi
  fi
  PYTHON_LIBRARY="${PYTHON_LIBRARY_DIR}/${PYTHON_LIBRARY}"
  echo $PYTHON_BUILD_DIR $PYTHON_VERSION $VISIT_PYTHON_DIR

}

function bv_python_system_python
{
  echo "Using system python"
   TEST=`which python-config`
   [ $? != 0 ] && error "System python-config not found, cannot configure python"

  bv_python_enable
  USE_SYSTEM_PYTHON="yes"
  PYTHON_COMMAND="python"
  PYTHON_CONFIG_COMMAND="python-config"
  PYTHON_FILE=""
  python_set_vars_helper #set vars..
}

function bv_python_alt_python_dir
{
  echo "Using alternate python directory"

  [ ! -e "$1/bin/python-config" ] && error "Python not found in $1"

  bv_python_enable
  USE_SYSTEM_PYTHON="yes"
  PYTHON_ALT_DIR="$1"
  PYTHON_COMMAND="$PYTHON_ALT_DIR/bin/python"
  PYTHON_CONFIG_COMMAND="$PYTHON_ALT_DIR/bin/python-config"
  PYTHON_FILE=""
  python_set_vars_helper #set vars..

}


function bv_python_depends_on
{
echo ""
}

function bv_python_info
{
export PYTHON_FILE_SUFFIX="tgz"
export PYTHON_VERSION=${PYTHON_VERSION:-"2.7.6"}
export PYTHON_COMPATIBILITY_VERSION=${PYTHON_COMPATIBILITY_VERSION:-"2.7"}
export PYTHON_FILE="Python-$PYTHON_VERSION.$PYTHON_FILE_SUFFIX"
export PYTHON_BUILD_DIR="Python-$PYTHON_VERSION"
export PYTHON_MD5_CHECKSUM="2cf641732ac23b18d139be077bd906cd"
export PYTHON_SHA256_CHECKSUM=""

export PIL_URL=${PIL_URL:-"http://effbot.org/media/downloads"}
export PIL_FILE=${PIL_FILE:-"Imaging-1.1.6.tar.gz"}
export PIL_BUILD_DIR=${PIL_BUILD_DIR:-"Imaging-1.1.6"}

export PYPARSING_FILE=${PYPARSING_FILE:-"pyparsing-1.5.2.tar.gz"}
export PYPARSING_BUILD_DIR=${PYPARSING_BUILD_DIR:-"pyparsing-1.5.2"}

export PYREQUESTS_FILE=${PYREQUESTS_FILE:-"requests-2.5.1.tar.gz"}
export PYREQUESTS_BUILD_DIR=${PYREQUESTS_BUILD_DIR:-"requests-2.5.1"}

export SEEDME_FILE=${SEEDME_FILE:-"seedme-python-client-cb1f3c409788.tar.gz"}
export SEEDME_BUILD_DIR=${SEEDME_BUILD_DIR:-"seedme-python-client-cb1f3c409788"}


}

function bv_python_print
{
  printf "%s%s\n" "PYTHON_FILE=" "${PYTHON_FILE}"
  printf "%s%s\n" "PYTHON_VERSION=" "${PYTHON_VERSION}"
  printf "%s%s\n" "PYTHON_COMPATIBILITY_VERSION=" "${PYTHON_COMPATIBILITY_VERSION}"
  printf "%s%s\n" "PYTHON_BUILD_DIR=" "${PYTHON_BUILD_DIR}"
}

function bv_python_print_usage
{
printf "%-15s %s [%s]\n" "--python" "Build Python" "built by default unless --no-thirdparty flag is used"
printf "%-15s %s [%s]\n" "--system-python" "Use System Python" "Used unless --no-thirdparty flag is used"
}

function bv_python_host_profile
{
    if [[ "$DO_PYTHON" == "yes" ]] ; then
        echo >> $HOSTCONF
        echo "##" >> $HOSTCONF
        echo "## Python" >> $HOSTCONF
        echo "##" >> $HOSTCONF

        if [[ "$USE_SYSTEM_PYTHON" == "yes" ]]; then
            echo "VISIT_OPTION_DEFAULT(VISIT_PYTHON_DIR $VISIT_PYTHON_DIR)" >> $HOSTCONF
            #incase the PYTHON_DIR does not find the include and library set it manually...
            echo "VISIT_OPTION_DEFAULT(PYTHON_INCLUDE_PATH $PYTHON_INCLUDE_PATH)" >> $HOSTCONF
            echo "VISIT_OPTION_DEFAULT(PYTHON_LIBRARY ${PYTHON_LIBRARY})" >> $HOSTCONF
            echo "VISIT_OPTION_DEFAULT(PYTHON_LIBRARY_DIR $PYTHON_LIBRARY_DIR)" >> $HOSTCONF
            echo "VISIT_OPTION_DEFAULT(PYTHON_VERSION $PYTHON_COMPATIBILITY_VERSION)" >> $HOSTCONF
            echo "SET(VISIT_PYTHON_SKIP_INSTALL ON)" >> $HOSTCONF
        else
            echo "VISIT_OPTION_DEFAULT(VISIT_PYTHON_DIR \${VISITHOME}/python/$PYTHON_VERSION/\${VISITARCH})" \
		>> $HOSTCONF
#           echo "VISIT_OPTION_DEFAULT(VISIT_PYTHON_DIR $VISIT_PYTHON_DIR)" >> $HOSTCONF
        fi
    fi
}

function bv_python_initialize_vars
{
    if [[ "$USE_SYSTEM_PYTHON" == "no" ]]; then
    
        #assign any default values that other libraries should be aware of
        #when they build..
        #this is for when python is being built and system python was not selected..
        export VISIT_PYTHON_DIR=${VISIT_PYTHON_DIR:-"$VISITDIR/python/${PYTHON_VERSION}/${VISITARCH}"}
        export PYTHON_COMMAND="${VISIT_PYTHON_DIR}/bin/python"
        export PYTHON_LIBRARY_DIR="${VISIT_PYTHON_DIR}/bin/python"
        export PYTHON_INCLUDE_DIR="${VISIT_PYTHON_DIR}/include/python${PYTHON_COMPATIBILITY_VERSION}"
        export PYTHON_LIBRARY="${VISIT_PYTHON_DIR}/lib/libpython${PYTHON_COMPATIBILITY_VERSION}.${SO_EXT}"
    fi
}

function bv_python_ensure
{
    if [[ "$USE_SYSTEM_PYTHON" == "no" ]]; then
        if [[ "$DO_DBIO_ONLY" != "yes" ]]; then
            if [[ "$DO_PYTHON" == "yes" || "$DO_VTK" == "yes" ]] ; then
                ensure_built_or_ready "python" $PYTHON_VERSION $PYTHON_BUILD_DIR $PYTHON_FILE
                if [[ $? != 0 ]] ; then
                    return 1
                fi
            fi
        fi
    fi
}

function bv_python_dry_run
{
  if [[ "$DO_PYTHON" == "yes" ]] ; then
    echo "Dry run option not set for python."
  fi
}

function apply_python_osx104_patch
{
   info "Patching Python: fix _environ issue for OS X 10.4"
   patch -f -p0 << \EOF
diff -c Modules.orig/posixmodule.c Modules/posixmodule.c
*** Modules.orig/posixmodule.c  Mon May  3 12:17:59 2010
--- Modules/posixmodule.c       Mon May  3 12:19:31 2010
***************
*** 360,365 ****
--- 360,369 ----
  #endif
  #endif
  
+ /* On OS X 10.4, we need to use a function to get access to environ; 
+  * otherwise we get an unresolved "_environ" when linking shared libs */
+ #define WITH_NEXT_FRAMEWORK
+ 
  /* Return a dictionary corresponding to the POSIX environment table */
  #ifdef WITH_NEXT_FRAMEWORK
  /* On Darwin/MacOSX a shared library or framework has no access to
EOF
   if [[ $? != 0 ]] ; then
      warn "Python patch on OS X 10.4 failed."
      return 1
   fi

   return 0
}

function apply_python_icc_patch
{
   info "Patching Python: fix ffi64 issue for icc"
   patch -f -p0 << \EOF
diff -c Modules/_ctypes/libffi/src/x86/ffi64.c.orig Modules/_ctypes/libffi/src/x86/ffi64.c
*** Modules/_ctypes/libffi/src/x86/ffi64.c.orig	2013-11-12 15:02:51.585653535 -0800
--- Modules/_ctypes/libffi/src/x86/ffi64.c	2013-11-12 15:20:47.000000000 -0800
***************
*** 39,44 ****
--- 39,45 ----
  #define MAX_SSE_REGS 8
  
  #if defined(__INTEL_COMPILER)
+ #include "xmmintrin.h"
  #define UINT128 __m128
  #else
  #if defined(__SUNPRO_C)
EOF
    if [[ $? != 0 ]] ; then
        warn "Python patch for icc fialed."
        return 1
    fi

    return 0
}

function apply_python_patch
{
    if [[ "$OPSYS" == "Darwin" ]]; then
        VER=$(uname -r)
        if [[ ${VER%%.*} == 8 ]] ; then
            apply_python_osx104_patch
            if [[ $? != 0 ]] ; then
                return 1
            fi
        fi
    fi
    if [[ "${C_COMPILER}" == "icc" ]]; then
        apply_python_icc_patch
        if [[ $? != 0 ]] ; then
            return 1
        fi
    fi

    return 0
}


# *************************************************************************** #
#                         Function 7, build_python                            #
# *************************************************************************** #

function build_python
{
    prepare_build_dir $PYTHON_BUILD_DIR $PYTHON_FILE
    untarred_python=$?
    if [[ $untarred_python == -1 ]] ; then
       warn "Unable to prepare Python build directory. Giving Up!"
       return 1
    fi

    #
    # Do any python patches if necessary
    #
    cd $PYTHON_BUILD_DIR || error "Can't cd to Python build dir."
    apply_python_patch
    if [[ $? != 0 ]] ; then
        warn "Patch failed, but continuing."
    fi

    #
    # Call configure
    #
    cCompiler="${C_COMPILER}"
    cFlags="${CFLAGS} ${C_OPT_FLAGS}"
    cxxCompiler="${CXX_COMPILER}"
    cxxFlags="{$CXXFLAGS} ${CXX_OPT_FLAGS}"
    if [[ "$OPSYS" == "Linux" && "$C_COMPILER" == "xlc" ]]; then
        cCompiler="gxlc"
        cxxCompiler="gxlC"
        cFlags=`echo ${CFLAGS} ${C_OPT_FLAGS} | sed "s/-qpic/-fPIC/g"`
        cxxFlags=`echo $CXXFLAGS} ${CXX_OPT_FLAGS} | sed "s/-qpic/-fPIC/g"`
    fi
    PYTHON_OPT="$cFlags"
    PYTHON_LDFLAGS=""
    PYTHON_PREFIX_DIR="$VISITDIR/python/$PYTHON_VERSION/$VISITARCH"
    if [[ "$DO_STATIC_BUILD" == "no" ]]; then
        PYTHON_SHARED="--enable-shared"
        if [[ "$C_COMPILER" == "gcc" ]]; then
            #
            # python's --enable-shared configure flag doesn't link
            # the exes it builds correclty when installed to a non standard
            # prefix. To resolve this we need to add a rpath linker flags.
            #
            mkdir -p ${PYTHON_PREFIX_DIR}/lib/
            if [[ $? != 0 ]] ; then
                 warn "Python configure failed.  Giving up"
            return 1
            fi

            if [[ "$OPSYS" != "Darwin" || ${VER%%.*} -ge 9 ]]; then
                PYTHON_LDFLAGS="-Wl,-rpath,${PYTHON_PREFIX_DIR}/lib/ -pthread"
            fi
        fi
    fi
    if [[ "$OPSYS" == "AIX" ]]; then
        info "Configuring Python (AIX): ./configure OPT=\"$PYTHON_OPT\" CXX=\"$cxxCompiler\" CC=\"$cCompiler\"" \
             "--prefix=\"$PYTHON_PREFIX_DIR\" --disable-ipv6"
        ./configure OPT="$PYTHON_OPT" CXX="$cxxCompiler" CC="$cCompiler" \
                --prefix="$PYTHON_PREFIX_DIR" --disable-ipv6
    else
        info "Configuring Python : ./configure OPT=\"$PYTHON_OPT\" CXX=\"$cxxCompiler\" CC=\"$cCompiler\"" \
             "LDFLAGS=\"$PYTHON_LDFLAGS\""\
             "${PYTHON_SHARED} --prefix=\"$PYTHON_PREFIX_DIR\" --disable-ipv6"
        ./configure OPT="$PYTHON_OPT" CXX="$cxxCompiler" CC="$cCompiler" LDFLAGS="$PYTHON_LDFLAGS" \
                  ${PYTHON_SHARED} \
                --prefix="$PYTHON_PREFIX_DIR" --disable-ipv6
    fi
    if [[ $? != 0 ]] ; then
       warn "Python configure failed.  Giving up"
       return 1
    fi

    #
    # Build Python.
    #
    info "Building Python . . . (~3 minutes)"
    $MAKE $MAKE_OPT_FLAGS
    if [[ $? != 0 ]] ; then
       warn "Python build failed.  Giving up"
       return 1
    fi
    info "Installing Python . . ."
    $MAKE install
    if [[ $? != 0 ]] ; then
       warn "Python build (make install) failed.  Giving up"
       return 1
    fi


    if [[ "$DO_STATIC_BUILD" == "no" && "$OPSYS" == "AIX" ]]; then
        # configure flag --enable-shared doesn't work on llnl aix5 systems
        # we need to create the shared lib manually and place it in the
        # proper loc
        mv $VISITDIR/python/$PYTHON_VERSION/$VISITARCH/lib/libpython$PYTHON_COMPATIBILITY_VERSION.$SO_EXT \
           $VISITDIR/python/$PYTHON_VERSION/$VISITARCH/lib/libpython$PYTHON_COMPATIBILITY_VERSION.static.a

        $C_COMPILER -qmkshrobj -lm \
             $VISITDIR/python/$PYTHON_VERSION/$VISITARCH/lib/libpython$PYTHON_COMPATIBILITY_VERSION.static.a \
          -o $VISITDIR/python/$PYTHON_VERSION/$VISITARCH/lib/libpython$PYTHON_COMPATIBILITY_VERSION.$SO_EXT

        if [[ $? != 0 ]] ; then
          warn "Python dynamic library build failed.  Giving up"
          return 1
        fi

        # we can safely remove this version of the static lib b/c it also exists under python2.6/config/
        rm -f $VISITDIR/python/$PYTHON_VERSION/$VISITARCH/lib/libpython$PYTHON_COMPATIBILITY_VERSION.static.a
    fi


    if [[ "$DO_GROUP" == "yes" ]] ; then
       chmod -R ug+w,a+rX "$VISITDIR/python"
       chgrp -R ${GROUP} "$VISITDIR/python"
    fi
    cd "$START_DIR"
    info "Done with Python"

    return 0
}


# *************************************************************************** #
# The PIL module's detection logic doesn't include /usr/lib64/                #
# On some systems zlib & libjpeg only exist in /usr/lib64, so we patch the    #
# module build script to add /usr/lib64.
# *************************************************************************** #
function apply_python_pil_patch
{
   info "Patching PIL: Add /usr/lib64/ to lib search path."
   patch -f -p0 << \EOF
diff -c Imaging-1.1.6.orig/setup.py Imaging-1.1.6/setup.py
*** Imaging-1.1.6.orig/setup.py Sun Dec  3 03:37:29 2006
--- Imaging-1.1.6/setup.py      Tue Dec 14 13:39:39 2010
***************
*** 196,201 ****
--- 196,205 ----
          add_directory(library_dirs, "/usr/local/lib")
          add_directory(include_dirs, "/usr/local/include")

+         add_directory(library_dirs, "/usr/lib/x86_64-linux-gnu")
+         add_directory(library_dirs, "/usr/lib64")
+         add_directory(include_dirs, "/usr/include")
+
          add_directory(library_dirs, "/usr/lib")
          add_directory(include_dirs, "/usr/include")
EOF
   if [[ $? != 0 ]] ; then
      warn "Python PIL patch adding /usr/lib64/ to lib search path failed."
      return 1
   fi

   return 0
}

# *************************************************************************** #
#                            Function 7.1, build_pil                          #
# *************************************************************************** #
function build_pil
{
    if ! test -f ${PIL_FILE} ; then
        download_file ${PIL_FILE} \
            "${PIL_URL}"
        if [[ $? != 0 ]] ; then
            warn "Could not download ${PIL_FILE}"
            return 1
        fi
    fi
    if ! test -d ${PIL_BUILD_DIR} ; then
        info "Extracting PIL ..."
        uncompress_untar ${PIL_FILE}
        if test $? -ne 0 ; then
            warn "Could not extract ${PIL_FILE}"
            return 1
        fi
    fi

    # apply PIL patches
    apply_python_pil_patch
    if [[ $? != 0 ]] ; then
            warn "Patch failed, but continuing."
    fi

    # NOTE:
    # we need to compose both XFLAGS and X_OPT_FLAGS to get the correct
    # settings from build_visit command line opts
    # see:https://visitbugs.ornl.gov/issues/1443
    #

    PYEXT_CFLAGS="${CFLAGS} ${C_OPT_FLAGS}"
    PYEXT_CXXFLAGS="${CXXFLAGS} ${CXX_OPT_FLAGS}"

    PYHOME="${VISITDIR}/python/${PYTHON_VERSION}/${VISITARCH}"
    pushd $PIL_BUILD_DIR > /dev/null
        info "Building PIL ...\n" \
        "CC=${C_COMPILER} CXX=${CXX_COMPILER} CFLAGS=${PYEXT_CFLAGS} CXXFLAGS=${PYEXT_CXXFLAGS}" \
        "  ${PYHOME}/bin/python ./setup.py build "
        CC=${C_COMPILER} CXX=${CXX_COMPILER} CFLAGS=${PYEXT_CFLAGS} CXXFLAGS=${PYEXT_CXXFLAGS} \
            ${PYHOME}/bin/python ./setup.py build 
        info "Installing PIL ..."
        ${PYHOME}/bin/python ./setup.py install --prefix="${PYHOME}"
    popd > /dev/null

    # PIL installs into site-packages dir of Visit's Python.
    # Simply re-execute the python perms command.
    if [[ "$DO_GROUP" == "yes" ]] ; then
       chmod -R ug+w,a+rX "$VISITDIR/python"
       chgrp -R ${GROUP} "$VISITDIR/python"
    fi

    info "Done with PIL."
    return 0
}

# *************************************************************************** #
#                            Function 7.2, build_pyparsing                    #
# *************************************************************************** #
function build_pyparsing
{
    if ! test -f ${PYPARSING_FILE} ; then
        download_file ${PYPARSING_FILE}
        if [[ $? != 0 ]] ; then
            warn "Could not download ${PYPARSING_FILE}"
            return 1
        fi
    fi
    if ! test -d ${PYPARSING_BUILD_DIR} ; then
        info "Extracting pyparsing ..."
        uncompress_untar ${PYPARSING_FILE}
        if test $? -ne 0 ; then
            warn "Could not extract ${PYPARSING_FILE}"
            return 1
        fi
    fi

    PYHOME="${VISITDIR}/python/${PYTHON_VERSION}/${VISITARCH}"
    pushd $PYPARSING_BUILD_DIR > /dev/null
        info "Installing pyparsing ..."
        ${PYHOME}/bin/python ./setup.py install --prefix="${PYHOME}"
    popd > /dev/null

    # pyparsing installs into site-packages dir of Visit's Python.
    # Simply re-execute the python perms command.
    if [[ "$DO_GROUP" == "yes" ]] ; then
       chmod -R ug+w,a+rX "$VISITDIR/python"
       chgrp -R ${GROUP} "$VISITDIR/python"
    fi

    info "Done with pyparsing."
    return 0
}

# *************************************************************************** #
#                            Function 7.3, build_requests                     #
# *************************************************************************** #
function build_requests
{
    if ! test -f ${PYREQUESTS_FILE} ; then
        download_file ${PYREQUESTS_FILE}
        if [[ $? != 0 ]] ; then
            warn "Could not download ${PYREQUESTS_FILE}"
            return 1
        fi
    fi
    if ! test -d ${PYREQUESTS_BUILD_DIR} ; then
        info "Extracting python requests module ..."
        uncompress_untar ${PYREQUESTS_FILE}
        if test $? -ne 0 ; then
            warn "Could not extract ${PYREQUESTS_FILE}"
            return 1
        fi
    fi

    PYHOME="${VISITDIR}/python/${PYTHON_VERSION}/${VISITARCH}"
    pushd $PYREQUESTS_BUILD_DIR > /dev/null
        info "Installing python requests module ..."
        ${PYHOME}/bin/python ./setup.py install --prefix="${PYHOME}"
    popd > /dev/null

    # installs into site-packages dir of VisIt's Python.
    # Simply re-execute the python perms command.
    if [[ "$DO_GROUP" == "yes" ]] ; then
       chmod -R ug+w,a+rX "$VISITDIR/python"
       chgrp -R ${GROUP} "$VISITDIR/python"
    fi

    info "Done with python requests module."
    return 0
}

# *************************************************************************** #
#                            Function 7.4, build_seedme                       #
# *************************************************************************** #
function build_seedme
{
    if ! test -f ${SEEDME_FILE} ; then
        download_file ${SEEDME_FILE}
        if [[ $? != 0 ]] ; then
            warn "Could not download ${SEEDME_FILE}"
            return 1
        fi
    fi
    if ! test -d ${SEEDME_BUILD_DIR} ; then
        info "Extracting seedme python module ..."
        uncompress_untar ${SEEDME_FILE}
        if test $? -ne 0 ; then
            warn "Could not extract ${SEEDME_BUILD_DIR}"
            return 1
        fi
    fi

    PYHOME="${VISITDIR}/python/${PYTHON_VERSION}/${VISITARCH}"
    pushd $SEEDME_BUILD_DIR > /dev/null
        info "Installing python requests module ..."
        ${PYHOME}/bin/python ./setup.py install --prefix="${PYHOME}"
    popd > /dev/null

    # installs into site-packages dir of VisIt's Python.
    # Simply re-execute the python perms command.
    if [[ "$DO_GROUP" == "yes" ]] ; then
       chmod -R ug+w,a+rX "$VISITDIR/python"
       chgrp -R ${GROUP} "$VISITDIR/python"
    fi

    info "Done with seedme python module."
    return 0
}

function bv_python_is_enabled
{
    if [[ $DO_PYTHON == "yes" ]]; then
        return 1    
    fi
    return 0
}

function bv_python_is_installed
{
    if [[ $USE_SYSTEM_PYTHON == "yes" ]]; then
        return 1
    fi
    check_if_installed "python" $PYTHON_VERSION
    if [[ $? == 0 ]] ; then
        return 1
    fi
    return 0
}

function bv_python_build
{
#
# Build Python
#
cd "$START_DIR"
if [[ "$DO_PYTHON" == "yes" && "$USE_SYSTEM_PYTHON" == "no" ]] ; then
    check_if_installed "python" $PYTHON_VERSION
    if [[ $? == 0 ]] ; then
        info "Skipping Python build.  Python is already installed."
    else
        info "Building Python (~3 minutes)"
        build_python
        if [[ $? != 0 ]] ; then
            error "Unable to build or install Python.  Bailing out."
        fi
        info "Done building Python"

        info "Building the Python Imaging Library"
        build_pil
        if [[ $? != 0 ]] ; then
            warn "PIL build failed."
        fi
        info "Done building the Python Imaging Library"

        build_pyparsing
        if [[ $? != 0 ]] ; then
            warn "pyparsing build failed."
        fi
        info "Done building the pyparsing module."

        build_requests
        if [[ $? != 0 ]] ; then
            warn "requests python module build failed."
        fi
        info "Done building the requests python module."

        build_seedme
        if [[ $? != 0 ]] ; then
            warn "seedme python module build failed."
        fi
        info "Done building the requests seedme python module."

    fi
fi
}

