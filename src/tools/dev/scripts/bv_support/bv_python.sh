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
        info "Extracting python ${MOD_NAME} module ..."
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

    if [[ "$DO_PYTHON39" == "yes" ]] ; then
        ${PYTHON_COMMAND} -m pip install --no-deps --no-cache-dir .
    else
        ${PYTHON_COMMAND} ./setup.py install --prefix="${PYHOME}"
    fi

    if test $? -ne 0 ; then
        popd > /dev/null
        warn "Could not install ${MOD_NAME}"
        return 1
    fi
    popd > /dev/null

    return 0
}

function install_py_module_with_setup
{
    MOD_DIR=$1
    MOD_NAME=$2

    pushd ${MOD_DIR} > /dev/null
    info "Installing ${MOD_NAME} ..."

    ${PYTHON_COMMAND} ./setup.py install --prefix="${PYHOME}"

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
    export DO_PYTHON39="no"
    export FORCE_PYTHON="no"
    export USE_SYSTEM_PYTHON="no"
    export BUILD_MPI4PY="no"
    export BUILD_SPHINX="yes"
    export VISIT_PYTHON_DIR=${VISIT_PYTHON_DIR:-""}
    add_extra_commandline_args "python" "system-python" 0 "Using system python"
    add_extra_commandline_args "python" "alt-python-dir" 1 "Using alternate python directory"
    add_extra_commandline_args "python" "mpi4py" 0 "Build mpi4py"
    add_extra_commandline_args "python" "no-sphinx" 0 "Disable building sphinx"
    add_extra_commandline_args "python" "use-python39" 0 "Use Python 3.9 insterad of Python 3.7"
}

function bv_python_enable
{
    DO_PYTHON="yes"
    FORCE_PYTHON="yes"
}

function bv_python_disable
{
    DO_PYTHON="no"
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
    export BUILD_MPI4PY="yes"
}

function bv_python_use_python39
{
    echo "configuring to build python 3.9, (NOT python 3.7)"
    export DO_PYTHON39="yes"
    # call bv_python_info to update all vars
    bv_python_info
}

