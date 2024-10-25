# *************************************************************************** #
# Function: check_if_py_module_installed                                      #
#                                                                             #
# Purpose: Checks if givne .                       #
#                                                                             #
# Programmer: Cyrus Harrison                                                  #
# Date: Wed Apr 29 15:29:04 PDT 2020                                          #
#                                                                             #
# *************************************************************************** #
function check_if_py_module_installed
{
    MOD_NAME=$1

    PYHOME=${VISIT_PYTHON_DIR}
    PYTHON_COMMAND="${VISIT_PYTHON_DIR}/bin/python3"

    echo "import ${MOD_NAME}; print(${MOD_NAME})" | ${PYTHON_COMMAND}

    if [[ $? != 0 ]] ; then
        return 1
    fi

    return 0
}

function download_py_module
{
    MOD_FILE=$1
    MOD_URL=$2
 
    if ! test -f ${MOD_FILE} ; then
        download_file ${MOD_FILE} "${MOD_URL}"
        if [[ $? != 0 ]] ; then
            warn "Could not download ${MOD_FILE}"
            return 1
        fi
    fi

    return 0
}

function extract_py_module
{
    MOD_DIR=$1
    MOD_FILE=$2
    MOD_NAME=$3

    if ! test -d ${MOD_DIR} ; then
        info "Extracting python ${MOD_NAME} module (file ${MOD_FILE} to dir ${MOD_DIR} ) ..."
        uncompress_untar ${MOD_FILE}
        if test $? -ne 0 ; then
            warn "Could not extract ${MOD_FILE}"
            return 1
        fi
    fi

    return 0
}

function install_py_module
{
    MOD_DIR=$1
    MOD_NAME=$2

    pushd ${MOD_DIR} > /dev/null
    info "Installing ${MOD_NAME} ..."

    echo ${PYTHON_COMMAND} -m pip --no-cache-dir --disable-pip-version-check install --no-index --no-deps --no-build-isolation .
    ${PYTHON_COMMAND} -m pip --no-cache-dir --disable-pip-version-check install --no-index --no-deps --no-build-isolation .

    if test $? -ne 0 ; then
        popd > /dev/null
        warn "Could not install ${MOD_NAME}"
        return 1
    fi
    popd > /dev/null

    return 0
}

function fix_py_permissions
{
    if [[ "$DO_GROUP" == "yes" ]] ; then
        chmod -R ug+w,a+rX "$VISITDIR/python"
        chgrp -R ${GROUP} "$VISITDIR/python"
    fi
    return 0
}


function bv_python_initialize
{
    export DO_PYTHON="yes"
    export USE_SYSTEM_PYTHON="no"
    export PY_BUILD_MPI4PY="no"
    export PY_BUILD_SPHINX="yes"
    export VISIT_PYTHON_DIR=${VISIT_PYTHON_DIR:-""}
    add_extra_commandline_args "python" "system-python" 0 "Using system python"
    add_extra_commandline_args "python" "alt-python-dir" 1 "Using alternate python directory"
    add_extra_commandline_args "python" "mpi4py" 0 "Build mpi4py"
    add_extra_commandline_args "python" "no-sphinx" 0 "Disable building sphinx"
}

function bv_python_enable
{
    DO_PYTHON="yes"
}

function bv_python_disable
{
    DO_PYTHON="no"
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
    PYTHON_VERSION_MINOR=`echo $PYTHON_VERSION | cut -d. -f2`
    if [[ $PYTHON_VERSION_MINOR -ge 8 ]] ; then
        PYTHON_LIBRARY=`"$PYTHON_CONFIG_COMMAND" --libs --embed`
    else
        PYTHON_LIBRARY=`"$PYTHON_CONFIG_COMMAND" --libs`
    fi
    #remove all other libraries except for python..
    PYTHON_LIBRARY=`echo $PYTHON_LIBRARY | sed "s/.*\(python[^ ]*\).*/\1/g"`

    if [[ "$DO_STATIC_BUILD" == "yes" ]]; then
        PYTHON_LIBRARY="lib${PYTHON_LIBRARY}.a"
    else
          if [[ "$OPSYS" == "Darwin" ]]; then
              PYTHON_LIBRARY="lib${PYTHON_LIBRARY}.dylib"
          else
              PYTHON_LIBRARY="lib${PYTHON_LIBRARY}.so"
          fi
    fi
    #
    # use python's distutils info to get the proper library directory.
    #
    PYTHON_LIBRARY_DIR=`"$PYTHON_COMMAND" -c "import sys;from distutils.sysconfig import get_config_var; sys.stdout.write(get_config_var('LIBDIR'))"`
    if [ ! -e "${PYTHON_LIBRARY_DIR}/${PYTHON_LIBRARY}" ]
    then
        # some systems eg fedora use lib64...
        PYTHON_LIBRARY_DIR="${VISIT_PYTHON_DIR}/lib64"
        if [ ! -e "${PYTHON_LIBRARY_DIR}/${PYTHON_LIBRARY}" ]
        then
            # some systems eg debian use x86_64-linux-gnu...
            PYTHON_LIBRARY_DIR="${VISIT_PYTHON_DIR}/lib/x86_64-linux-gnu"
            if [ ! -e "${PYTHON_LIBRARY_DIR}/${PYTHON_LIBRARY}" ]
            then
                error "python library was not found, cannot configure python"
            fi
        fi
    fi
    PYTHON_LIBRARY="${PYTHON_LIBRARY_DIR}/${PYTHON_LIBRARY}"
    echo $PYTHON_BUILD_DIR $PYTHON_VERSION $VISIT_PYTHON_DIR

}

function bv_python_system_python
{
    echo "Using system python"

    # this method uses 'which' to find the full path to system python and it's config command
    TEST=`which python3-config`
    if [ $? == 0 ]
    then 
        PYTHON_COMMAND=`which python3`
        PYTHON_CONFIG_COMMAND=$TEST
    else
        TEST=`which python-config`
        [ $? != 0 ] && error "Neither system python3-config nor python-config found, cannot configure python"
        PYTHON_COMMAND=`which python`
        PYTHON_CONFIG_COMMAND=$TEST
    fi

    USE_SYSTEM_PYTHON="yes"
    bv_python_enable
    PYTHON_FILE=""
    python_set_vars_helper #set vars..
}

function bv_python_mpi4py
{
    echo "configuring for building mpi4py"
    export PY_BUILD_MPI4PY="yes"
}


function bv_python_no_sphinx
{
    echo "Disabling building sphinx"
    export PY_BUILD_SPHINX="no"
}

function bv_python_alt_python_dir
{
    echo "Using alternate python directory"

    if [ -e "$1/bin/python3-config" ]
    then
        PYTHON_COMMAND="$1/bin/python3"
        PYTHON_CONFIG_COMMAND="$1/bin/python3-config"
    elif [ -e "$1/bin/python-config" ]
    then
        PYTHON_COMMAND="$1/bin/python"
        PYTHON_CONFIG_COMMAND="$1/bin/python-config"
    else
        error "Python (python3-config or python-config) not found in $1"
    fi

    bv_python_enable
    USE_SYSTEM_PYTHON="yes"
    PYTHON_ALT_DIR="$1"
    PYTHON_FILE=""
    python_set_vars_helper #set vars..
}