function bv_python_no_sphinx
{
    echo "Disabling building sphinx"
    export BUILD_SPHINX="no"
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
    info "  do py39? $DO_PYTHON39"
    # force python 3.9 with VTK 9, which is also forced with Qt6
    if [[ "$DO_VTK9" == "yes" ]] ; then
        info "  setting py39 to true because doing vtk 9"
        DO_PYTHON39="yes"
    elif [[ "$DO_QT6" == "yes" ]] ; then
        info "  setting py39 to true because doing qt 6"
        DO_PYTHON39="yes"
    fi

    # IMPORANT NOTE: 
    # We don't use export VAR={VAR:-"value"} style of init here b/c
    # it undermines us choosing between python 2 and python 3
    
    # if python 3.9
    if [[ "$DO_PYTHON39" == "yes" ]] ; then
        export PYTHON_URL="https://www.python.org/ftp/python/3.9.17"
        export PYTHON_FILE_SUFFIX="tgz"
        export PYTHON_VERSION="3.9.17"
        export PYTHON_COMPATIBILITY_VERSION="3.9"
        export PYTHON_FILE="Python-$PYTHON_VERSION.$PYTHON_FILE_SUFFIX"
        export PYTHON_BUILD_DIR="Python-$PYTHON_VERSION"
        export PYTHON_MD5_CHECKSUM=""
        export PYTHON_SHA256_CHECKSUM=""
    else
        export PYTHON_URL="https://www.python.org/ftp/python/3.7.7"
        export PYTHON_FILE_SUFFIX="tgz"
        export PYTHON_VERSION="3.7.7"
        # TODO: May need logic for "m" suffix
        export PYTHON_COMPATIBILITY_VERSION="3.7m"
        export PYTHON_FILE="Python-$PYTHON_VERSION.$PYTHON_FILE_SUFFIX"
        export PYTHON_BUILD_DIR="Python-$PYTHON_VERSION"
        export PYTHON_MD5_CHECKSUM="d348d978a5387512fbc7d7d52dd3a5ef"
        export PYTHON_SHA256_CHECKSUM="8c8be91cd2648a1a0c251f04ea0bb4c2a5570feb9c45eaaa2241c785585b475a"
    fi

    if [[ "$DO_PYTHON39" == "yes" ]] ; then
        export PILLOW_URL=${PILLOW_URL:-"https://github.com/python-pillow/Pillow/archive/refs/tags/"}
        export PILLOW_FILE="Pillow-10.0.0.tar.gz"
        export PILLOW_BUILD_DIR="Pillow-10.0.0"
        export PILLOW_MD5_CHECKSUM=""
        export PILLOW_SHA256_CHECKSUM=""
    else
        # python 3 Pillow, not PIL -- we need Pillow for Python 3 Support
        export PILLOW_URL=${PIL_URL:-"https://files.pythonhosted.org/packages/ce/ef/e793f6ffe245c960c42492d0bb50f8d14e2ba223f1922a5c3c81569cec44/"}
        export PILLOW_FILE="Pillow-7.1.2.tar.gz"
        export PILLOW_BUILD_DIR="Pillow-7.1.2"
        export PILLOW_MD5_CHECKSUM="f1f7592c51260e5080d3cd71781ea675"
        export PILLOW_SHA256_CHECKSUM="a0b49960110bc6ff5fead46013bcb8825d101026d466f3a4de3476defe0fb0dd"
    fi

    if [[ "$DO_PYTHON39" == "yes" ]] ; then
        export PYPARSING_URL=""
        export PYPARSING_FILE="pyparsing-3.1.0.tar.gz"
        export PYPARSING_BUILD_DIR="pyparsing-3.1.0"
        export PYPARSING_MD5_CHECKSUM=""
        export PYPARSING_SHA256_CHECKSUM=""
    else
        export PYPARSING_URL=""
        export PYPARSING_FILE="pyparsing-2.4.6.tar.gz"
        export PYPARSING_BUILD_DIR="pyparsing-2.4.6"
        export PYPARSING_MD5_CHECKSUM="29733ea8cbee0291aad121c69c6e51a1"
        export PYPARSING_SHA256_CHECKSUM="4c830582a84fb022400b85429791bc551f1f4871c33f23e44f353119e92f969f"
    fi

    if [[ "$DO_PYTHON39" == "yes" ]] ; then
        export REQUESTS_URL=""
        export REQUESTS_FILE="requests-2.31.0.tar.gz"
        export REQUESTS_BUILD_DIR="requests-2.31.0"
        export REQUESTS_MD5_CHECKSUM=""
        export REQUESTS_SHA256_CHECKSUM=""
    else
        export REQUESTS_URL=""
        export REQUESTS_FILE="requests-2.22.0.tar.gz"
        export REQUESTS_BUILD_DIR="requests-2.22.0"
        export REQUESTS_MD5_CHECKSUM="ee28bee2de76e9198fc41e48f3a7dd47"
        export REQUESTS_SHA256_CHECKSUM="11e007a8a2aa0323f5a921e9e6a2d7e4e67d9877e85773fba9ba6419025cbeb4"
    fi

    if [[ "$DO_PYTHON39" == "yes" ]] ; then
        export SETUPTOOLS_URL=""
        export SETUPTOOLS_FILE="setuptools-68.0.0.tar.gz"
        export SETUPTOOLS_BUILD_DIR="setuptools-68.0.0"
        export SETUPTOOLS_MD5_CHECKSUM=""
        export SETUPTOOLS_SHA256_CHECKSUM=""
    else
        export SETUPTOOLS_URL=""
        export SETUPTOOLS_FILE="setuptools-44.0.0.zip"
        export SETUPTOOLS_BUILD_DIR="setuptools-44.0.0"
        export SETUPTOOLS_MD5_CHECKSUM="32b6cdce670ce462086d246bea181e9d"
        export SETUPTOOLS_SHA256_CHECKSUM="e5baf7723e5bb8382fc146e33032b241efc63314211a3a120aaa55d62d2bb008"
    fi

    if [[ "$DO_PYTHON39" == "yes" ]] ; then
        export CYTHON_URL=""
        export CYTHON_FILE="Cython-3.0.0.tar.gz"
        export CYTHON_BUILD_DIR="Cython-3.0.0"
        export CYTHON_MD5_CHECKSUM=""
        export CYTHON_SHA256_CHECKSUM=""
    else
        export CYTHON_URL="https://files.pythonhosted.org/packages/99/36/a3dc962cc6d08749aa4b9d85af08b6e354d09c5468a3e0edc610f44c856b/"
        export CYTHON_FILE="Cython-0.29.17.tar.gz"
        export CYTHON_BUILD_DIR="Cython-0.29.17"
        export CYTHON_MD5_CHECKSUM="0936311ccd09f1164ab2f46ca5cd8c3b"
        export CYTHON_SHA256_CHECKSUM="6361588cb1d82875bcfbad83d7dd66c442099759f895cf547995f00601f9caf2"
    fi

    if [[ "$DO_PYTHON39" == "yes" ]] ; then
        export NUMPY_URL="https://github.com/numpy/numpy/releases/download/v1.25.1/"
        export NUMPY_FILE="numpy-1.25.1.tar.gz"
        export NUMPY_BUILD_DIR="numpy-1.25.1"
        export NUMPY_MD5_CHECKSUM=""
        export NUMPY_SHA256_CHECKSUM=""
    else
        export NUMPY_URL="https://github.com/numpy/numpy/releases/download/v1.16.6/"
        export NUMPY_FILE="numpy-1.16.6.zip"
        export NUMPY_BUILD_DIR="numpy-1.16.6"
        export NUMPY_MD5_CHECKSUM="3dc21c84a295fe77eadf8f872685a7de"
        export NUMPY_SHA256_CHECKSUM="e5cf3fdf13401885e8eea8170624ec96225e2174eb0c611c6f26dd33b489e3ff"
    fi

    if [[ "$DO_PYTHON39" == "yes" ]] ; then
        export MPI4PY_URL="https://pypi.python.org/pypi/mpi4py"
        export MPI4PY_FILE="mpi4py-3.1.4.tar.gz"
        export MPI4PY_BUILD_DIR="mpi4py-3.1.4"
        export MPI4PY_MD5_CHECKSUM=""
        export MPI4PY_SHA256_CHECKSUM=""
    else
        export MPI4PY_URL="https://pypi.python.org/pypi/mpi4py"
        export MPI4PY_FILE="mpi4py-2.0.0.tar.gz"
        export MPI4PY_BUILD_DIR="mpi4py-2.0.0"
        export MPI4PY_MD5_CHECKSUM="4f7d8126d7367c239fd67615680990e3"
        export MPI4PY_SHA256_CHECKSUM="6543a05851a7aa1e6d165e673d422ba24e45c41e4221f0993fe1e5924a00cb81"
    fi

    if [[ "$DO_PYTHON39" == "yes" ]] ; then
        export PACKAGING_URL=""
        export PACKAGING_FILE="packaging-23.1.tar.gz"
        export PACKAGING_BUILD_DIR="packaging-23.1"
        export PACKAGING_MD5_CHECKSUM=""
        export PACKAGING_SHA256_CHECKSUM=""
    else
        export PACKAGING_URL=""
        export PACKAGING_FILE="packaging-19.2.tar.gz"
        export PACKAGING_BUILD_DIR="packaging-19.2"
        export PACKAGING_MD5_CHECKSUM="867ce70984dc7b89bbbc3cac2a72b171"
        export PACKAGING_SHA256_CHECKSUM="28b924174df7a2fa32c1953825ff29c61e2f5e082343165438812f00d3a7fc47"
    fi

    if [[ "$DO_PYTHON39" == "yes" ]] ; then
        export IMAGESIZE_URL=""
        export IMAGESIZE_FILE="imagesize-1.4.1.tar.gz"
        export IMAGESIZE_BUILD_DIR="imagesize-1.4.1"
        export IMAGESIZE_MD5_CHECKSUM=""
        export IMAGESIZE_SHA256_CHECKSUM=""
    else
        export IMAGESIZE_URL=""
        export IMAGESIZE_FILE="imagesize-1.1.0.tar.gz"
        export IMAGESIZE_BUILD_DIR="imagesize-1.1.0"
        export IMAGESIZE_MD5_CHECKSUM="2f89749b05e07c79c46330dbc62f1e02"
        export IMAGESIZE_SHA256_CHECKSUM="f3832918bc3c66617f92e35f5d70729187676313caa60c187eb0f28b8fe5e3b5"
    fi

    if [[ "$DO_PYTHON39" == "yes" ]] ; then
        export ALABASTER_URL=""
        export ALABASTER_FILE="alabaster-0.7.13.tar.gz"
        export ALABASTER_BUILD_DIR="alabaster-0.7.13"
        export ALABASTER_MD5_CHECKSUM=""
        export ALABASTER_SHA256_CHECKSUM=""
    else
        export ALABASTER_URL=""
        export ALABASTER_FILE="alabaster-0.7.12.tar.gz"
        export ALABASTER_BUILD_DIR="alabaster-0.7.12"
        export ALABASTER_MD5_CHECKSUM="3591827fde96d1dd23970fb05410ed04"
        export ALABASTER_SHA256_CHECKSUM="a661d72d58e6ea8a57f7a86e37d86716863ee5e92788398526d58b26a4e4dc02"
    fi

    if [[ "$DO_PYTHON39" == "yes" ]] ; then
        export BABEL_URL=""
        export BABEL_FILE="Babel-2.12.1.tar.gz"
        export BABEL_BUILD_DIR="Babel-2.12.1"
        export BABEL_MD5_CHECKSUM=""
        export BABEL_SHA256_CHECKSUM=""
    else
        export BABEL_URL=""
        export BABEL_FILE="Babel-2.7.0.tar.gz"
        export BABEL_BUILD_DIR="Babel-2.7.0"
        export BABEL_MD5_CHECKSUM="83c158b7dae9135750a7cf204e6e2eea"
        export BABEL_SHA256_CHECKSUM="e86135ae101e31e2c8ec20a4e0c5220f4eed12487d5cf3f78be7e98d3a57fc28"
    fi

    if [[ "$DO_PYTHON39" == "yes" ]] ; then
        export SNOWBALLSTEMMER_URL=""
        export SNOWBALLSTEMMER_FILE="snowballstemmer-2.2.0.tar.gz"
        export SNOWBALLSTEMMER_BUILD_DIR="snowballstemmer-2.2.0"
        export SNOWBALLSTEMMER_MD5_CHECKSUM=""
        export SNOWBALLSTEMMER_SHA256_CHECKSUM=""
    else
        export SNOWBALLSTEMMER_URL=""
        export SNOWBALLSTEMMER_FILE="snowballstemmer-2.0.0.tar.gz"
        export SNOWBALLSTEMMER_BUILD_DIR="snowballstemmer-2.0.0"
        export SNOWBALLSTEMMER_MD5_CHECKSUM="c05ec4a897be3c953c8b8b844c4241d4"
        export SNOWBALLSTEMMER_SHA256_CHECKSUM="df3bac3df4c2c01363f3dd2cfa78cce2840a79b9f1c2d2de9ce8d31683992f52"
   fi

    if [[ "$DO_PYTHON39" == "yes" ]] ; then
        export DOCUTILS_URL=""
        export DOCUTILS_FILE="docutils-0.18.1.tar.gz"
        export DOCUTILS_BUILD_DIR="docutils-0.18.1"
        export DOCUTILS_MD5_CHECKSUM=""
        export DOCUTILS_SHA256_CHECKSUM=""
    else
        export DOCUTILS_URL=""
        export DOCUTILS_FILE="docutils-0.15.2.tar.gz"
        export DOCUTILS_BUILD_DIR="docutils-0.15.2"
        export DOCUTILS_MD5_CHECKSUM="e26a308d8000b0bed7416a633217c676"
        export DOCUTILS_SHA256_CHECKSUM="a2aeea129088da402665e92e0b25b04b073c04b2dce4ab65caaa38b7ce2e1a99"
    fi

    if [[ "$DO_PYTHON39" == "yes" ]] ; then
        export PYGMENTS_URL=""
        export PYGMENTS_FILE="Pygments-2.15.1.tar.gz"
        export PYGMENTS_BUILD_DIR="Pygments-2.15.1"
        export PYGMENTS_MD5_CHECKSUM=""
        export PYGMENTS_SHA256_CHECKSUM=""
    else
        export PYGMENTS_URL=""
        export PYGMENTS_FILE="Pygments-2.5.2.tar.gz"
        export PYGMENTS_BUILD_DIR="Pygments-2.5.2"
        export PYGMENTS_MD5_CHECKSUM="465a35559863089d959d783a69f79b9f"
        export PYGMENTS_SHA256_CHECKSUM="98c8aa5a9f778fcd1026a17361ddaf7330d1b7c62ae97c3bb0ae73e0b9b6b0fe"
    fi

    if [[ "$DO_PYTHON39" == "yes" ]] ; then
        export JINJA2_URL=""
        export JINJA2_FILE="Jinja2-3.1.2.tar.gz"
        export JINJA2_BUILD_DIR="Jinja2-3.1.2"
        export JINJA2_MD5_CHECKSUM=""
        export JINJA2_SHA256_CHECKSUM=""
    else
        export JINJA2_URL=""
        export JINJA2_FILE="Jinja2-2.10.3.tar.gz"
        export JINJA2_BUILD_DIR="Jinja2-2.10.3"
        export JINJA2_MD5_CHECKSUM="7883559bc5cc3e2781d94b4be61cfdcd"
        export JINJA2_SHA256_CHECKSUM="9fe95f19286cfefaa917656583d020be14e7859c6b0252588391e47db34527de"
    fi

    if [[ "$DO_PYTHON39" == "yes" ]] ; then
        export SPHINXCONTRIB_QTHELP_URL=""
        export SPHINXCONTRIB_QTHELP_FILE="sphinxcontrib-qthelp-1.0.3.tar.gz"
        export SPHINXCONTRIB_QTHELP_BUILD_DIR="sphinxcontrib-qthelp-1.0.3"
        export SPHINXCONTRIB_QTHELP_MD5_CHECKSUM=""
        export SPHINXCONTRIB_QTHELP_SHA256_CHECKSUM=""
    else
        export SPHINXCONTRIB_QTHELP_URL=""
        export SPHINXCONTRIB_QTHELP_FILE="sphinxcontrib-qthelp-1.0.2.tar.gz"
        export SPHINXCONTRIB_QTHELP_BUILD_DIR="sphinxcontrib-qthelp-1.0.2"
        export SPHINXCONTRIB_QTHELP_MD5_CHECKSUM="3532d4643d0b1cc3806e43f59495c030"
        export SPHINXCONTRIB_QTHELP_SHA256_CHECKSUM="79465ce11ae5694ff165becda529a600c754f4bc459778778c7017374d4d406f"
    fi

    if [[ "$DO_PYTHON39" == "yes" ]] ; then
        export SPHINXCONTRIB_SERIALIZINGHTML_URL=""
        export SPHINXCONTRIB_SERIALIZINGHTML_FILE="sphinxcontrib-serializinghtml-1.1.5.tar.gz"
        export SPHINXCONTRIB_SERIALIZINGHTML_BUILD_DIR="sphinxcontrib-serializinghtml-1.1.5"
        export SPHINXCONTRIB_SERIALIZINGHTML_MD5_CHECKSUM=""
        export SPHINXCONTRIB_SERIALIZINGHTML_SHA256_CHECKSUM=""
    else
        export SPHINXCONTRIB_SERIALIZINGHTML_URL=""
        export SPHINXCONTRIB_SERIALIZINGHTML_FILE="sphinxcontrib-serializinghtml-1.1.3.tar.gz"
        export SPHINXCONTRIB_SERIALIZINGHTML_BUILD_DIR="sphinxcontrib-serializinghtml-1.1.3"
        export SPHINXCONTRIB_SERIALIZINGHTML_MD5_CHECKSUM="6a4318d6d11c345fbc669e6a86f32766"
        export SPHINXCONTRIB_SERIALIZINGHTML_SHA256_CHECKSUM="c0efb33f8052c04fd7a26c0a07f1678e8512e0faec19f4aa8f2473a8b81d5227"
    fi

    if [[ "$DO_PYTHON39" == "yes" ]] ; then
        export SPHINXCONTRIB_HTMLHELP_URL=""
        export SPHINXCONTRIB_HTMLHELP_FILE="sphinxcontrib-htmlhelp-2.0.1.tar.gz"
        export SPHINXCONTRIB_HTMLHELP_BUILD_DIR="sphinxcontrib-htmlhelp-2.0.1"
        export SPHINXCONTRIB_HTMLHELP_MD5_CHECKSUM=""
        export SPHINXCONTRIB_HTMLHELP_SHA256_CHECKSUM=""
    else
        export SPHINXCONTRIB_HTMLHELP_URL=""
        export SPHINXCONTRIB_HTMLHELP_FILE="sphinxcontrib-htmlhelp-1.0.2.tar.gz"
        export SPHINXCONTRIB_HTMLHELP_BUILD_DIR="sphinxcontrib-htmlhelp-1.0.2"
        export SPHINXCONTRIB_HTMLHELP_MD5_CHECKSUM="f72e4b26ec0f6387d855c74819500b66"
        export SPHINXCONTRIB_HTMLHELP_SHA256_CHECKSUM="4670f99f8951bd78cd4ad2ab962f798f5618b17675c35c5ac3b2132a14ea8422"
    fi

    export SPHINXCONTRIB_JSMATH_URL=""
    export SPHINXCONTRIB_JSMATH_FILE="sphinxcontrib-jsmath-1.0.1.tar.gz"
    export SPHINXCONTRIB_JSMATH_BUILD_DIR="sphinxcontrib-jsmath-1.0.1"
    export SPHINXCONTRIB_JSMATH_MD5_CHECKSUM="e45179f0a3608b6766862e0f34c23b62"
    export SPHINXCONTRIB_JSMATH_SHA256_CHECKSUM="a9925e4a4587247ed2191a22df5f6970656cb8ca2bd6284309578f2153e0c4b8"

    if [[ "$DO_PYTHON39" == "yes" ]] ; then
        export SPHINXCONTRIB_DEVHELP_URL=""
        export SPHINXCONTRIB_DEVHELP_FILE="sphinxcontrib-devhelp-1.0.2.tar.gz"
        export SPHINXCONTRIB_DEVHELP_BUILD_DIR="sphinxcontrib-devhelp-1.0.2"
        export SPHINXCONTRIB_DEVHELP_MD5_CHECKSUM=""
        export SPHINXCONTRIB_DEVHELP_SHA256_CHECKSUM=""
    else
        export SPHINXCONTRIB_DEVHELP_URL=""
        export SPHINXCONTRIB_DEVHELP_FILE="sphinxcontrib-devhelp-1.0.1.tar.gz"
        export SPHINXCONTRIB_DEVHELP_BUILD_DIR="sphinxcontrib-devhelp-1.0.1"
        export SPHINXCONTRIB_DEVHELP_MD5_CHECKSUM="ecb33259e2e8300493d210140af7d957"
        export SPHINXCONTRIB_DEVHELP_SHA256_CHECKSUM="6c64b077937330a9128a4da74586e8c2130262f014689b4b89e2d08ee7294a34"
    fi

    if [[ "$DO_PYTHON39" == "yes" ]] ; then
        export SPHINXCONTRIB_APPLEHELP_URL=""
        export SPHINXCONTRIB_APPLEHELP_FILE="sphinxcontrib-applehelp-1.0.4.tar.gz"
        export SPHINXCONTRIB_APPLEHELP_BUILD_DIR="sphinxcontrib-applehelp-1.0.4"
        export SPHINXCONTRIB_APPLEHELP_MD5_CHECKSUM=""
        export SPHINXCONTRIB_APPLEHELP_SHA256_CHECKSUM=""
    else
        export SPHINXCONTRIB_APPLEHELP_URL=""
        export SPHINXCONTRIB_APPLEHELP_FILE="sphinxcontrib-applehelp-1.0.1.tar.gz"
        export SPHINXCONTRIB_APPLEHELP_BUILD_DIR="sphinxcontrib-applehelp-1.0.1"
        export SPHINXCONTRIB_APPLEHELP_MD5_CHECKSUM="c3424507cc28291f8005081b6a96afb1"
        export SPHINXCONTRIB_APPLEHELP_SHA256_CHECKSUM="edaa0ab2b2bc74403149cb0209d6775c96de797dfd5b5e2a71981309efab3897"
    fi

    if [[ "$DO_PYTHON39" == "no" ]] ; then
        export SIX_URL=""
        export SIX_FILE="six-1.13.0.tar.gz"
        export SIX_BUILD_DIR="six-1.13.0"
        export SIX_MD5_CHECKSUM="e92c23c882c7d5564ce5773fe31b2771"
        export SIX_SHA256_CHECKSUM="30f610279e8b2578cab6db20741130331735c781b56053c59c4076da27f06b66"
    fi

    if [[ "$DO_PYTHON39" == "yes" ]] ; then
        export URLLIB3_URL=""
        export URLLIB3_FILE="urllib3-2.0.3.tar.gz"
        export URLLIB3_BUILD_DIR="urllib3-2.0.3"
        export URLLIB3_MD5_CHECKSUM=""
        export URLLIB3_SHA256_CHECKSUM=""
    else
        export URLLIB3_URL=""
        export URLLIB3_FILE="urllib3-1.25.7.tar.gz"
        export URLLIB3_BUILD_DIR="urllib3-1.25.7"
        export URLLIB3_MD5_CHECKSUM="85e1e3925f8c1095172bff343f3312ed"
        export URLLIB3_SHA256_CHECKSUM="f3c5fd51747d450d4dcf6f923c81f78f811aab8205fda64b0aba34a4e48b0745"
    fi

    if [[ "$DO_PYTHON39" == "yes" ]] ; then
        export IDNA_URL=""
        export IDNA_FILE="idna-3.4.tar.gz"
        export IDNA_BUILD_DIR="idna-3.4"
        export IDNA_MD5_CHECKSUM=""
        export IDNA_SHA256_CHECKSUM=""
    else
        export IDNA_URL=""
        export IDNA_FILE="idna-2.8.tar.gz"
        export IDNA_BUILD_DIR="idna-2.8"
        export IDNA_MD5_CHECKSUM="2e9ae0b4a0b26d1747c6127cdb060bc1"
        export IDNA_SHA256_CHECKSUM="c357b3f628cf53ae2c4c05627ecc484553142ca23264e593d327bcde5e9c3407"
    fi

    if [[ "$DO_PYTHON39" == "yes" ]] ; then
        export CHARSET_NORMALIZER_URL=""
        export CHARSET_NORMALIZER_FILE="charset-normalizer-3.2.0.tar.gz"
        export CHARSET_NORMALIZER_BUILD_DIR="charset-normalizer-3.2.0"
        export CHARSET_NORMALIZER_MD5_CHECKSUM=""
        export CHARSET_NORMALIZER_SHA256_CHECKSUM=""
    else
        export CHARDET_URL=""
        export CHARDET_FILE="chardet-3.0.4.tar.gz"
        export CHARDET_BUILD_DIR="chardet-3.0.4"
        export CHARDET_MD5_CHECKSUM="7dd1ba7f9c77e32351b0a0cfacf4055c"
        export CHARDET_SHA256_CHECKSUM="84ab92ed1c4d4f16916e05906b6b75a6c0fb5db821cc65e70cbd64a3e2a5eaae"
    fi

    if [[ "$DO_PYTHON39" == "yes" ]] ; then
        export CERTIFI_URL=""
        export CERTIFI_FILE="certifi-2023.5.7.tar.gz"
        export CERTIFI_BUILD_DIR="certifi-2023.5.7"
        export CERTIFI_MD5_CHECKSUM=""
        export CERTIFI_SHA256_CHECKSUM=""
    else
        export CERTIFI_URL=""
        export CERTIFI_FILE="certifi-2019.11.28.tar.gz"
        export CERTIFI_BUILD_DIR="certifi-2019.11.28"
        export CERTIFI_MD5_CHECKSUM="4d5229c4d9f0a4a79106f9e2c2cfd381"
        export CERTIFI_SHA256_CHECKSUM="25b64c7da4cd7479594d035c08c2d809eb4aab3a26e5a990ea98cc450c320f1f"
    fi

    if [[ "$DO_PYTHON39" == "no" ]] ; then
        export PYTZ_URL=""
        export PYTZ_FILE="pytz-2019.3.tar.gz"
        export PYTZ_BUILD_DIR="pytz-2019.3"
        export PYTZ_MD5_CHECKSUM="c3d84a465fc56a4edd52cca8873ac0df"
        export PYTZ_SHA256_CHECKSUM="b02c06db6cf09c12dd25137e563b31700d3b80fcc4ad23abb7a315f2789819be"
    fi

    if [[ "$DO_PYTHON39" == "yes" ]] ; then
        export MARKUPSAFE_URL=""
        export MARKUPSAFE_FILE="MarkupSafe-2.1.3.tar.gz"
        export MARKUPSAFE_BUILD_DIR="MarkupSafe-2.1.3"
        export MARKUPSAFE_MD5_CHECKSUM=""
        export MARKUPSAFE_SHA256_CHECKSUM=""
    else
        export MARKUPSAFE_URL=""
        export MARKUPSAFE_FILE="MarkupSafe-1.1.1.tar.gz"
        export MARKUPSAFE_BUILD_DIR="MarkupSafe-1.1.1"
        export MARKUPSAFE_MD5_CHECKSUM="43fd756864fe42063068e092e220c57b"
        export MARKUPSAFE_SHA256_CHECKSUM="29872e92839765e546828bb7754a68c418d927cd064fd4708fab9fe9c8bb116b"
    fi

    if [[ "$DO_PYTHON39" == "yes" ]] ; then
        export ZIPP_URL=""
        export ZIPP_FILE="zipp-3.16.2.tar.gz"
        export ZIPP_BUILD_DIR="zipp-3.16.2"
        export ZIPP_MD5_CHECKSUM=""
        export ZIPP_SHA256_CHECKSUM=""

        export IMPORTLIB_METADATA_URL=""
        export IMPORTLIB_METADATA_FILE="importlib_metadata-6.8.0.tar.gz"
        export IMPORTLIB_METADATA_BUILD_DIR="importlib_metadata-6.8.0"
        export IMPORTLIB_METADATA_MD5_CHECKSUM=""
        export IMPORTLIB_METADATA_SHA256_CHECKSUM=""
    fi

    if [[ "$DO_PYTHON39" == "yes" ]] ; then
        export SPHINX_URL=""
        export SPHINX_FILE="Sphinx-7.0.1.tar.gz"
        export SPHINX_BUILD_DIR="Sphinx-7.0.1"
        export SPHINX_MD5_CHECKSUM=""
        export SPHINX_SHA256_CHECKSUM=""
    else
        export SPHINX_URL="https://files.pythonhosted.org/packages/f6/3a/c51fc285c0c5c30bcd9426bf096187840683d9383df716a6b6a4ca0a8bde"
        export SPHINX_FILE="Sphinx-2.2.1.tar.gz"
        export SPHINX_BUILD_DIR="Sphinx-2.2.1"
        export SPHINX_MD5_CHECKSUM="60ea892a09b463e5ecb6ea26d2470f36"
        export SPHINX_SHA256_CHECKSUM="31088dfb95359384b1005619827eaee3056243798c62724fd3fa4b84ee4d71bd"
    fi

    if [[ "$DO_PYTHON39" == "yes" ]] ; then
        export SPHINX_RTD_THEME_URL=""
        export SPHINX_RTD_THEME_FILE="sphinx_rtd_theme-1.2.2.tar.gz"
        export SPHINX_RTD_THEME_BUILD_DIR="sphinx_rtd_theme-1.2.2"
        export SPHINX_RTD_THEME_MD5_CHECKSUM=""
        export SPHINX_RTD_THEME_SHA256_CHECKSUM=""
    else
        export SPHINX_RTD_THEME_URL="https://files.pythonhosted.org/packages/ed/73/7e550d6e4cf9f78a0e0b60b9d93dba295389c3d271c034bf2ea3463a79f9"
        export SPHINX_RTD_THEME_FILE="sphinx_rtd_theme-0.4.3.tar.gz"
        export SPHINX_RTD_THEME_BUILD_DIR="sphinx_rtd_theme-0.4.3"
        export SPHINX_RTD_THEME_MD5_CHECKSUM="6c50f30bc39046f497d336039a0c13fa"
        export SPHINX_RTD_THEME_SHA256_CHECKSUM="728607e34d60456d736cc7991fd236afb828b21b82f956c5ea75f94c8414040a"
    fi

    if [[ "$DO_PYTHON39" == "yes" ]] ; then
        # needed by sphinx_rtd_theme
        export SPHINXCONTRIB_JQUERY_URL=""
        export SPHINXCONTRIB_JQUERY_FILE="sphinxcontrib-jquery-4.1.tar.gz"
        export SPHINXCONTRIB_JQUERY_BUILD_DIR="sphinxcontrib-jquery-4.1"
        export SPHINXCONTRIB_JQUERY_MD5_CHECKSUM=""
        export SPHINXCONTRIB_JQUERY_SHA256_CHECKSUM=""
    fi

    if [[ "$DO_PYTHON39" == "yes" ]] ; then
        export SPHINX_TABS_URL=""
        export SPHINX_TABS_FILE="sphinx-tabs-3.4.1.tar.gz"
        export SPHINX_TABS_BUILD_DIR="sphinx-tabs-3.4.1"
        export SPHINX_TABS_MD5_CHECKSUM=""
        export SPHINX_TABS_SHA256_CHECKSUM=""
    else
        export SPHINX_TABS_URL="https://github.com/executablebooks/sphinx-tabs/archive/refs/tags"
        export SPHINX_TABS_FILE="v2.1.0.tar.gz"
        export SPHINX_TABS_BUILD_DIR="sphinx-tabs-2.1.0"
        export SPHINX_TABS_MD5_CHECKSUM="985650c490898ae674492b48f81ae497"
        export SPHINX_TABS_SHA256_CHECKSUM="39bfc9e2051f2a048eaa9da2dbf1f56b0c03c17cc72192fc8b4357cb32a95765"
    fi
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
    printf "%-20s %s [%s]\n" "--use-python39" "Build Python 3.9 instead of Python 3.7"
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

function apply_python_macos11_configure_patch
{
    # These two patches come from python-3.7.12, to address build failures on MacOS11
    info "Patching Python: applying configure patch for MacOS 11"
    patch -f -p0 << \EOF
*** configure.orig	Mon Mar  9 23:11:12 2020
--- configure	Fri Sep  3 20:49:21 2021
***************
*** 3374,3380 ****
    # has no effect, don't bother defining them
    Darwin/[6789].*)
      define_xopen_source=no;;
!   Darwin/1[0-9].*)
      define_xopen_source=no;;
    # On AIX 4 and 5.1, mbstate_t is defined only when _XOPEN_SOURCE == 500 but
    # used in wcsnrtombs() and mbsnrtowcs() even if _XOPEN_SOURCE is not defined
--- 3374,3380 ----
    # has no effect, don't bother defining them
    Darwin/[6789].*)
      define_xopen_source=no;;
!   Darwin/[12][0-9].*)
      define_xopen_source=no;;
    # On AIX 4 and 5.1, mbstate_t is defined only when _XOPEN_SOURCE == 500 but
    # used in wcsnrtombs() and mbsnrtowcs() even if _XOPEN_SOURCE is not defined
***************
*** 9251,9256 ****
--- 9251,9259 ----
      	ppc)
      		MACOSX_DEFAULT_ARCH="ppc64"
      		;;
+     	arm64)
+     		MACOSX_DEFAULT_ARCH="arm64"
+     		;;
      	*)
      		as_fn_error $? "Unexpected output of 'arch' on OSX" "$LINENO" 5
      		;;
EOF
    if [[ $? != 0 ]] ; then
        warn "Python patch to configure for MacOS 11  failed."
        return 1
    fi

    info "Patching Python: applying configure.ac patch for MacOS 11"
    patch -f -p0 << \EOF
*** configure.ac.orig	Mon Mar  9 23:11:12 2020
--- configure.ac	Fri Sep  3 20:49:21 2021
***************
*** 490,496 ****
    # has no effect, don't bother defining them
    Darwin/@<:@6789@:>@.*)
      define_xopen_source=no;;