function bv_python_depends_on
{
     pydep=""
     if [[ $USE_SYSTEM_PYTHON == "no" ]] ; then
        pydep="zlib"
     fi
     echo $pydep
}

function bv_python_info
{
    info "bv_python_info"

    # python 3.9
    export PYTHON_FILE_SUFFIX="tgz"
    export PYTHON_VERSION="3.9.18"
    export PYTHON_COMPATIBILITY_VERSION="3.9"
    export PYTHON_FILE="Python-$PYTHON_VERSION.$PYTHON_FILE_SUFFIX"
    export PYTHON_BUILD_DIR="Python-$PYTHON_VERSION"
    export PYTHON_SHA256_CHECKSUM="504ce8cfd59addc04c22f590377c6be454ae7406cb1ebf6f5a350149225a9354"

    export PY_SETUPTOOLS_FILE="setuptools-68.0.0.tar.gz"
    export PY_SETUPTOOLS_BUILD_DIR="setuptools-68.0.0"
    export PY_SETUPTOOLS_SHA256_CHECKSUM=""

    export PY_PILLOW_FILE="Pillow-10.0.0.tar.gz"
    export PY_PILLOW_BUILD_DIR="Pillow-10.0.0"
    export PY_PILLOW_SHA256_CHECKSUM=""

    export PY_REQUESTS_FILE="requests-2.31.0.tar.gz"
    export PY_REQUESTS_BUILD_DIR="requests-2.31.0"
    export PY_REQUESTS_SHA256_CHECKSUM=""

    export PY_CYTHON_FILE="Cython-3.0.0.tar.gz"
    export PY_CYTHON_BUILD_DIR="Cython-3.0.0"
    export PY_CYTHON_SHA256_CHECKSUM=""

    export PY_NUMPY_FILE="numpy-1.25.1.tar.gz"
    export PY_NUMPY_BUILD_DIR="numpy-1.25.1"
    export PY_NUMPY_SHA256_CHECKSUM=""

    export PY_MPI4PY_FILE="mpi4py-3.1.4.tar.gz"
    export PY_MPI4PY_BUILD_DIR="mpi4py-3.1.4"
    export PY_MPI4PY_SHA256_CHECKSUM=""

    export PY_PACKAGING_FILE="packaging-23.1.tar.gz"
    export PY_PACKAGING_BUILD_DIR="packaging-23.1"
    export PY_PACKAGING_SHA256_CHECKSUM=""

    export PY_IMAGESIZE_FILE="imagesize-1.4.1.tar.gz"
    export PY_IMAGESIZE_BUILD_DIR="imagesize-1.4.1"
    export PY_IMAGESIZE_SHA256_CHECKSUM=""

    export PY_ALABASTER_FILE="alabaster-0.7.13.tar.gz"
    export PY_ALABASTER_BUILD_DIR="alabaster-0.7.13"
    export PY_ALABASTER_SHA256_CHECKSUM=""

    export PY_BABEL_FILE="Babel-2.12.1.tar.gz"
    export PY_BABEL_BUILD_DIR="Babel-2.12.1"
    export PY_BABEL_SHA256_CHECKSUM=""

    export PY_SNOWBALLSTEMMER_FILE="snowballstemmer-2.2.0.tar.gz"
    export PY_SNOWBALLSTEMMER_BUILD_DIR="snowballstemmer-2.2.0"
    export PY_SNOWBALLSTEMMER_SHA256_CHECKSUM=""

    export PY_DOCUTILS_FILE="docutils-0.18.1.tar.gz"
    export PY_DOCUTILS_BUILD_DIR="docutils-0.18.1"
    export PY_DOCUTILS_SHA256_CHECKSUM=""

    export PY_PYGMENTS_FILE="Pygments-2.15.1.tar.gz"
    export PY_PYGMENTS_BUILD_DIR="Pygments-2.15.1"
    export PY_PYGMENTS_SHA256_CHECKSUM=""

    export PY_JINJA2_FILE="Jinja2-3.1.2.tar.gz"
    export PY_JINJA2_BUILD_DIR="Jinja2-3.1.2"
    export PY_JINJA2_SHA256_CHECKSUM=""

    export PY_SPHINXCONTRIB_QTHELP_FILE="sphinxcontrib-qthelp-1.0.3.tar.gz"
    export PY_SPHINXCONTRIB_QTHELP_BUILD_DIR="sphinxcontrib-qthelp-1.0.3"
    export PY_SPHINXCONTRIB_QTHELP_SHA256_CHECKSUM=""

    export PY_SPHINXCONTRIB_SERIALIZINGHTML_FILE="sphinxcontrib-serializinghtml-1.1.5.tar.gz"
    export PY_SPHINXCONTRIB_SERIALIZINGHTML_BUILD_DIR="sphinxcontrib-serializinghtml-1.1.5"
    export PY_SPHINXCONTRIB_SERIALIZINGHTML_SHA256_CHECKSUM=""

    export PY_SPHINXCONTRIB_HTMLHELP_FILE="sphinxcontrib-htmlhelp-2.0.1.tar.gz"
    export PY_SPHINXCONTRIB_HTMLHELP_BUILD_DIR="sphinxcontrib-htmlhelp-2.0.1"
    export PY_SPHINXCONTRIB_HTMLHELP_SHA256_CHECKSUM=""

    export PY_SPHINXCONTRIB_JSMATH_FILE="sphinxcontrib-jsmath-1.0.1.tar.gz"
    export PY_SPHINXCONTRIB_JSMATH_BUILD_DIR="sphinxcontrib-jsmath-1.0.1"
    export PY_SPHINXCONTRIB_JSMATH_SHA256_CHECKSUM="a9925e4a4587247ed2191a22df5f6970656cb8ca2bd6284309578f2153e0c4b8"

    export PY_SPHINXCONTRIB_DEVHELP_FILE="sphinxcontrib-devhelp-1.0.2.tar.gz"
    export PY_SPHINXCONTRIB_DEVHELP_BUILD_DIR="sphinxcontrib-devhelp-1.0.2"
    export PY_SPHINXCONTRIB_DEVHELP_SHA256_CHECKSUM=""

    export PY_SPHINXCONTRIB_APPLEHELP_FILE="sphinxcontrib-applehelp-1.0.4.tar.gz"
    export PY_SPHINXCONTRIB_APPLEHELP_BUILD_DIR="sphinxcontrib-applehelp-1.0.4"
    export PY_SPHINXCONTRIB_APPLEHELP_SHA256_CHECKSUM=""

    export PY_URLLIB3_FILE="urllib3-2.0.3.tar.gz"
    export PY_URLLIB3_BUILD_DIR="urllib3-2.0.3"
    export PY_URLLIB3_SHA256_CHECKSUM=""

    export PY_IDNA_FILE="idna-3.4.tar.gz"
    export PY_IDNA_BUILD_DIR="idna-3.4"
    export PY_IDNA_SHA256_CHECKSUM=""
 
    export PY_CHARSET_NORMALIZER_FILE="charset-normalizer-3.2.0.tar.gz"
    export PY_CHARSET_NORMALIZER_BUILD_DIR="charset-normalizer-3.2.0"
    export PY_CHARSET_NORMALIZER_SHA256_CHECKSUM=""

    export PY_CERTIFI_FILE="certifi-2023.5.7.tar.gz"
    export PY_CERTIFI_BUILD_DIR="certifi-2023.5.7"
    export PY_CERTIFI_SHA256_CHECKSUM=""

    export PY_FLITCORE_FILE="flit_core-3.9.0.tar.gz"
    export PY_FLITCORE_BUILD_DIR="flit_core-3.9.0"
    export PY_FLITCORE_SHA256_CHECKSUM=""

    export PY_TOML_FILE="toml-0.10.2.tar.gz"
    export PY_TOML_BUILD_DIR="toml-0.10.2"
    export PY_TOML_SHA256_CHECKSUM=""

    # and yes, this is a different one from toml!
    export PY_TOMLI_FILE="tomli-2.0.1.tar.gz"
    export PY_TOMLI_BUILD_DIR="tomli-2.0.1"
    export PY_TOMLI_SHA256_CHECKSUM=""

    export PY_PATHSPEC_FILE="pathspec-0.11.2.tar.gz"
    export PY_PATHSPEC_BUILD_DIR="pathspec-0.11.2"
    export PY_PATHSPEC_SHA256_CHECKSUM=""

    export PY_WHEEL_FILE="wheel-0.41.1.tar.gz"
    export PY_WHEEL_BUILD_DIR="wheel-0.41.1"
    export PY_WHEEL_SHA256_CHECKSUM=""

    export PY_CALVER_FILE="calver-2022.6.26.tar.gz"
    export PY_CALVER_BUILD_DIR="calver-2022.6.26"
    export PY_CALVER_SHA256_CHECKSUM=""

    export PY_TROVECLASSIFIERS_FILE="trove-classifiers-2023.8.7.tar.gz"
    export PY_TROVECLASSIFIERS_BUILD_DIR="trove-classifiers-2023.8.7"
    export PY_TROVECLASSIFIERS_SHA256_CHECKSUM=""

    export PY_EDITABLES_FILE="editables-0.5.tar.gz"
    export PY_EDITABLES_BUILD_DIR="editables-0.5"
    export PY_EDITABLES_SHA256_CHECKSUM=""

    export PY_PLUGGY_FILE="pluggy-1.2.0.tar.gz"
    export PY_PLUGGY_BUILD_DIR="pluggy-1.2.0"
    export PY_PLUGGY_SHA256_CHECKSUM=""

    export PY_HATCHLING_FILE="hatchling-1.18.0.tar.gz"
    export PY_HATCHLING_BUILD_DIR="hatchling-1.18.0"
    export PY_HATCHLING_SHA256_CHECKSUM=""

    export PY_MARKUPSAFE_FILE="MarkupSafe-2.1.3.tar.gz"
    export PY_MARKUPSAFE_BUILD_DIR="MarkupSafe-2.1.3"
    export PY_MARKUPSAFE_SHA256_CHECKSUM=""

    export PY_ZIPP_FILE="zipp-3.16.2.tar.gz"
    export PY_ZIPP_BUILD_DIR="zipp-3.16.2"
    export PY_ZIPP_SHA256_CHECKSUM=""

    export PY_IMPORTLIB_METADATA_URL=""
    export PY_IMPORTLIB_METADATA_FILE="importlib_metadata-6.8.0.tar.gz"
    export PY_IMPORTLIB_METADATA_BUILD_DIR="importlib_metadata-6.8.0"
    export PY_IMPORTLIB_METADATA_SHA256_CHECKSUM=""

    export PY_SPHINX_FILE="Sphinx-7.0.1.tar.gz"
    export PY_SPHINX_BUILD_DIR="Sphinx-7.0.1"
    export PY_SPHINX_SHA256_CHECKSUM=""

    export PY_SPHINX_RTD_THEME_FILE="sphinx_rtd_theme-1.2.2.tar.gz"
    export PY_SPHINX_RTD_THEME_BUILD_DIR="sphinx_rtd_theme-1.2.2"
    export PY_SPHINX_RTD_THEME_SHA256_CHECKSUM=""

    # needed by sphinx_rtd_theme
    export PY_SPHINXCONTRIB_JQUERY_FILE="sphinxcontrib-jquery-4.1.tar.gz"
    export PY_SPHINXCONTRIB_JQUERY_BUILD_DIR="sphinxcontrib-jquery-4.1"
    export PY_SPHINXCONTRIB_JQUERY_SHA256_CHECKSUM=""

    export PY_SPHINX_TABS_FILE="sphinx-tabs-3.4.1.tar.gz"
    export PY_SPHINX_TABS_BUILD_DIR="sphinx-tabs-3.4.1"
    export PY_SPHINX_TABS_SHA256_CHECKSUM=""

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
    printf "%-20s %s\n" "--python" "Build Python"
    printf "%-20s %s [%s]\n" "--system-python" "Use the system installed Python"
    printf "%-20s %s [%s]\n" "--alt-python-dir" "Use Python from an alternative directory"
    printf "%-20s %s [%s]\n" "--mpi4py" "Build mpi4py with Python"
    printf "%-20s %s [%s]\n" "--no-sphinx" "Disable building sphinx"
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

        export PYHOME=${VISIT_PYTHON_DIR}
        export PYTHON_COMMAND="${VISIT_PYTHON_DIR}/bin/python3"
        # CYRUS NOTE: PYTHON_LIBRARY_DIR looks wrong?
        # export PYTHON_LIBRARY_DIR="${VISIT_PYTHON_DIR}/bin/python"
        export PYTHON_INCLUDE_DIR="${VISIT_PYTHON_DIR}/include/python${PYTHON_COMPATIBILITY_VERSION}"
        export PYTHON_LIBRARY="${VISIT_PYTHON_DIR}/lib/libpython${PYTHON_COMPATIBILITY_VERSION}.${SO_EXT}"
    else
        export PYTHON_COMMAND="${PYTHON_COMMAND}"
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


function apply_python_patch
{
    # no patches for 3.9
    return 0
}


# *************************************************************************** #
#                         Function 7, build_python                            #
# *************************************************************************** #

function build_python
{
    prepare_build_dir $PYTHON_BUILD_DIR $PYTHON_FILE
    untarred_python=$?
    # 0, already exists, 1 untarred src, 2 error

    if [[ $untarred_python == -1 ]] ; then
        warn "Unable to prepare Python build directory. Giving Up!"
        return 1
    fi

    #
    # Apply patches
    #
    cd $PYTHON_BUILD_DIR || error "Can't cd to Python build dir."
    apply_python_patch
    if [[ $? != 0 ]] ; then
        if [[ $untarred_python == 1 ]] ; then
            warn "Giving up on Python build because the patch failed."
            return 1
        else
            warn "Patch failed, but continuing.  I believe that this script\n" \
                 "tried to apply a patch to an existing directory that had\n" \
                 "already been patched ... that is, the patch is\n" \
                 "failing harmlessly on a second application."
        fi
    fi

    #
    # Configure Python
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
    PYTHON_CPPFLAGS=""
    PYTHON_PREFIX_DIR="$VISITDIR/python/$PYTHON_VERSION/$VISITARCH"
    if [[ "$DO_STATIC_BUILD" == "no" ]]; then
        PYTHON_SHARED="--enable-shared"
        #
        # python's --enable-shared configure flag doesn't link
        # the exes it builds correctly when installed to a non standard
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

    PY_ZLIB_INCLUDE="$VISITDIR/zlib/$ZLIB_VERSION/$VISITARCH/include"
    PY_ZLIB_LIB="$VISITDIR/zlib/$ZLIB_VERSION/$VISITARCH/lib"
    PYTHON_LDFLAGS="${PYTHON_LDFLAGS} -L${PY_ZLIB_LIB}"
    PYTHON_CPPFLAGS="${PYTHON_CPPFLAGS} -I${PY_ZLIB_INCLUDE}"

    info "Configuring Python"
    set -x
    ./configure OPT="$PYTHON_OPT" CXX="$cxxCompiler" CC="$cCompiler" \
                LDFLAGS="$PYTHON_LDFLAGS" \
                CPPFLAGS="$PYTHON_CPPFLAGS" \
                ${PYTHON_SHARED} \
                --prefix="$PYTHON_PREFIX_DIR" --disable-ipv6
    set +x

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

    cd "$START_DIR"
    info "Done with Python"


    # wheel and its dependencies
    download_py_module ${PY_FLITCORE_FILE} ${PY_FLITCORE_URL}
    if test $? -ne 0 ; then
        return 1
    fi
  
    download_py_module ${PY_WHEEL_FILE} ${PY_WHEEL_URL}
    if [[ $? != 0 ]] ; then
        return 1
    fi

    extract_py_module ${PY_FLITCORE_BUILD_DIR} ${PY_FLITCORE_FILE}  "flit_core"
    if test $? -ne 0 ; then
        return 1
    fi

    extract_py_module ${PY_WHEEL_BUILD_DIR} ${PY_WHEEL_FILE} "wheel"
    if test $? -ne 0 ; then
        return 1
    fi

    install_py_module ${PY_FLITCORE_BUILD_DIR} "flit_core"
    if test $? -ne 0 ; then
        return 1
    fi

    install_py_module ${PY_WHEEL_BUILD_DIR} "wheel"
    if [[ $? != 0 ]] ; then
        return 1
    fi

    # setuptools
    # need the newest version required by a module, not the default
    # version from python
    download_py_module ${PY_SETUPTOOLS_FILE} ${PY_SETUPTOOLS_URL}
    if test $? -ne 0 ; then
        return 1
    fi

    extract_py_module ${PY_SETUPTOOLS_BUILD_DIR} ${PY_SETUPTOOLS_FILE} "setuptools"
    if test $? -ne 0 ; then
        return 1
    fi

    install_py_module ${PY_SETUPTOOLS_BUILD_DIR} "setuptools"
    if test $? -ne 0 ; then
        return 1
    fi

    fix_py_permissions

    info "Done with python setuptools module."
    return 0

}

# *************************************************************************** #
#                            Function 7.1, build_pillow                       #
# *************************************************************************** #
function build_pillow
{
    download_py_module ${PY_PILLOW_FILE} ${PY_PILLOW_URL}
    if [[ $? != 0 ]] ; then
        return 1
    fi

    extract_py_module ${PY_PILLOW_BUILD_DIR} ${PY_PILLOW_FILE} "pillow"
    if [[ $? != 0 ]] ; then
        return 1
    fi

    # Pillow depends on Zlib
    PY_ZLIB_INCLUDE="$VISITDIR/zlib/$ZLIB_VERSION/$VISITARCH/include"
    PY_ZLIB_LIB="$VISITDIR/zlib/$ZLIB_VERSION/$VISITARCH/lib"

    PYEXT_CFLAGS="${CFLAGS} ${C_OPT_FLAGS}"
    PYEXT_CFLAGS="${CFLAGS} ${C_OPT_FLAGS} -I${PY_ZLIB_INCLUDE}"
    PYEXT_CXXFLAGS="${CXXFLAGS} ${CXX_OPT_FLAGS} -I${PY_ZLIB_INCLUDE}"
    PYEXT_LDFLAGS="-L${PY_ZLIB_LIB}"

    if [[ "$OPSYS" == "Darwin" ]]; then
        PYEXT_CFLAGS="${PYEXT_CFLAGS} -I/opt/X11/include"
        PYEXT_CXXFLAGS="${PYEXT_CXXFLAGS} -I/opt/X11/include"
    fi

    pushd $PY_PILLOW_BUILD_DIR > /dev/null

    info "Building Pillow ...\n" \
    set -x
    CC=${C_COMPILER} CXX=${CXX_COMPILER} CFLAGS="${PYEXT_CFLAGS}" \
     CXXFLAGS="${PYEXT_CXXFLAGS}" \
     LDFLAGS="${PYEXT_LDFLAGS}" \
     ${PYTHON_COMMAND} ./setup.py build_ext --disable-webp --disable-webpmux --disable-freetype --disable-lcms --disable-tiff --disable-xcb --disable-jpeg2000 --disable-jpeg install --prefix="${PYHOME}"
    set +x

    if test $? -ne 0 ; then
        popd > /dev/null
        warn "Could not build and install Pillow"
        return 1
    fi
    popd > /dev/null

    # Pillow installs into site-packages dir of Visit's Python.
    # Simply re-execute the python perms command.
    fix_py_permissions

    info "Done with Pillow."
    return 0
}


# *************************************************************************** #
#                            Function 7.3, build_requests                     #
# *************************************************************************** #
function build_requests
{
    download_py_module ${PY_CERTIFI_FILE} ${PY_CERTIFIY_URL}
    if test $? -ne 0 ; then
        return 1
    fi

    download_py_module ${PY_TOML_FILE} ${PY_TOML_URL}
    if test $? -ne 0 ; then
        return 1
    fi

    download_py_module ${PY_TOMLI_FILE} ${PY_TOMLI_URL}
    if test $? -ne 0 ; then
        return 1
    fi

    download_py_module ${PY_PATHSPEC_FILE} ${PY_PATHSPEC_URL}
    if test $? -ne 0 ; then
        return 1
    fi

    download_py_module ${PY_CALVER_FILE} ${PY_TCALVER_URL}
    if test $? -ne 0 ; then
        return 1
    fi

    download_py_module ${PY_TROVECLASSIFIERS_FILE} ${PY_TROVECLASSIFIERS_URL}
    if test $? -ne 0 ; then
        return 1
    fi

    download_py_module ${PY_PACKAGING_FILE} ${PY_PACKAGING_URL}
    if test $? -ne 0 ; then
        return 1
    fi

    download_py_module ${PY_EDITABLES_FILE} ${PY_EDITABLES_URL}
    if test $? -ne 0 ; then
        return 1
    fi

    download_py_module ${PY_PLUGGY_FILE} ${PY_PLUGGY_URL}
    if test $? -ne 0 ; then
        return 1
    fi

    download_py_module ${PY_HATCHLING_FILE} ${PY_HATCHLING_URL}
    if test $? -ne 0 ; then
        return 1
    fi

    download_py_module ${PY_URLLIB3_FILE} ${PY_URLLIB3_URL}
    if test $? -ne 0 ; then
        return 1
    fi

    download_py_module ${PY_IDNA_FILE} ${PY_IDNA_URL}
    if test $? -ne 0 ; then
        return 1
    fi

    download_py_module ${PY_CHARSET_NORMALIZER_FILE} ${PY_CHARSET_NORMALIZER_URL}
    if test $? -ne 0 ; then
        return 1
    fi

    extract_py_module ${PY_CERTIFI_BUILD_DIR} ${PY_CERTIFI_FILE} "certifi"
    if test $? -ne 0 ; then
        return 1
    fi

    extract_py_module ${PY_TOML_BUILD_DIR} ${PY_TOML_FILE} "toml"
    if test $? -ne 0 ; then
        return 1
    fi

    extract_py_module ${PY_TOMLI_BUILD_DIR} ${PY_TOMLI_FILE} "tomli"
    if test $? -ne 0 ; then
        return 1
    fi

    extract_py_module ${PY_PATHSPEC_BUILD_DIR} ${PY_PATHSPEC_FILE} "pathspec"
    if test $? -ne 0 ; then
        return 1
    fi

    extract_py_module ${PY_CALVER_BUILD_DIR} ${PY_CALVER_FILE} "calver"
    if test $? -ne 0 ; then
        return 1
    fi

    extract_py_module ${PY_TROVECLASSIFIERS_BUILD_DIR} ${PY_TROVECLASSIFIERS_FILE} "trove_classifiers"
    if test $? -ne 0 ; then
        return 1
    fi

    extract_py_module ${PY_PACKAGING_BUILD_DIR} ${PY_PACKAGING_FILE} "packaging"
    if test $? -ne 0 ; then
        return 1
    fi

    extract_py_module ${PY_EDITABLES_BUILD_DIR} ${PY_EDITABLES_FILE} "editables"
    if test $? -ne 0 ; then
        return 1
    fi

    extract_py_module ${PY_PLUGGY_BUILD_DIR} ${PY_PLUGGY_FILE} "pluggy"
    if test $? -ne 0 ; then
        return 1
    fi

    extract_py_module ${PY_HATCHLING_BUILD_DIR} ${PY_HATCHLING_FILE} "hatchling"
    if test $? -ne 0 ; then
        return 1
    fi

    extract_py_module ${PY_URLLIB3_BUILD_DIR} ${PY_URLLIB3_FILE} "urllib3"
    if test $? -ne 0 ; then
        return 1
    fi

    extract_py_module ${PY_IDNA_BUILD_DIR} ${PY_IDNA_FILE} "idna"
    if test $? -ne 0 ; then
        return 1
    fi

    extract_py_module ${PY_CHARSET_NORMALIZER_BUILD_DIR} ${PY_CHARSET_NORMALIZER_FILE} "charset-normalizer"
    if test $? -ne 0 ; then
            return 1
    fi

    install_py_module ${PY_CERTIFI_BUILD_DIR} "certifi"
    if test $? -ne 0 ; then
        return 1
    fi

    install_py_module ${PY_TOML_BUILD_DIR} "toml"
    if test $? -ne 0 ; then
          return 1
    fi

    install_py_module ${PY_TOMLI_BUILD_DIR} "tomlI"
    if test $? -ne 0 ; then
          return 1
    fi

    install_py_module ${PY_PATHSPEC_BUILD_DIR} "pathspec"
    if test $? -ne 0 ; then
          return 1
    fi

    install_py_module ${PY_CALVER_BUILD_DIR} "calver"
    if test $? -ne 0 ; then
          return 1
    fi

    install_py_module ${PY_TROVECLASSIFIERS_BUILD_DIR} "trove_classifiers"
    if test $? -ne 0 ; then
          return 1
    fi

    install_py_module ${PY_PACKAGING_BUILD_DIR} "packaging"
    if test $? -ne 0 ; then
          return 1
    fi

    install_py_module ${PY_EDITABLES_BUILD_DIR} "editables"
    if test $? -ne 0 ; then
          return 1
    fi

    install_py_module ${PY_PLUGGY_BUILD_DIR} "pluggy"
    if test $? -ne 0 ; then
        return 1
    fi

    install_py_module ${PY_HATCHLING_BUILD_DIR} "hatchling"
    if test $? -ne 0 ; then
          return 1
    fi

    install_py_module ${PY_URLLIB3_BUILD_DIR} "urllib3"
    if test $? -ne 0 ; then
        return 1
    fi

    install_py_module ${PY_IDNA_BUILD_DIR} "idna"
    if test $? -ne 0 ; then
        return 1
    fi

    install_py_module ${PY_CHARSET_NORMALIZER_BUILD_DIR} "charset-normalizer"
    if test $? -ne 0 ; then
        return 1
    fi

    download_py_module ${PY_REQUESTS_FILE} ${REQUESTS_URL}
    if test $? -ne 0 ; then
        return 1
    fi

    extract_py_module ${PY_REQUESTS_BUILD_DIR} ${REQUESTS_FILE} "requests"
    if test $? -ne 0 ; then
        return 1
    fi

    install_py_module ${PY_REQUESTS_BUILD_DIR} "requests"
    if test $? -ne 0 ; then
        return 1
    fi

    # installs into site-packages dir of VisIt's Python.
    # Simply re-execute the python perms command.
    fix_py_permissions

    info "Done with python requests module."
    return 0
}

# *************************************************************************** #
#                                  build_mpi4py                               #
# *************************************************************************** #
function build_mpi4py
{
    download_py_module ${PY_MPI4PY_FILE} ${PY_MPI4PY_URL}
    if test $? -ne 0 ; then
        return 1
    fi

    extract_py_module ${PY_MPI4PY_BUILD_DIR} ${PY_MPI4PY_FILE} "mpi4py"
    if test $? -ne 0 ; then
        return 1
    fi

    install_py_module ${PY_MPI4PY_BUILD_DIR} "mpi4py"
    if test $? -ne 0 ; then
        return 1
    fi

    fix_py_permissions

    return 0
}

# *************************************************************************** #
#                                  build_numpy                                #
# *************************************************************************** #
function build_numpy
{
    download_py_module ${PY_CYTHON_FILE} ${PY_CYTHON_URL}
    if [[ $? != 0 ]] ; then
        return 1
    fi

    download_py_module ${PY_NUMPY_FILE} ${PY_NUMPY_URL}
    if [[ $? != 0 ]] ; then
        return 1
    fi

    extract_py_module ${PY_CYTHON_BUILD_DIR} ${PY_CYTHON_FILE} "cython"
    if [[ $? != 0 ]] ; then
        return 1
    fi

    extract_py_module ${PY_NUMPY_BUILD_DIR} ${PY_NUMPY_FILE} "numpy"
    if [[ $? != 0 ]] ; then
        return 1
    fi

    install_py_module ${PY_CYTHON_BUILD_DIR} "cython"
    if [[ $? != 0 ]] ; then
        return 1
    fi

    # Disable blas and lapack on macOS but only if user isn't somehow fiddling with them also.
    # https://numpy.org/doc/1.25/user/building.html#disabling-atlas-and-other-accelerated-libraries
    we_set_numpy_lib_vars=0
    if [ "$OPSYS" == "Darwin" ] && [ -z "$NPY_BLAS_ORDER" ] && [ -z "$NPY_LAPACK_ORDER" ]; then
        we_set_numpy_lib_vars=1
        export NPY_BLAS_ORDER= NPY_LAPACK_ORDER=
    fi
    install_py_module ${PY_NUMPY_BUILD_DIR} "numpy"
    return_status=$?
    if [ $we_set_numpy_lib_vars -eq 1 ]; then
        unset NPY_BLAS_ORDER NPY_LAPACK_ORDER
    fi
    if [ $return_status -ne 0 ] ; then
        return 1
    fi

    fix_py_permissions

    return 0
}

# *************************************************************************** #
#                                  build_sphinx                               #
# *************************************************************************** #
function build_sphinx
{
    info "building sphinx"

    download_py_module ${PY_PACKAGING_FILE} ${PY_PACKAGING_URL}
    if test $? -ne 0 ; then
        return 1
    fi

    download_py_module ${PY_IMAGESIZE_FILE} ${PY_IMAGESIZE_URL}
    if test $? -ne 0 ; then
        return 1
    fi

    download_py_module ${PY_ALABASTER_FILE} ${PY_ALABASTER_URL}
    if test $? -ne 0 ; then
        return 1
    fi

    download_py_module ${PY_BABEL_FILE} ${PY_BABEL_URL}
    if test $? -ne 0 ; then
        return 1
    fi

    download_py_module ${PY_SNOWBALLSTEMMER_FILE} ${PY_SNOWBALLSTEMMER_URL}
    if test $? -ne 0 ; then
        return 1
    fi

    download_py_module ${PY_DOCUTILS_FILE} ${PY_DOCUTILS_URL}
    if test $? -ne 0 ; then
        return 1
    fi

    download_py_module ${PY_PYGMENTS_FILE} ${PY_PYGMENTS_URL}
    if test $? -ne 0 ; then
        return 1
    fi

    download_py_module ${PY_JINJA2_FILE} ${PY_JINJA2_URL}
    if test $? -ne 0 ; then
        return 1
    fi

    download_py_module ${PY_SPHINXCONTRIB_QTHELP_FILE} ${PY_SPHINXCONTRIB_QTHELP_URL}
    if test $? -ne 0 ; then
        return 1
    fi

    download_py_module ${PY_SPHINXCONTRIB_SERIALIZINGHTML_FILE} ${PY_SPHINXCONTRIB_SERIALIZINGHTML_URL}
    if test $? -ne 0 ; then
        return 1
    fi

    download_py_module ${PY_SPHINXCONTRIB_HTMLHELP_FILE} ${PY_SPHINXCONTRIB_HTMLHELP_URL}
    if test $? -ne 0 ; then
        return 1
    fi

    download_py_module ${PY_SPHINXCONTRIB_JSMATH_FILE} ${PY_SPHINXCONTRIB_JSMATH_URL}
    if test $? -ne 0 ; then
        return 1
    fi

    download_py_module ${PY_SPHINXCONTRIB_DEVHELP_FILE} ${PY_SPHINXCONTRIB_DEVHELP_URL}
    if test $? -ne 0 ; then
        return 1
    fi

    download_py_module ${PY_SPHINXCONTRIB_APPLEHELP_FILE} ${PY_SPHINXCONTRIB_APPLEHELP_ULR}
    if test $? -ne 0 ; then
        return 1
    fi

    download_py_module ${PY_SIX_FILE} ${PY_SIX_URL}
    if test $? -ne 0 ; then
        return 1
    fi

    download_py_module ${PY_MARKUPSAFE_FILE} ${PY_MARKUPSAFE_URL}
    if test $? -ne 0 ; then
        return 1
    fi

    download_py_module ${PY_ZIPP_FILE} ${PY_ZIPP_URL}
    if test $? -ne 0 ; then
        return 1
    fi

    download_py_module ${PY_IMPORTLIB_METADATA_FILE} ${PY_IMPORTLIB_METADATA_URL}
    if test $? -ne 0 ; then
        return 1
    fi

    download_py_module ${PY_SPHINX_FILE} ${PY_SPHINX_URL}
    if test $? -ne 0 ; then
        return 1
    fi

    extract_py_module ${PY_PACKAGING_BUILD_DIR} ${PY_PACKAGING_FILE} "packaging"
    if test $? -ne 0 ; then
        return 1
    fi

    extract_py_module ${PY_IMAGESIZE_BUILD_DIR} ${PY_IMAGESIZE_FILE} "imagesize"
    if test $? -ne 0 ; then
        return 1
    fi

    extract_py_module ${PY_ALABASTER_BUILD_DIR} ${PY_ALABASTER_FILE} "alabaster"
    if test $? -ne 0 ; then
        return 1
    fi

    extract_py_module ${PY_BABEL_BUILD_DIR} ${PY_BABEL_FILE} "babel"
    if test $? -ne 0 ; then
        return 1
    fi

    extract_py_module ${PY_SNOWBALLSTEMMER_BUILD_DIR} ${PY_SNOWBALLSTEMMER_FILE} "snowballstemmer"
    if test $? -ne 0 ; then
        return 1
    fi

    extract_py_module ${PY_DOCUTILS_BUILD_DIR} ${PY_DOCUTILS_FILE} "docutils"
    if test $? -ne 0 ; then
        return 1
    fi

    extract_py_module ${PY_PYGMENTS_BUILD_DIR} ${PY_PYGMENTS_FILE} "pygments"
    if test $? -ne 0 ; then
        return 1
    fi

    extract_py_module ${PY_JINJA2_BUILD_DIR} ${PY_JINJA2_FILE} "jinja2"
    if test $? -ne 0 ; then
        return 1
    fi

    extract_py_module ${PY_SPHINXCONTRIB_QTHELP_BUILD_DIR} ${PY_SPHINXCONTRIB_QTHELP_FILE} "sphinxcontrib-qthelp"
    if test $? -ne 0 ; then
        return 1
    fi

    extract_py_module ${PY_SPHINXCONTRIB_SERIALIZINGHTML_BUILD_DIR} ${PY_SPHINXCONTRIB_SERIALIZINGHTML_FILE} "sphinxcontrib-serializinghtml"
    if test $? -ne 0 ; then
        return 1
    fi

    extract_py_module ${PY_SPHINXCONTRIB_HTMLHELP_BUILD_DIR} ${PY_SPHINXCONTRIB_HTMLHELP_FILE} "sphinxcontrib-htmlhelp"
    if test $? -ne 0 ; then
        return 1
    fi

    extract_py_module ${PY_SPHINXCONTRIB_JSMATH_BUILD_DIR} ${PY_SPHINXCONTRIB_JSMATH_FILE} "sphinxcontrib-jsmath"
    if test $? -ne 0 ; then
        return 1
    fi

    extract_py_module ${PY_SPHINXCONTRIB_DEVHELP_BUILD_DIR} ${PY_SPHINXCONTRIB_DEVHELP_FILE} "sphinxcontrib-devhelp"
    if test $? -ne 0 ; then
        return 1
    fi

    extract_py_module ${PY_SPHINXCONTRIB_APPLEHELP_BUILD_DIR} ${PY_SPHINXCONTRIB_APPLEHELP_FILE} "sphinxcontrib-applehelp"
    if test $? -ne 0 ; then
        return 1
    fi

    extract_py_module ${PY_MARKUPSAFE_BUILD_DIR} ${PY_MARKUPSAFE_FILE} "markupsafe"
    if test $? -ne 0 ; then
        return 1
    fi

    extract_py_module ${PY_ZIPP_BUILD_DIR} ${PY_ZIPP_FILE} "zipp"
    if test $? -ne 0 ; then
        return 1
    fi

    extract_py_module ${PY_IMPORTLIB_METADATA_BUILD_DIR} ${PY_IMPORTLIB_METADATA_FILE} "importlib-metadata"
    if test $? -ne 0 ; then
        return 1
    fi

    extract_py_module ${PY_SPHINX_BUILD_DIR} ${PY_SPHINX_FILE} "sphinx"
    if test $? -ne 0 ; then
        return 1
    fi

    # patch
    SED_CMD="sed -i "
    if [[ "$OPSYS" == "Darwin" ]]; then
        SED_CMD="sed -i '' " # the intention of this sed command is foiled by shell variable expansion
    fi
    pushd $PY_SPHINX_BUILD_DIR > /dev/null
    ${SED_CMD} "s/docutils>=0.12/docutils<0.16,>=0.12/" ./Sphinx.egg-info/requires.txt
    ${SED_CMD} "s/docutils>=0.12/docutils<0.16,>=0.12/" ./setup.py
    popd > /dev/null

    install_py_module ${PY_PACKAGING_BUILD_DIR} "packaging"
    if test $? -ne 0 ; then
        return 1
    fi

    install_py_module ${PY_IMAGESIZE_BUILD_DIR} "imagesize"
    if test $? -ne 0 ; then
        return 1
    fi

    install_py_module ${PY_ALABASTER_BUILD_DIR} "alabaster"
    if test $? -ne 0 ; then
        return 1
    fi

    install_py_module ${PY_BABEL_BUILD_DIR} "babel"
    if test $? -ne 0 ; then
        return 1
    fi

    install_py_module ${PY_SNOWBALLSTEMMER_BUILD_DIR} "snowballstemmer"
    if test $? -ne 0 ; then
        return 1
    fi

    install_py_module ${PY_DOCUTILS_BUILD_DIR} "docutils"
    if test $? -ne 0 ; then
        return 1
    fi

    install_py_module ${PY_PYGMENTS_BUILD_DIR} "pygments"
    if test $? -ne 0 ; then
        return 1
    fi

    install_py_module ${PY_MARKUPSAFE_BUILD_DIR} "markupsafe"
    if test $? -ne 0 ; then
        return 1
    fi

    install_py_module ${PY_JINJA2_BUILD_DIR} "jinja2"
    if test $? -ne 0 ; then
        return 1
    fi

    install_py_module ${PY_SPHINXCONTRIB_QTHELP_BUILD_DIR} "sphinxcontrib-qthelp"
    if test $? -ne 0 ; then
        return 1
    fi

    install_py_module ${PY_SPHINXCONTRIB_SERIALIZINGHTML_BUILD_DIR} "sphinxcontrib-serializinghtml"
    if test $? -ne 0 ; then
        return 1
    fi

    install_py_module ${PY_SPHINXCONTRIB_HTMLHELP_BUILD_DIR} "sphinxcontrib-htmlhelp"
    if test $? -ne 0 ; then
        return 1
    fi

    install_py_module ${PY_SPHINXCONTRIB_JSMATH_BUILD_DIR} "sphinxcontrib-jsmath"
    if test $? -ne 0 ; then
        return 1
    fi

    install_py_module ${PY_SPHINXCONTRIB_DEVHELP_BUILD_DIR} "sphinxcontrib-devhelp"
    if test $? -ne 0 ; then
        return 1
    fi

    install_py_module ${PY_SPHINXCONTRIB_APPLEHELP_BUILD_DIR} "sphinxcontrib-applehelp"
    if test $? -ne 0 ; then
        return 1
    fi

    install_py_module ${PY_ZIPP_BUILD_DIR} "zipp"
    if test $? -ne 0 ; then
        return 1
    fi

    install_py_module ${PY_IMPORTLIB_METADATA_BUILD_DIR} "importlib-metadata"
    if test $? -ne 0 ; then
        return 1
    fi

    install_py_module ${PY_SPHINX_BUILD_DIR} "sphinx"
    if test $? -ne 0 ; then
        return 1
    fi

    fix_py_permissions

    # fix shebangs. On Darwin, if python is available in Xcode,
    # Sphinx scripts may get installed with shebangs that are absolute
    # paths to Xcode's python interpreter. We want VisIt's python.
    if [[ "$OPSYS" == "Darwin" ]]; then
        for f in ${VISIT_PYTHON_DIR}/bin/*; do
            if [[ -z "$(file $f | grep -i 'ascii text')" ]]; then
                continue # Process only scripts
            fi
            # -i '' means do in-place...don't create backups
            # 1s means do substitution only on line 1
            # @ choosen as sep char for s sed cmd to not collide w/slashes
            # ! needs to be escaped with a backslash
            # don't use ${SED_CMD}
            sed -i '' -e "1s@^#\!.*\$@#\!${VISIT_PYTHON_DIR}/bin/python3@" $f
        done
    fi

    return 0
}

# *************************************************************************** #
#                              build_sphinx_rtd                               #
# *************************************************************************** #
function build_sphinx_rtd
{
    download_py_module ${PY_SPHINXCONTRIB_JQUERY_FILE} ${PY_SPHINXCONTRIB_JQUERY_URL}
    if [[ $? != 0 ]] ; then
        return 1
    fi

    extract_py_module ${PY_SPHINXCONTRIB_JQUERY_BUILD_DIR} ${PY_SPHINXCONTRIB_JQUERY_FILE} "sphinxcontrib-jquery"
    if [[ $? != 0 ]] ; then
        return 1
    fi

    install_py_module ${PY_SPHINXCONTRIB_JQUERY_BUILD_DIR} "sphinxcontrib-jquery"
    if [[ $? != 0 ]] ; then
        return 1
    fi

    download_py_module ${PY_SPHINX_RTD_THEME_FILE} ${PY_SPHINX_RTD_THEME_URL}
    if [[ $? != 0 ]] ; then
        return 1
    fi

    extract_py_module ${PY_SPHINX_RTD_THEME_BUILD_DIR} ${PY_SPHINX_RTD_THEME_FILE} "sphinx_rtd_theme"
    if [[ $? != 0 ]] ; then
        return 1
    fi

    install_py_module ${PY_SPHINX_RTD_THEME_BUILD_DIR} "sphinx_rtd_theme"
    if [[ $? != 0 ]] ; then
        return 1
    fi

    fix_py_permissions

    return 0
}

# *************************************************************************** #
#                              build_sphinx_tabs                              #
# *************************************************************************** #
function build_sphinx_tabs
{
    download_py_module ${PY_SPHINX_TABS_FILE} ${PY_SPHINX_TABS_URL}
    if [[ $? != 0 ]] ; then
        return 1
    fi

    extract_py_module ${PY_SPHINX_TABS_BUILD_DIR} ${PY_SPHINX_TABS_FILE} "sphinx-tabs"
    if [[ $? != 0 ]] ; then
        return 1
    fi

    install_py_module ${PY_SPHINX_TABS_BUILD_DIR} "sphinx-tabs"
    if [[ $? != 0 ]] ; then
        return 1
    fi

    fix_py_permissions

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
    PY_CHECK_ECHO=0
    if [[ $# == 1 ]]; then
        PY_CHECK_ECHO=$1
    fi
    
    info "checking if python is installed"
    # byo python, assume mods are there
    if [[ $USE_SYSTEM_PYTHON == "yes" ]]; then
        return 1
    fi
    
    check_if_installed "python" $PYTHON_VERSION
    if [[ $? != 0 ]] ; then
        if [[ $PY_CHECK_ECHO != 0 ]] ; then
            info "python is not installed"
        fi
        return 0
    fi

    PY_OK=1

    # we also need to check all the python modules ...
    check_if_py_module_installed "PIL"
    if [[ $? != 0 ]] ; then
        if [[ $PY_CHECK_ECHO != 0 ]] ; then
            info "python module Pillow is not installed"
        fi
        PY_OK=0
    fi

    check_if_py_module_installed "numpy"
    if [[ $? != 0 ]] ; then
        if [[ $PY_CHECK_ECHO != 0 ]] ; then
            info "python module numpy is not installed"
        fi
        PY_OK=0
    fi

    if [[ "$PY_BUILD_SPHINX" == "yes" ]]; then

        check_if_py_module_installed "sphinx"
        if [[ $? != 0 ]] ; then
            if [[ $PY_CHECK_ECHO != 0 ]] ; then
                info "python module sphinx is not installed"
            fi
            PY_OK=0
        fi

        check_if_py_module_installed "sphinx_rtd_theme"
        if [[ $? != 0 ]] ; then
            if [[ $PY_CHECK_ECHO != 0 ]] ; then
                info "python module sphinx_rtd_theme is not installed"
            fi
            PY_OK=0
        fi
        check_if_py_module_installed "sphinx_tabs"
        if [[ $? != 0 ]] ; then
            if [[ $PY_CHECK_ECHO != 0 ]] ; then
                info "python module sphinx_tabs is not installed"
            fi
            PY_OK=0
        fi

    fi

    if [[ "$PY_BUILD_MPI4PY" == "yes" ]]; then

        check_if_py_module_installed "mpi4py"
        if [[ $? != 0 ]] ; then
            if [[ $PY_CHECK_ECHO != 0 ]] ; then
                info "python module mpi4py is not installed"
            fi
            PY_OK=0
        fi
    fi

    return $PY_OK
}

function bv_python_build
{
    #
    # Build Python
    #
    cd "$START_DIR"
    if [[ "$DO_PYTHON" == "yes" && "$USE_SYSTEM_PYTHON" == "no" ]] ; then

        bv_python_is_installed 1
        
        if [[ $? == 1 ]] ; then
            info "Skipping Python build.  Python is already installed."
        else

            # check python proper, then mods
            check_if_installed "python" $PYTHON_VERSION
            if [[ $? != 0 ]] ; then
                info "Building Python (~3 minutes)"
                build_python
                if [[ $? != 0 ]] ; then
                    error "Unable to build or install Python.  Bailing out."
                fi
                info "Done building Python"
            fi

            # Do not build those packages for a static build!
            if [[ "$DO_STATIC_BUILD" == "yes" ]]; then
                return 0
            fi

            # setup PYHOME and PYTHON_COMMAND which is used in our build
            # of these python modules
            export PYHOME="${VISITDIR}/python/${PYTHON_VERSION}/${VISITARCH}"
            export PYTHON_COMMAND="${PYHOME}/bin/python3"

            check_if_py_module_installed "numpy"
            if [[ $? != 0 ]] ; then
                info "Building the numpy module"
                build_numpy
                if [[ $? != 0 ]] ; then
                    error "numpy build failed. Bailing out."
                fi
                info "Done building the numpy module."
            fi

            check_if_py_module_installed "PIL"
            # use Pillow for when python 3
            if [[ $? != 0 ]] ; then
                info "Building the Python Pillow Imaging Library"
                build_pillow
                if [[ $? != 0 ]] ; then
                    error "Pillow build failed. Bailing out."
                fi
                info "Done building the Python Pillow Imaging Library"
            fi

            if [[ "$PY_BUILD_MPI4PY" == "yes" ]]; then

                check_if_py_module_installed "mpi4py"
                if [[ $? != 0 ]] ; then
                    info "Building the mpi4py module"
                    build_mpi4py
                    if [[ $? != 0 ]] ; then
                        error "mpi4py build failed. Bailing out."
                    fi
                    info "Done building the mpi4py module"
                fi
            fi

            if [[ "$PY_BUILD_SPHINX" == "yes" ]]; then
                # requests is needed by sphinx.
                check_if_py_module_installed "requests"
                if [[ $? != 0 ]] ; then
                    build_requests
                    if [[ $? != 0 ]] ; then
                        error "requests python module build failed. Bailing out."
                    fi
                    info "Done building the requests python module."
                fi

                check_if_py_module_installed "sphinx"
                if [[ $? != 0 ]] ; then
                    build_sphinx
                    if [[ $? != 0 ]] ; then
                        error "sphinx python module build failed. Bailing out."
                    fi
                    info "Done building the sphinx python module."
                fi

                check_if_py_module_installed "sphinx_rtd_theme"
                if [[ $? != 0 ]] ; then
                    build_sphinx_rtd
                    if [[ $? != 0 ]] ; then
                        error "sphinx rtd python theme build failed. Bailing out."
                    fi
                    info "Done building the sphinx rtd python theme."
                fi

                check_if_py_module_installed "sphinx_tabs"
                if [[ $? != 0 ]] ; then
                    build_sphinx_tabs
                    if [[ $? != 0 ]] ; then
                        error "sphinx tabs python module build failed. Bailing out."
                    fi
                    info "Done building the sphinx tabs."
                fi
            fi
        fi
    fi
}