!   Darwin/1@<:@0-9@:>@.*)
      define_xopen_source=no;;
    # On AIX 4 and 5.1, mbstate_t is defined only when _XOPEN_SOURCE == 500 but
    # used in wcsnrtombs() and mbsnrtowcs() even if _XOPEN_SOURCE is not defined
--- 490,496 ----
    # has no effect, don't bother defining them
    Darwin/@<:@6789@:>@.*)
      define_xopen_source=no;;
!   Darwin/@<:@[12]@:>@@<:@0-9@:>@.*)
      define_xopen_source=no;;
    # On AIX 4 and 5.1, mbstate_t is defined only when _XOPEN_SOURCE == 500 but
    # used in wcsnrtombs() and mbsnrtowcs() even if _XOPEN_SOURCE is not defined
***************
*** 2456,2461 ****
--- 2456,2464 ----
      	ppc)
      		MACOSX_DEFAULT_ARCH="ppc64"
      		;;
+     	arm64)
+     		MACOSX_DEFAULT_ARCH="arm64"
+     		;;
      	*)
      		AC_MSG_ERROR([Unexpected output of 'arch' on OSX])
      		;;


EOF
    if [[ $? != 0 ]] ; then
        warn "Python patch to configure.ac for MacOS 11 failed."
        return 1
    fi

    return 0
}

function apply_python_patch
{
    # not sure if these apply for 3.9

    if [[ "$DO_PYTHON39" == "no" ]] ; then
        if [[ "$OPSYS" == "Darwin" ]]; then
            VER=$(uname -r)
            if [[ ${VER%%.*} == 8 ]] ; then
                apply_python_osx104_patch
                if [[ $? != 0 ]] ; then
                    return 1
                fi
            fi

            # shouldn't do any harm applying this
            # on all mac versions
            apply_python_macos11_configure_patch
            if [[ $? != 0 ]] ; then
                return 1
            fi
        fi
    fi

    return 0
}

function apply_python39_pillow_patch
{
    info "Patching Python: fix setup.py in Pillow."
    patch -f -p0 << \EOF
--- setup.py.orig       2023-07-19 10:28:48.931905000 -0700
+++ setup.py    2023-07-19 10:45:27.500838000 -0700
@@ -989,7 +989,7 @@
         version=PILLOW_VERSION,
         cmdclass={"build_ext": pil_build_ext},
         ext_modules=ext_modules,
-        zip_safe=not (debug_build() or PLATFORM_MINGW),
+        zip_safe=False
     )
 except RequiredDependencyException as err:
     msg = f"""

EOF
     if [[ $? != 0 ]] ; then
         warn "Python patch for setup.py in Pillow failed."
         return 1
     fi

     return 0
}

function apply_python37_pillow_patch
{
    info "Patching Python: fix setup.py in Pillow."
    patch -f -p1 << \EOF
    diff --git a/setup.py b/setup.py
    index 3e1a812..3520895 100755
    --- a/setup.py
    +++ b/setup.py
    @@ -896,7 +896,7 @@ try:
             packages=["PIL"],
             package_dir={"": "src"},
             keywords=["Imaging"],
    -        zip_safe=not (debug_build() or PLATFORM_MINGW),
    +        zip_safe=False,
         )
     except RequiredDependencyException as err:
         msg = """
EOF
     if [[ $? != 0 ]] ; then
         warn "Python patch for setup.py in Pillow failed."
         return 1
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

    fix_py_permissions

    cd "$START_DIR"
    info "Done with Python"

    return 0
}

# *************************************************************************** #
#                            Function 7.1, build_pillow                       #
# *************************************************************************** #
function build_pillow
{
    download_py_module ${PILLOW_FILE} ${PILLOW_URL}
    if [[ $? != 0 ]] ; then
        return 1
    fi

    extract_py_module ${PILLOW_BUILD_DIR} ${PILLOW_FILE} "pillow"
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

    pushd $PILLOW_BUILD_DIR > /dev/null

    #
    # Apply patches
    #
    if [[ "$DO_PYTHON39" == "yes" ]] ; then
        apply_python39_pillow_patch
    else
        apply_python37_pillow_patch
    fi
    if [[ $? != 0 ]] ; then
        if [[ $untarred_python == 1 ]] ; then
            warn "Giving up on pillow install."
            return 1
        else
            warn "Patch failed, but continuing.  I believe that this script\n" \
                 "tried to apply a patch to an existing directory that had\n" \
                 "already been patched ... that is, the patch is\n" \
                 "failing harmlessly on a second application."
        fi
    fi

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
#                            Function 7.2, build_pyparsing                    #
# *************************************************************************** #
function build_pyparsing
{
    download_py_module ${PYPARSING_FILE} ${PYPARSING_URL}
    if [[ $? != 0 ]] ; then
        return 1
    fi

    extract_py_module ${PYPARSING_BUILD_DIR} ${PYPARSING_FILE} "pyparsing"
    if [[ $? != 0 ]] ; then
        return 1
    fi

    install_py_module ${PYPARSING_BUILD_DIR} "pyparsing"
    if test $? -ne 0 ; then
        return 1
    fi

    # pyparsing installs into site-packages dir of Visit's Python.
    # Simply re-execute the python perms command.
    fix_py_permissions

    info "Done with pyparsing."
    return 0
}

# *************************************************************************** #
#                            Function 7.3, build_requests                     #
# *************************************************************************** #
function build_requests
{
    # python 3: Requests depends on certifi, urllib3, idna,
    # and  chardet (3.7) or charset-normalizer (3.9).

    download_py_module ${CERTIFI_FILE} ${CERTIFIY_URL}
    if test $? -ne 0 ; then
        return 1
    fi

    download_py_module ${URLLIB3_FILE} ${URLLIB3_URL}
    if test $? -ne 0 ; then
        return 1
    fi

    download_py_module ${IDNA_FILE} ${IDNA_URL}
    if test $? -ne 0 ; then
        return 1
    fi

    if [[ "$DO_PYTHON39" == "yes" ]] ; then
        download_py_module ${CHARSET_NORMALIZER_FILE} ${CHARSET_NORMALIZER_URL}
        if test $? -ne 0 ; then
            return 1
        fi
    else
        download_py_module ${CHARDET_FILE} ${CHARDET_URL}
        if test $? -ne 0 ; then
            return 1
        fi
    fi

    extract_py_module ${CERTIFI_BUILD_DIR} ${CERTIFI_FILE} "certifi"
    if test $? -ne 0 ; then
        return 1
    fi

    extract_py_module ${URLLIB3_BUILD_DIR} ${URLLIB3_FILE} "urllib3"
    if test $? -ne 0 ; then
        return 1
    fi

    extract_py_module ${IDNA_BUILD_DIR} ${IDNA_FILE} "idna"
    if test $? -ne 0 ; then
        return 1
    fi

    if [[ "$DO_PYTHON39" == "yes" ]] ; then
        extract_py_module ${CHARSET_NORMALIZER_BUILD_DIR} ${CHARSET_NORMALIZER_FILE} "charset-normalizer"
        if test $? -ne 0 ; then
            return 1
        fi
    else
        extract_py_module ${CHARDET_BUILD_DIR} ${CHARDET_FILE} "chardet"
        if test $? -ne 0 ; then
            return 1
        fi
    fi

    install_py_module ${CERTIFI_BUILD_DIR} "certifi"
    if test $? -ne 0 ; then
        return 1
    fi

    install_py_module ${URLLIB3_BUILD_DIR} "urllib3"
    if test $? -ne 0 ; then
        return 1
    fi

    install_py_module ${IDNA_BUILD_DIR} "idna"
    if test $? -ne 0 ; then
        return 1
    fi

    if [[ "$DO_PYTHON39" == "yes" ]] ; then
        install_py_module ${CHARSET_NORMALIZER_BUILD_DIR} "charset-normalizer"
        if test $? -ne 0 ; then
            return 1
        fi
    else
        install_py_module ${CHARDET_BUILD_DIR} "chardet"
        if test $? -ne 0 ; then
            return 1
        fi
    fi

    download_py_module ${REQUESTS_FILE} ${REQUESTS_URL}
    if test $? -ne 0 ; then
        return 1
    fi

    extract_py_module ${REQUESTS_BUILD_DIR} ${REQUESTS_FILE} "requests"
    if test $? -ne 0 ; then
        return 1
    fi

    install_py_module ${REQUESTS_BUILD_DIR} "requests"
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
    download_py_module ${MPI4PY_FILE} ${MPI4PY_URL}
    if test $? -ne 0 ; then
        return 1
    fi

    extract_py_module ${MPI4PY_BUILD_DIR} ${MPI4PY_FILE} "mpi4py"
    if test $? -ne 0 ; then
        return 1
    fi

    # Not use install_py_module function because we need to use setup.py
    # even for python 3.9 ??
    install_py_module ${MPI4PY_BUILD_DIR} "mpi4py"
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
    download_py_module ${SETUPTOOLS_FILE} ${SETUPTOOLS_URL}
    if [[ $? != 0 ]] ; then
        return 1
    fi

    download_py_module ${CYTHON_FILE} ${CYTHON_URL}
    if [[ $? != 0 ]] ; then
        return 1
    fi

    download_py_module ${NUMPY_FILE} ${NUMPY_URL}
    if [[ $? != 0 ]] ; then
        return 1
    fi

    extract_py_module ${SETUPTOOLS_BUILD_DIR} ${SETUPTOOLS_FILE} "setuptools"
    if [[ $? != 0 ]] ; then
        return 1
    fi

    extract_py_module ${CYTHON_BUILD_DIR} ${CYTHON_FILE} "cython"
    if [[ $? != 0 ]] ; then
        return 1
    fi

    extract_py_module ${NUMPY_BUILD_DIR} ${NUMPY_FILE} "numpy"
    if [[ $? != 0 ]] ; then
        return 1
    fi

    install_py_module_with_setup ${SETUPTOOLS_BUILD_DIR} "setuptools"
    if [[ $? != 0 ]] ; then
        return 1
    fi

    install_py_module_with_setup ${CYTHON_BUILD_DIR} "cython"
    if [[ $? != 0 ]] ; then
        return 1
    fi

    pushd $NUMPY_BUILD_DIR > /dev/null
    cat << \EOF > site.cfg
[openblas]
libraries =
library_dirs =
include_dirs =
EOF
    info "Installing numpy (~ 2 min) ..."
    sed -i 's#\\\\\"%s\\\\\"#%s#' numpy/distutils/system_info.py
    CC=${C_COMPILER} BLAS=None LAPACK=None ATLAS=None ${PYTHON_COMMAND} ./setup.py install --prefix="${PYHOME}"
    if test $? -ne 0 ; then
        popd > /dev/null
        warn "Could not install numpy"
        return 1
    fi
    popd > /dev/null

    fix_py_permissions

    return 0
}

# *************************************************************************** #
#                                  build_sphinx                               #
# *************************************************************************** #
function build_sphinx
{
    download_py_module ${PACKAGING_FILE} ${PACKAGING_URL}
    if test $? -ne 0 ; then
        return 1
    fi

    # handle the fact that this may have been installed by numpy?
    if [[ "$DO_PYTHON39" == "no" ]] ; then
        download_py_module ${SETUPTOOLS_FILE} ${SETUPTOOLS_URL} 
        if test $? -ne 0 ; then
            return 1
        fi
    fi

    download_py_module ${IMAGESIZE_FILE} ${IMAGESIZE_URL}
    if test $? -ne 0 ; then
        return 1
    fi

    download_py_module ${ALABASTER_FILE} ${ALABASTER_URL}
    if test $? -ne 0 ; then
        return 1
    fi

    download_py_module ${BABEL_FILE} ${BABEL_URL}
    if test $? -ne 0 ; then
        return 1
    fi

    download_py_module ${SNOWBALLSTEMMER_FILE} ${SNOWBALLSTEMMER_URL} 
    if test $? -ne 0 ; then
        return 1
    fi

    download_py_module ${DOCUTILS_FILE} ${DOCUTILS_URL}
    if test $? -ne 0 ; then
        return 1
    fi

    download_py_module ${PYGMENTS_FILE} ${PYGMENTS_URL}
    if test $? -ne 0 ; then
        return 1
    fi

    download_py_module ${JINJA2_FILE} ${JINJA2_URL}
    if test $? -ne 0 ; then
        return 1
    fi

    download_py_module ${SPHINXCONTRIB_QTHELP_FILE} ${SPHINXCONTRIB_QTHELP_URL}
    if test $? -ne 0 ; then
        return 1
    fi

    download_py_module ${SPHINXCONTRIB_SERIALIZINGHTML_FILE} ${SPHINXCONTRIB_SERIALIZINGHTML_URL}
    if test $? -ne 0 ; then
        return 1
    fi

    download_py_module ${SPHINXCONTRIB_HTMLHELP_FILE} ${SPHINXCONTRIB_HTMLHELP_URL}
    if test $? -ne 0 ; then
        return 1
    fi

    download_py_module ${SPHINXCONTRIB_JSMATH_FILE} ${SPHINXCONTRIB_JSMATH_URL}
    if test $? -ne 0 ; then
        return 1
    fi

    download_py_module ${SPHINXCONTRIB_DEVHELP_FILE} ${SPHINXCONTRIB_DEVHELP_URL}
    if test $? -ne 0 ; then
        return 1
    fi

    download_py_module ${SPHINXCONTRIB_APPLEHELP_FILE} ${SPHINXCONTRIB_APPLEHELP_ULR}
    if test $? -ne 0 ; then
        return 1
    fi

    if [[ "$DO_PYTHON39" == "no" ]] ; then
        download_py_module ${SIX_FILE} ${SIX_URL}
        if test $? -ne 0 ; then
            return 1
        fi

        download_py_module ${PYTZ_FILE} ${PYTZ_URL}
        if test $? -ne 0 ; then
            return 1
        fi
    fi

    download_py_module ${MARKUPSAFE_FILE} ${MARKUPSAFE_URL}
    if test $? -ne 0 ; then
        return 1
    fi

    if [[ "$DO_PYTHON39" == "yes" ]] ; then
        download_py_module ${ZIPP_FILE} ${ZIPP_URL}
        if test $? -ne 0 ; then
            return 1
        fi

        download_py_module ${IMPORTLIB_METADATA_FILE} ${IMPORTLIB_METADATA_URL}
        if test $? -ne 0 ; then
            return 1
        fi
    fi

    download_py_module ${SPHINX_FILE} ${SPHINX_URL}
    if test $? -ne 0 ; then
        return 1
    fi


    extract_py_module ${PACKAGING_BUILD_DIR} ${PACKAGING_FILE} "packaging"
    if test $? -ne 0 ; then
        return 1
    fi

    if [[ "$DO_PYTHON39" == "no" ]] ; then
        extract_py_module ${SETUPTOOLS_BUILD_DIR} ${SETUPTOOLS_FILE} "setuptools"
        if test $? -ne 0 ; then
            return 1
        fi
    fi

    extract_py_module ${IMAGESIZE_BUILD_DIR} ${IMAGESIZE_FILE} "imagesize"
    if test $? -ne 0 ; then
        return 1
    fi

    extract_py_module ${ALABASTER_BUILD_DIR} ${ALABASTER_FILE} "alabaster"
    if test $? -ne 0 ; then
        return 1
    fi

    extract_py_module ${BABEL_BUILD_DIR} ${BABEL_FILE} "babel"
    if test $? -ne 0 ; then
        return 1
    fi

    extract_py_module ${SNOWBALLSTEMMER_BUILD_DIR} ${SNOWBALLSTEMMER_FILE} "snowballstemmer"
    if test $? -ne 0 ; then
        return 1
    fi

    extract_py_module ${DOCUTILS_BUILD_DIR} ${DOCUTILS_FILE} "docutils"
    if test $? -ne 0 ; then
        return 1
    fi

    extract_py_module ${PYGMENTS_BUILD_DIR} ${PYGMENTS_FILE} "pygments"
    if test $? -ne 0 ; then
        return 1
    fi

    extract_py_module ${JINJA2_BUILD_DIR} ${JINJA2_FILE} "jinja2"
    if test $? -ne 0 ; then
        return 1
    fi

    extract_py_module ${SPHINXCONTRIB_QTHELP_BUILD_DIR} ${SPHINXCONTRIB_QTHELP_FILE} "sphinxcontrib-qthelp"
    if test $? -ne 0 ; then
        return 1
    fi

    extract_py_module ${SPHINXCONTRIB_SERIALIZINGHTML_BUILD_DIR} ${SPHINXCONTRIB_SERIALIZINGHTML_FILE} "sphinxcontrib-serializinghtml"
    if test $? -ne 0 ; then
        return 1
    fi

    extract_py_module ${SPHINXCONTRIB_HTMLHELP_BUILD_DIR} ${SPHINXCONTRIB_HTMLHELP_FILE} "sphinxcontrib-htmlhelp"
    if test $? -ne 0 ; then
        return 1
    fi

    extract_py_module ${SPHINXCONTRIB_JSMATH_BUILD_DIR} ${SPHINXCONTRIB_JSMATH_FILE} "sphinxcontrib-jsmath"
    if test $? -ne 0 ; then
        return 1
    fi

    extract_py_module ${SPHINXCONTRIB_DEVHELP_BUILD_DIR} ${SPHINXCONTRIB_DEVHELP_FILE} "sphinxcontrib-devhelp"
    if test $? -ne 0 ; then
        return 1
    fi

    extract_py_module ${SPHINXCONTRIB_APPLEHELP_BUILD_DIR} ${SPHINXCONTRIB_APPLEHELP_FILE} "sphinxcontrib-applehelp"
    if test $? -ne 0 ; then
        return 1
    fi

    if [[ "$DO_PYTHON39" == "no" ]] ; then
        extract_py_module ${SIX_BUILD_DIR} ${SIX_FILE} "six"
        if test $? -ne 0 ; then
            return 1
        fi

        extract_py_module ${PYTZ_BUILD_DIR} ${PYTZ_FILE} "pytz"
        if test $? -ne 0 ; then
            return 1
        fi
    fi

    extract_py_module ${MARKUPSAFE_BUILD_DIR} ${MARKUPSAFE_FILE} "markupsafe"
    if test $? -ne 0 ; then
        return 1
    fi

    if [[ "$DO_PYTHON39" == "yes" ]] ; then
        extract_py_module ${ZIPP_BUILD_DIR} ${ZIPP_FILE} "zipp"
        if test $? -ne 0 ; then
            return 1
        fi

        extract_py_module ${IMPORTLIB_METADATA_BUILD_DIR} ${IMPORTLIB_METADATA_FILE} "importlib-metadata"
        if test $? -ne 0 ; then
            return 1
        fi
    fi

    extract_py_module ${SPHINX_BUILD_DIR} ${SPHINX_FILE} "sphinx"
    if test $? -ne 0 ; then
        return 1
    fi

    # patch
    SED_CMD="sed -i "
    if [[ "$OPSYS" == "Darwin" ]]; then
        SED_CMD="sed -i '' " # the intention of this sed command is foiled by shell variable expansion
    fi
    pushd $SPHINX_BUILD_DIR > /dev/null
    ${SED_CMD} "s/docutils>=0.12/docutils<0.16,>=0.12/" ./Sphinx.egg-info/requires.txt
    ${SED_CMD} "s/docutils>=0.12/docutils<0.16,>=0.12/" ./setup.py
    popd > /dev/null




    if [[ "$DO_PYTHON39" == "no" ]] ; then
        install_py_module ${SIX_BUILD_DIR} "six"
        if test $? -ne 0 ; then
            return 1
        fi

        # pyparsing may already have been installed by function bv_python_build
        check_if_py_module_installed "pyparsing"
        if [[ $? != 0 ]] ; then
            info "Building the pyparsing module"

            build_pyparsing
            if [[ $? != 0 ]] ; then
                error "pyparsing build failed. Bailing out."
            fi
            info "Done building the pyparsing module."
        fi
    fi

    install_py_module ${PACKAGING_BUILD_DIR} "packaging"
    if test $? -ne 0 ; then
        return 1
    fi

    if [[ "$DO_PYTHON39" == "no" ]] ; then
        # may have been installed with numpy
        check_if_py_module_installed "setuptools"
        if test $? -ne 0 ; then
            install_py_module ${SETUPTOOLS_BUILD_DIR} "setuptools"
            if test $? -ne 0 ; then
                return 1
            fi
        fi
    fi

    install_py_module ${IMAGESIZE_BUILD_DIR} "imagesize"
    if test $? -ne 0 ; then
        return 1
    fi

    install_py_module ${ALABASTER_BUILD_DIR} "alabaster"
    if test $? -ne 0 ; then
        return 1
    fi

    if [[ "$DO_PYTHON39" == "no" ]] ; then
        install_py_module ${PYTZ_BUILD_DIR} "pytz"
        if test $? -ne 0 ; then
            return 1
        fi
    fi

    install_py_module ${BABEL_BUILD_DIR} "babel"
    if test $? -ne 0 ; then
        return 1
    fi

    install_py_module ${SNOWBALLSTEMMER_BUILD_DIR} "snowballstemmer"
    if test $? -ne 0 ; then
        return 1
    fi

    install_py_module ${DOCUTILS_BUILD_DIR} "docutils"
    if test $? -ne 0 ; then
        return 1
    fi

    install_py_module ${PYGMENTS_BUILD_DIR} "pygments"
    if test $? -ne 0 ; then
        return 1
    fi

    install_py_module ${MARKUPSAFE_BUILD_DIR} "markupsafe"
    if test $? -ne 0 ; then
        return 1
    fi

    install_py_module ${JINJA2_BUILD_DIR} "jinja2"
    if test $? -ne 0 ; then
        return 1
    fi

    install_py_module ${SPHINXCONTRIB_QTHELP_BUILD_DIR} "sphinxcontrib-qthelp"
    if test $? -ne 0 ; then
        return 1
    fi

    install_py_module ${SPHINXCONTRIB_SERIALIZINGHTML_BUILD_DIR} "sphinxcontrib-serializinghtml"
    if test $? -ne 0 ; then
        return 1
    fi

    install_py_module ${SPHINXCONTRIB_HTMLHELP_BUILD_DIR} "sphinxcontrib-htmlhelp"
    if test $? -ne 0 ; then
        return 1
    fi

    install_py_module ${SPHINXCONTRIB_JSMATH_BUILD_DIR} "sphinxcontrib-jsmath"
    if test $? -ne 0 ; then
        return 1
    fi

    install_py_module ${SPHINXCONTRIB_DEVHELP_BUILD_DIR} "sphinxcontrib-devhelp"
    if test $? -ne 0 ; then
        return 1
    fi

    install_py_module ${SPHINXCONTRIB_APPLEHELP_BUILD_DIR} "sphinxcontrib-applehelp"
    if test $? -ne 0 ; then
        return 1
    fi

    if [[ "$DO_PYTHON39" == "yes" ]] ; then
        install_py_module ${ZIPP_BUILD_DIR} "zipp"
        if test $? -ne 0 ; then
            return 1
        fi

        install_py_module ${IMPORTLIB_METADATA_BUILD_DIR} "importlib-metadata"
        if test $? -ne 0 ; then
            return 1
        fi
    fi

    install_py_module ${SPHINX_BUILD_DIR} "sphinx"
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
    if [[ "$DO_PYTHON39" == "yes" ]] ; then
        download_py_module ${SPHINXCONTRIB_JQUERY_FILE} ${SPHINXCONTRIB_JQUERY_URL}
        if [[ $? != 0 ]] ; then
            return 1
        fi

        extract_py_module ${SPHINXCONTRIB_JQUERY_BUILD_DIR} ${SPHINXCONTRIB_JQUERY_FILE} "sphinxcontrib-jquery"
        if [[ $? != 0 ]] ; then
            return 1
        fi

        install_py_module ${SPHINXCONTRIB_JQUERY_BUILD_DIR} "sphinxcontrib-jquery"
        if [[ $? != 0 ]] ; then
            return 1
        fi
    fi


    download_py_module ${SPHINX_RTD_THEME_FILE} ${SPHINX_RTD_THEME_URL}
    if [[ $? != 0 ]] ; then
        return 1
    fi

    extract_py_module ${SPHINX_RTD_THEME_BUILD_DIR} ${SPHINX_RTD_THEME_FILE} "sphinx_rtd_theme"
    if [[ $? != 0 ]] ; then
        return 1
    fi

    install_py_module ${SPHINX_RTD_THEME_BUILD_DIR} "sphinx_rtd_theme"
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
    download_py_module ${SPHINX_TABS_FILE} ${SPHINX_TABS_URL}
    if [[ $? != 0 ]] ; then
        return 1
    fi

    extract_py_module ${SPHINX_TABS_BUILD_DIR} ${SPHINX_TABS_FILE} "sphinx-tabs"
    if [[ $? != 0 ]] ; then
        return 1
    fi

    install_py_module ${SPHINX_TABS_BUILD_DIR} "sphinx-tabs"
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

    check_if_py_module_installed "pyparsing"
    if [[ $? != 0 ]] ; then
        if [[ $PY_CHECK_ECHO != 0 ]] ; then
            info "python module pyparsing is not installed"
        fi
        PY_OK=0
    fi

    if [[ "$BUILD_SPHINX" == "yes" ]]; then

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
    
    if [[ "$BUILD_MPI4PY" == "yes" ]]; then

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

            if [[ "$BUILD_MPI4PY" == "yes" ]]; then

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

            check_if_py_module_installed "pyparsing"
            if [[ $? != 0 ]] ; then
                info "Building the pyparsing module"

                build_pyparsing
                if [[ $? != 0 ]] ; then
                    error "pyparsing build failed. Bailing out."
                fi
                info "Done building the pyparsing module."
            fi

            if [[ "$BUILD_SPHINX" == "yes" ]]; then
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
