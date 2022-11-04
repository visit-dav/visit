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
    if [[ "$DO_PYTHON2" == "yes" ]] ; then
        PYTHON_COMMAND="${VISIT_PYTHON_DIR}/bin/python"
    else
        PYTHON_COMMAND="${VISIT_PYTHON_DIR}/bin/python3"
    fi

    echo "import ${MOD_NAME}; print(${MOD_NAME})" | ${PYTHON_COMMAND}

    if [[ $? != 0 ]] ; then
        return 1
    fi

    return 0
}




function bv_python_initialize
{
    export DO_PYTHON="yes"
    export DO_PYTHON2="no"
    export FORCE_PYTHON="no"
    export USE_SYSTEM_PYTHON="no"
    export BUILD_MPI4PY="no"
    export BUILD_SPHINX="yes"
    export VISIT_PYTHON_DIR=${VISIT_PYTHON_DIR:-""}
    add_extra_commandline_args "python" "system-python" 0 "Using system python"
    add_extra_commandline_args "python" "alt-python-dir" 1 "Using alternate python directory"
    add_extra_commandline_args "python" "mpi4py" 0 "Build mpi4py"
    add_extra_commandline_args "python" "no-sphinx" 0 "Disable building sphinx"
    add_extra_commandline_args "python" "use-python2" 0 "Use Python 2 instead of Python 3"
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
    if [[ "$DO_PYTHON2" == "yes" ]] ; then
        PYTHON_LIBRARY=`"$PYTHON_CONFIG_COMMAND" --libs`
    else
        PYTHON_VERSION_MINOR=`echo $PYTHON_VERSION | cut -d. -f2`
        if [[ $PYTHON_VERSION_MINOR -ge 8 ]] ; then
            PYTHON_LIBRARY=`"$PYTHON_CONFIG_COMMAND" --libs --embed`
        else
            PYTHON_LIBRARY=`"$PYTHON_CONFIG_COMMAND" --libs`
        fi
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

    if [[ $DO_PYTHON2 == "no" ]]; then
        # prefer python3 
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
    else
        # only try python 2
        TEST=`which python-config`
        [ $? != 0 ] && error "System python-config found, cannot configure python"
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

function bv_python_use_python2
{
    info "configuring to build python 2 (NOT python 3)"
    export DO_PYTHON2="yes"
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

    if [[ $DO_PYTHON2 == "no" ]]; then
        # prefer python3
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
    else
        # only try python2
        if [ -e "$1/bin/python-config" ]
        then
            PYTHON_COMMAND="$1/bin/python"
            PYTHON_CONFIG_COMMAND="$1/bin/python-config"
        else
            error "Python (python-config) not found in $1"
        fi
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

    # IMPORANT NOTE: 
    # We don't use export VAR={VAR:-"value"} style of init here b/c
    # it undermines us choosing between python 2 and python 3
    
    # if python 2
    if [[ "$DO_PYTHON2" == "yes" ]] ; then
        export PYTHON_URL="https://www.python.org/ftp/python/2.7.14"
        export PYTHON_FILE_SUFFIX="tgz"
        export PYTHON_VERSION="2.7.14"
        export PYTHON_COMPATIBILITY_VERSION="2.7"
        export PYTHON_FILE="Python-$PYTHON_VERSION.$PYTHON_FILE_SUFFIX"
        export PYTHON_BUILD_DIR="Python-$PYTHON_VERSION"
        export PYTHON_MD5_CHECKSUM="cee2e4b33ad3750da77b2e85f2f8b724"
        export PYTHON_SHA256_CHECKSUM="304c9b202ea6fbd0a4a8e0ad3733715fbd4749f2204a9173a58ec53c32ea73e8"
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

    if [[ "$DO_PYTHON2" == "yes" ]] ; then
        export PILLOW_URL="https://files.pythonhosted.org/packages/b3/d0/a20d8440b71adfbf133452d4f6e0fe80de2df7c2578c9b498fb812083383/"
        export PILLOW_FILE="Pillow-6.2.2.tar.gz"
        export PILLOW_BUILD_DIR="Pillow-6.2.2"
        export PILLOW_MD5_CHECKSUM="46cad14f0044a5ac4b2d801271528893"
        export PILLOW_SHA256_CHECKSUM="db9ff0c251ed066d367f53b64827cc9e18ccea001b986d08c265e53625dab950"
    else
        # python 3 Pillow, not PIL -- we need Pillow for Python 3 Support
        export PILLOW_URL=${PIL_URL:-"https://files.pythonhosted.org/packages/ce/ef/e793f6ffe245c960c42492d0bb50f8d14e2ba223f1922a5c3c81569cec44/"}
        export PILLOW_FILE="Pillow-7.1.2.tar.gz"
        export PILLOW_BUILD_DIR="Pillow-7.1.2"
        export PILLOW_MD5_CHECKSUM="f1f7592c51260e5080d3cd71781ea675"
        export PILLOW_SHA256_CHECKSUM="a0b49960110bc6ff5fead46013bcb8825d101026d466f3a4de3476defe0fb0dd"
    fi

    if [[ "$DO_PYTHON2" == "yes" ]] ; then
        export PYPARSING_URL=""
        export PYPARSING_FILE="pyparsing-1.5.2.tar.gz"
        export PYPARSING_BUILD_DIR="pyparsing-1.5.2"
        export PYPARSING_MD5_CHECKSUM="13aed3cb21a427f8aeb0fe7ca472ba42"
        export PYPARSING_SHA256_CHECKSUM="1021fd2cfdf9c3b6ac0191b018c15d591501b77d977baded59d8ef76d375f21c"
    else
        export PYPARSING_URL=""
        export PYPARSING_FILE="pyparsing-2.4.6.tar.gz"
        export PYPARSING_BUILD_DIR="pyparsing-2.4.6"
        export PYPARSING_MD5_CHECKSUM="29733ea8cbee0291aad121c69c6e51a1"
        export PYPARSING_SHA256_CHECKSUM="4c830582a84fb022400b85429791bc551f1f4871c33f23e44f353119e92f969f"
    fi

    if [[ "$DO_PYTHON2" == "yes" ]] ; then
        export PYREQUESTS_URL=""
        export PYREQUESTS_FILE="requests-2.5.1.tar.gz"
        export PYREQUESTS_BUILD_DIR="requests-2.5.1"
        export PYREQUESTS_MD5_CHECKSUM="a89558d5dd35a5cb667e9a6e5d4f06f1"
        export PYREQUESTS_SHA256_CHECKSUM="1e5ea203d49273be90dcae2b98120481b2ecfc9f2ae512ce545baab96f57b58c"
    else
        export PYREQUESTS_URL=""
        export PYREQUESTS_FILE="requests-2.22.0.tar.gz"
        export PYREQUESTS_BUILD_DIR="requests-2.22.0"
        export PYREQUESTS_MD5_CHECKSUM="ee28bee2de76e9198fc41e48f3a7dd47"
        export PYREQUESTS_SHA256_CHECKSUM="11e007a8a2aa0323f5a921e9e6a2d7e4e67d9877e85773fba9ba6419025cbeb4"
    fi

    if [[ "$DO_PYTHON2" == "yes" ]] ; then
        export SETUPTOOLS_URL="https://pypi.python.org/packages/f7/94/eee867605a99ac113c4108534ad7c292ed48bf1d06dfe7b63daa51e49987/"
        export SETUPTOOLS_FILE="setuptools-28.0.0.tar.gz"
        export SETUPTOOLS_BUILD_DIR="setuptools-28.0.0"
        export SETUPTOOLS_MD5_CHECKSUM="9b23df90e1510c7353a5cf07873dcd22"
        export SETUPTOOLS_SHA256_CHECKSUM="e1a2850bb7ad820e4dd3643a6c597bea97a35de2909e9bf0afa3f337836b5ea3"
    else
        export SETUPTOOLS_URL=""
        export SETUPTOOLS_FILE="setuptools-44.0.0.zip"
        export SETUPTOOLS_BUILD_DIR="setuptools-44.0.0"
        export SETUPTOOLS_MD5_CHECKSUM="32b6cdce670ce462086d246bea181e9d"
        export SETUPTOOLS_SHA256_CHECKSUM="e5baf7723e5bb8382fc146e33032b241efc63314211a3a120aaa55d62d2bb008"
    fi

    if [[ "$DO_PYTHON2" == "yes" ]] ; then
        export CYTHON_URL="https://pypi.python.org/packages/c6/fe/97319581905de40f1be7015a0ea1bd336a756f6249914b148a17eefa75dc/"
        export CYTHON_FILE="Cython-0.25.2.tar.gz"
        export CYTHON_BUILD_DIR="Cython-0.25.2"
        export CYTHON_MD5_CHECKSUM="642c81285e1bb833b14ab3f439964086"
        export CYTHON_SHA256_CHECKSUM="f141d1f9c27a07b5a93f7dc5339472067e2d7140d1c5a9e20112a5665ca60306"
    else
        export CYTHON_URL="https://files.pythonhosted.org/packages/99/36/a3dc962cc6d08749aa4b9d85af08b6e354d09c5468a3e0edc610f44c856b/"
        export CYTHON_FILE="Cython-0.29.17.tar.gz"
        export CYTHON_BUILD_DIR="Cython-0.29.17"
        export CYTHON_MD5_CHECKSUM="0936311ccd09f1164ab2f46ca5cd8c3b"
        export CYTHON_SHA256_CHECKSUM="6361588cb1d82875bcfbad83d7dd66c442099759f895cf547995f00601f9caf2"
    fi

    if [[ "$DO_PYTHON2" == "yes" ]] ; then
        export NUMPY_URL="https://pypi.python.org/packages/a3/99/74aa456fc740a7e8f733af4e8302d8e61e123367ec660cd89c53a3cd4d70/"
        export NUMPY_FILE="numpy-1.14.1.zip"
        export NUMPY_BUILD_DIR="numpy-1.14.1"
        export NUMPY_MD5_CHECKSUM="b8324ef90ac9064cd0eac46b8b388674"
        export NUMPY_SHA256_CHECKSUM="fa0944650d5d3fb95869eaacd8eedbd2d83610c85e271bd9d3495ffa9bc4dc9c"
    else
        export NUMPY_URL="https://github.com/numpy/numpy/releases/download/v1.16.6/"
        export NUMPY_FILE="numpy-1.16.6.zip"
        export NUMPY_BUILD_DIR="numpy-1.16.6"
        export NUMPY_MD5_CHECKSUM="3dc21c84a295fe77eadf8f872685a7de"
        export NUMPY_SHA256_CHECKSUM="e5cf3fdf13401885e8eea8170624ec96225e2174eb0c611c6f26dd33b489e3ff"
    fi

    export MPI4PY_URL="https://pypi.python.org/pypi/mpi4py"
    export MPI4PY_FILE="mpi4py-2.0.0.tar.gz"
    export MPI4PY_BUILD_DIR="mpi4py-2.0.0"
    export MPI4PY_MD5_CHECKSUM="4f7d8126d7367c239fd67615680990e3"
    export MPI4PY_SHA256_CHECKSUM="6543a05851a7aa1e6d165e673d422ba24e45c41e4221f0993fe1e5924a00cb81"

    export PACKAGING_URL=""
    export PACKAGING_FILE="packaging-19.2.tar.gz"
    export PACKAGING_BUILD_DIR="packaging-19.2"
    export PACKAGING_MD5_CHECKSUM="867ce70984dc7b89bbbc3cac2a72b171"
    export PACKAGING_SHA256_CHECKSUM="28b924174df7a2fa32c1953825ff29c61e2f5e082343165438812f00d3a7fc47"

    export IMAGESIZE_URL=""
    export IMAGESIZE_FILE="imagesize-1.1.0.tar.gz"
    export IMAGESIZE_BUILD_DIR="imagesize-1.1.0"
    export IMAGESIZE_MD5_CHECKSUM="2f89749b05e07c79c46330dbc62f1e02"
    export IMAGESIZE_SHA256_CHECKSUM="f3832918bc3c66617f92e35f5d70729187676313caa60c187eb0f28b8fe5e3b5"

    export ALABASTER_URL=""
    export ALABASTER_FILE="alabaster-0.7.12.tar.gz"
    export ALABASTER_BUILD_DIR="alabaster-0.7.12"
    export ALABASTER_MD5_CHECKSUM="3591827fde96d1dd23970fb05410ed04"
    export ALABASTER_SHA256_CHECKSUM="a661d72d58e6ea8a57f7a86e37d86716863ee5e92788398526d58b26a4e4dc02"

    export BABEL_URL=""
    export BABEL_FILE="Babel-2.7.0.tar.gz"
    export BABEL_BUILD_DIR="Babel-2.7.0"
    export BABEL_MD5_CHECKSUM="83c158b7dae9135750a7cf204e6e2eea"
    export BABEL_SHA256_CHECKSUM="e86135ae101e31e2c8ec20a4e0c5220f4eed12487d5cf3f78be7e98d3a57fc28"

    export SNOWBALLSTEMMER_URL=""
    export SNOWBALLSTEMMER_FILE="snowballstemmer-2.0.0.tar.gz"
    export SNOWBALLSTEMMER_BUILD_DIR="snowballstemmer-2.0.0"
    export SNOWBALLSTEMMER_MD5_CHECKSUM="c05ec4a897be3c953c8b8b844c4241d4"
    export SNOWBALLSTEMMER_SHA256_CHECKSUM="df3bac3df4c2c01363f3dd2cfa78cce2840a79b9f1c2d2de9ce8d31683992f52"

    export DOCUTILS_URL=""
    export DOCUTILS_FILE="docutils-0.15.2.tar.gz"
    export DOCUTILS_BUILD_DIR="docutils-0.15.2"
    export DOCUTILS_MD5_CHECKSUM="e26a308d8000b0bed7416a633217c676"
    export DOCUTILS_SHA256_CHECKSUM="a2aeea129088da402665e92e0b25b04b073c04b2dce4ab65caaa38b7ce2e1a99"

    export PYGMENTS_URL=""
    export PYGMENTS_FILE="Pygments-2.5.2.tar.gz"
    export PYGMENTS_BUILD_DIR="Pygments-2.5.2"
    export PYGMENTS_MD5_CHECKSUM="465a35559863089d959d783a69f79b9f"
    export PYGMENTS_SHA256_CHECKSUM="98c8aa5a9f778fcd1026a17361ddaf7330d1b7c62ae97c3bb0ae73e0b9b6b0fe"

    export JINJA2_URL=""
    export JINJA2_FILE="Jinja2-2.10.3.tar.gz"
    export JINJA2_BUILD_DIR="Jinja2-2.10.3"
    export JINJA2_MD5_CHECKSUM="7883559bc5cc3e2781d94b4be61cfdcd"
    export JINJA2_SHA256_CHECKSUM="9fe95f19286cfefaa917656583d020be14e7859c6b0252588391e47db34527de"

    export SPHINXCONTRIB_QTHELP_URL=""
    export SPHINXCONTRIB_QTHELP_FILE="sphinxcontrib-qthelp-1.0.2.tar.gz"
    export SPHINXCONTRIB_QTHELP_BUILD_DIR="sphinxcontrib-qthelp-1.0.2"
    export SPHINXCONTRIB_QTHELP_MD5_CHECKSUM="3532d4643d0b1cc3806e43f59495c030"
    export SPHINXCONTRIB_QTHELP_SHA256_CHECKSUM="79465ce11ae5694ff165becda529a600c754f4bc459778778c7017374d4d406f"

    export SPHINXCONTRIB_SERIALIZINGHTML_URL=""
    export SPHINXCONTRIB_SERIALIZINGHTML_FILE="sphinxcontrib-serializinghtml-1.1.3.tar.gz"
    export SPHINXCONTRIB_SERIALIZINGHTML_BUILD_DIR="sphinxcontrib-serializinghtml-1.1.3"
    export SPHINXCONTRIB_SERIALIZINGHTML_MD5_CHECKSUM="6a4318d6d11c345fbc669e6a86f32766"
    export SPHINXCONTRIB_SERIALIZINGHTML_SHA256_CHECKSUM="c0efb33f8052c04fd7a26c0a07f1678e8512e0faec19f4aa8f2473a8b81d5227"

    export SPHINXCONTRIB_HTMLHELP_URL=""
    export SPHINXCONTRIB_HTMLHELP_FILE="sphinxcontrib-htmlhelp-1.0.2.tar.gz"
    export SPHINXCONTRIB_HTMLHELP_BUILD_DIR="sphinxcontrib-htmlhelp-1.0.2"
    export SPHINXCONTRIB_HTMLHELP_MD5_CHECKSUM="f72e4b26ec0f6387d855c74819500b66"
    export SPHINXCONTRIB_HTMLHELP_SHA256_CHECKSUM="4670f99f8951bd78cd4ad2ab962f798f5618b17675c35c5ac3b2132a14ea8422"

    export SPHINXCONTRIB_JSMATH_URL=""
    export SPHINXCONTRIB_JSMATH_FILE="sphinxcontrib-jsmath-1.0.1.tar.gz"
    export SPHINXCONTRIB_JSMATH_BUILD_DIR="sphinxcontrib-jsmath-1.0.1"
    export SPHINXCONTRIB_JSMATH_MD5_CHECKSUM="e45179f0a3608b6766862e0f34c23b62"
    export SPHINXCONTRIB_JSMATH_SHA256_CHECKSUM="a9925e4a4587247ed2191a22df5f6970656cb8ca2bd6284309578f2153e0c4b8"

    export SPHINXCONTRIB_DEVHELP_URL=""
    export SPHINXCONTRIB_DEVHELP_FILE="sphinxcontrib-devhelp-1.0.1.tar.gz"
    export SPHINXCONTRIB_DEVHELP_BUILD_DIR="sphinxcontrib-devhelp-1.0.1"
    export SPHINXCONTRIB_DEVHELP_MD5_CHECKSUM="ecb33259e2e8300493d210140af7d957"
    export SPHINXCONTRIB_DEVHELP_SHA256_CHECKSUM="6c64b077937330a9128a4da74586e8c2130262f014689b4b89e2d08ee7294a34"

    export SPHINXCONTRIB_APPLEHELP_URL=""
    export SPHINXCONTRIB_APPLEHELP_FILE="sphinxcontrib-applehelp-1.0.1.tar.gz"
    export SPHINXCONTRIB_APPLEHELP_BUILD_DIR="sphinxcontrib-applehelp-1.0.1"
    export SPHINXCONTRIB_APPLEHELP_MD5_CHECKSUM="c3424507cc28291f8005081b6a96afb1"
    export SPHINXCONTRIB_APPLEHELP_SHA256_CHECKSUM="edaa0ab2b2bc74403149cb0209d6775c96de797dfd5b5e2a71981309efab3897"

    export SIX_URL=""
    export SIX_FILE="six-1.13.0.tar.gz"
    export SIX_BUILD_DIR="six-1.13.0"
    export SIX_MD5_CHECKSUM="e92c23c882c7d5564ce5773fe31b2771"
    export SIX_SHA256_CHECKSUM="30f610279e8b2578cab6db20741130331735c781b56053c59c4076da27f06b66"

    export URLLIB3_URL=""
    export URLLIB3_FILE="urllib3-1.25.7.tar.gz"
    export URLLIB3_BUILD_DIR="urllib3-1.25.7"
    export URLLIB3_MD5_CHECKSUM="85e1e3925f8c1095172bff343f3312ed"
    export URLLIB3_SHA256_CHECKSUM="f3c5fd51747d450d4dcf6f923c81f78f811aab8205fda64b0aba34a4e48b0745"

    export IDNA_URL=""
    export IDNA_FILE="idna-2.8.tar.gz"
    export IDNA_BUILD_DIR="idna-2.8"
    export IDNA_MD5_CHECKSUM="2e9ae0b4a0b26d1747c6127cdb060bc1"
    export IDNA_SHA256_CHECKSUM="c357b3f628cf53ae2c4c05627ecc484553142ca23264e593d327bcde5e9c3407"

    export CHARDET_URL=""
    export CHARDET_FILE="chardet-3.0.4.tar.gz"
    export CHARDET_BUILD_DIR="chardet-3.0.4"
    export CHARDET_MD5_CHECKSUM="7dd1ba7f9c77e32351b0a0cfacf4055c"
    export CHARDET_SHA256_CHECKSUM="84ab92ed1c4d4f16916e05906b6b75a6c0fb5db821cc65e70cbd64a3e2a5eaae"

    export CERTIFI_URL=""
    export CERTIFI_FILE="certifi-2019.11.28.tar.gz"
    export CERTIFI_BUILD_DIR="certifi-2019.11.28"
    export CERTIFI_MD5_CHECKSUM="4d5229c4d9f0a4a79106f9e2c2cfd381"
    export CERTIFI_SHA256_CHECKSUM="25b64c7da4cd7479594d035c08c2d809eb4aab3a26e5a990ea98cc450c320f1f"

    export PYTZ_URL=""
    export PYTZ_FILE="pytz-2019.3.tar.gz"
    export PYTZ_BUILD_DIR="pytz-2019.3"
    export PYTZ_MD5_CHECKSUM="c3d84a465fc56a4edd52cca8873ac0df"
    export PYTZ_SHA256_CHECKSUM="b02c06db6cf09c12dd25137e563b31700d3b80fcc4ad23abb7a315f2789819be"

    export MARKUPSAFE_URL=""
    export MARKUPSAFE_FILE="MarkupSafe-1.1.1.tar.gz"
    export MARKUPSAFE_BUILD_DIR="MarkupSafe-1.1.1"
    export MARKUPSAFE_MD5_CHECKSUM="43fd756864fe42063068e092e220c57b"
    export MARKUPSAFE_SHA256_CHECKSUM="29872e92839765e546828bb7754a68c418d927cd064fd4708fab9fe9c8bb116b"

    export SPHINX_URL="https://files.pythonhosted.org/packages/f6/3a/c51fc285c0c5c30bcd9426bf096187840683d9383df716a6b6a4ca0a8bde"
    export SPHINX_FILE="Sphinx-2.2.1.tar.gz"
    export SPHINX_BUILD_DIR="Sphinx-2.2.1"
    export SPHINX_MD5_CHECKSUM="60ea892a09b463e5ecb6ea26d2470f36"
    export SPHINX_SHA256_CHECKSUM="31088dfb95359384b1005619827eaee3056243798c62724fd3fa4b84ee4d71bd"

    export SPHINX_RTD_URL="https://files.pythonhosted.org/packages/ed/73/7e550d6e4cf9f78a0e0b60b9d93dba295389c3d271c034bf2ea3463a79f9"
    export SPHINX_RTD_FILE="sphinx_rtd_theme-0.4.3.tar.gz"
    export SPHINX_RTD_BUILD_DIR="sphinx_rtd_theme-0.4.3"
    export SPHINX_RTD_MD5_CHECKSUM="6c50f30bc39046f497d336039a0c13fa"
    export SPHINX_RTD_SHA256_CHECKSUM="728607e34d60456d736cc7991fd236afb828b21b82f956c5ea75f94c8414040a"
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
    printf "%-20s %s [%s]\n" "--use-python2" "Build Python 2 instead of Python 3"
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
        if [[ "$DO_PYTHON2" == "yes" ]] ; then
            export PYTHON_COMMAND="${VISIT_PYTHON_DIR}/bin/python"
        else
            export PYTHON_COMMAND="${VISIT_PYTHON_DIR}/bin/python3"
        fi
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

    return 0
}

function apply_python_pillow_patch
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

    if [[ "$DO_GROUP" == "yes" ]] ; then
        chmod -R ug+w,a+rX "$VISITDIR/python"
        chgrp -R ${GROUP} "$VISITDIR/python"
    fi
    cd "$START_DIR"
    info "Done with Python"

    return 0
}

# *************************************************************************** #
#                            Function 7.1, build_pillow                       #
# *************************************************************************** #
function build_pillow
{
    if ! test -f ${PILLOW_FILE} ; then
        download_file ${PILLOW_FILE} "${PILLOW_URL}"
        if [[ $? != 0 ]] ; then
            warn "Could not download ${PILLOW_FILE}"
            return 1
        fi
    fi
    if ! test -d ${PILLOW_BUILD_DIR} ; then
        info "Extracting Pillow ..."
        uncompress_untar ${PILLOW_FILE}
        if test $? -ne 0 ; then
            warn "Could not extract ${PILLOW_FILE}"
            return 1
        fi
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
    apply_python_pillow_patch
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
    if [[ "$DO_GROUP" == "yes" ]] ; then
        chmod -R ug+w,a+rX "$VISITDIR/python"
        chgrp -R ${GROUP} "$VISITDIR/python"
    fi

    info "Done with Pillow."
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

    pushd $PYPARSING_BUILD_DIR > /dev/null
    info "Installing pyparsing ..."
    ${PYTHON_COMMAND} ./setup.py install --prefix="${PYHOME}"
    if test $? -ne 0 ; then
        popd > /dev/null
        warn "Could not install pyparsing"
        return 1
    fi
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

    if [[ "$DO_PYTHON2" == "no" ]] ; then
        # python 3: Requests depends on certifi, urllib3, idna, chardet
        if ! test -f ${CERTIFI_FILE} ; then
            download_file ${CERTIFI_FILE} "${CERTIFI_URL}"
            if [[ $? != 0 ]] ; then
                warn "Could not download ${CERTIFI_FILE}"
                return 1
            fi
        fi

        if ! test -f ${URLLIB3_FILE} ; then
            download_file ${URLLIB3_FILE} "${URLLIB3_URL}"
            if [[ $? != 0 ]] ; then
                warn "Could not download ${URLLIB3_FILE}"
                return 1
            fi
        fi

        if ! test -f ${IDNA_FILE} ; then
            download_file ${IDNA_FILE} "${IDNA_URL}"
            if [[ $? != 0 ]] ; then
                warn "Could not download ${IDNA_FILE}"
                return 1
            fi
        fi

        if ! test -f ${CHARDET_FILE} ; then
            download_file ${CHARDET_FILE} "${CHARDET_URL}"
            if [[ $? != 0 ]] ; then
                warn "Could not download ${CHARDET_FILE}"
                return 1
            fi
        fi

        if ! test -d ${CERTIFI_BUILD_DIR} ; then
            info "Extracting certifi ..."
            uncompress_untar ${CERTIFI_FILE}
            if test $? -ne 0 ; then
                warn "Could not extract ${CERTIFI_FILE}"
                return 1
            fi
        fi

        if ! test -d ${URLLIB3_BUILD_DIR} ; then
            info "Extracting urllib3 ..."
            uncompress_untar ${URLLIB3_FILE}
            if test $? -ne 0 ; then
                warn "Could not extract ${URLLIB3_FILE}"
                return 1
            fi
        fi

        if ! test -d ${IDNA_BUILD_DIR} ; then
            info "Extracting idna ..."
            uncompress_untar ${IDNA_FILE}
            if test $? -ne 0 ; then
                warn "Could not extract ${IDNA_FILE}"
                return 1
            fi
        fi

        if ! test -d ${CHARDET_BUILD_DIR} ; then
            info "Extracting chardet ..."
            uncompress_untar ${CHARDET_FILE}
            if test $? -ne 0 ; then
                warn "Could not extract ${CHARDET_FILE}"
                return 1
            fi
        fi

        pushd $CERTIFI_BUILD_DIR > /dev/null
        info "Installing certifi ..."
        ${PYTHON_COMMAND} ./setup.py install --prefix="${PYHOME}"
        if test $? -ne 0 ; then
            popd > /dev/null
            warn "Could not install certifi"
            return 1
        fi
        popd > /dev/null

        pushd $URLLIB3_BUILD_DIR > /dev/null
        info "Installing urllib3 ..."
        ${PYTHON_COMMAND} ./setup.py install --prefix="${PYHOME}"
        if test $? -ne 0 ; then
            popd > /dev/null
            warn "Could not install urllib3"
            return 1
        fi
        popd > /dev/null

        pushd $IDNA_BUILD_DIR > /dev/null
        info "Installing idna ..."
        ${PYTHON_COMMAND} ./setup.py install --prefix="${PYHOME}"
        if test $? -ne 0 ; then
            popd > /dev/null
            warn "Could not install idna"
            return 1
        fi
        popd > /dev/null

        pushd $CHARDET_BUILD_DIR > /dev/null
        info "Installing chardet ..."
        ${PYTHON_COMMAND} ./setup.py install --prefix="${PYHOME}"
        if test $? -ne 0 ; then
            popd > /dev/null
            warn "Could not install chardet"
            return 1
        fi
        popd > /dev/null
    fi

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

    pushd $PYREQUESTS_BUILD_DIR > /dev/null
    info "Installing python requests module ..."
    ${PYTHON_COMMAND} ./setup.py install --prefix="${PYHOME}"
    if test $? -ne 0 ; then
        popd > /dev/null
        warn "Could not install requests module"
        return 1
    fi
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
#                                  build_mpi4py                               #
# *************************************************************************** #
function build_mpi4py
{
    # download
    if ! test -f ${MPI4PY_FILE} ; then
        download_file ${MPI4PY_FILE}
        if [[ $? != 0 ]] ; then
            warn "Could not download ${MPI4PY_FILE}"
            return 1
        fi
    fi

    # extract
    if ! test -d ${MPI4PY_BUILD_DIR} ; then
        info "Extracting mpi4py ..."
        uncompress_untar ${MPI4PY_FILE}
        if test $? -ne 0 ; then
            warn "Could not extract ${MPI4PY_FILE}"
            return 1
        fi
    fi

    # install
    pushd $MPI4PY_BUILD_DIR > /dev/null
    info "Installing mpi4py (~ 2 min) ..."
    ${PYTHON_COMMAND} ./setup.py install --prefix="${PYHOME}"
    if test $? -ne 0 ; then
        popd > /dev/null
        warn "Could not install mpi4py"
        return 1
    fi
    popd > /dev/null

    # fix the perms
    if [[ "$DO_GROUP" == "yes" ]] ; then
        chmod -R ug+w,a+rX "$VISITDIR/python"
        chgrp -R ${GROUP} "$VISITDIR/python"
    fi

    return 0
}

# *************************************************************************** #
#                                  build_numpy                                #
# *************************************************************************** #
function build_numpy
{
    # download
    if ! test -f ${SETUPTOOLS_FILE} ; then
        download_file ${SETUPTOOLS_FILE}
        if [[ $? != 0 ]] ; then
            warn "Could not download ${SETUPTOOLS_FILE}"
            return 1
        fi
    fi

    if ! test -f ${CYTHON_FILE} ; then
        download_file ${CYTHON_FILE}
        if [[ $? != 0 ]] ; then
            warn "Could not download ${CYTHON_FILE}"
            return 1
        fi
    fi

    if ! test -f ${NUMPY_FILE} ; then
        download_file ${NUMPY_FILE}
        if [[ $? != 0 ]] ; then
            warn "Could not download ${NUMPY_FILE}"
            return 1
        fi
    fi

    # extract
    if ! test -d ${SETUPTOOLS_BUILD_DIR} ; then
        info "Extracting setuptools ..."
        uncompress_untar ${SETUPTOOLS_FILE}
        if test $? -ne 0 ; then
            warn "Could not extract ${SETUPTOOLS_FILE}"
            return 1
        fi
    fi

    if ! test -d ${CYTHON_BUILD_DIR} ; then
        info "Extracting cython ..."
        uncompress_untar ${CYTHON_FILE}
        if test $? -ne 0 ; then
            warn "Could not extract ${CYTHON_FILE}"
            return 1
        fi
    fi

    if ! test -d ${NUMPY_BUILD_DIR} ; then
        info "Extracting numpy ..."
        uncompress_untar ${NUMPY_FILE}
        if test $? -ne 0 ; then
            warn "Could not extract ${NUMPY_FILE}"
            return 1
        fi
    fi

    # install
    pushd $SETUPTOOLS_BUILD_DIR > /dev/null
    info "Installing setuptools (~1 min) ..."
    ${PYTHON_COMMAND} ./setup.py install --prefix="${PYHOME}"
    if test $? -ne 0 ; then
        popd > /dev/null
        warn "Could not install setuptools"
        return 1
    fi
    popd > /dev/null

    pushd $CYTHON_BUILD_DIR > /dev/null
    info "Installing cython (~ 2 min) ..."
    ${PYTHON_COMMAND} ./setup.py install --prefix="${PYHOME}"
    if test $? -ne 0 ; then
        popd > /dev/null
        warn "Could not install cython"
        return 1
    fi
    popd > /dev/null

    pushd $NUMPY_BUILD_DIR > /dev/null
    cat << \EOF > site.cfg
[openblas]
libraries =
library_dirs =
include_dirs =
EOF
    info "Installing numpy (~ 2 min) ..."
    sed -i 's#\\\\\"%s\\\\\"#%s#' numpy/distutils/system_info.py
    CC=${CMAKE_C_COMPILER} BLAS=None LAPACK=None ATLAS=None ${PYTHON_COMMAND} ./setup.py install --prefix="${PYHOME}"
    if test $? -ne 0 ; then
        popd > /dev/null
        warn "Could not install numpy"
        return 1
    fi
    popd > /dev/null

    # fix the perms
    if [[ "$DO_GROUP" == "yes" ]] ; then
        chmod -R ug+w,a+rX "$VISITDIR/python"
        chgrp -R ${GROUP} "$VISITDIR/python"
    fi

    return 0
}

# *************************************************************************** #
#                                  build_sphinx                               #
# *************************************************************************** #
function build_sphinx
{
    # download
    if ! test -f ${PACKAGING_FILE} ; then
        download_file ${PACKAGING_FILE} "${PACKAGING_URL}"
        if [[ $? != 0 ]] ; then
            warn "Could not download ${PACKAGING_FILE}"
            return 1
        fi
    fi

    if ! test -f ${SETUPTOOLS_FILE} ; then
        download_file ${SETUPTOOLS_FILE} "${SETUPTOOLS_URL}"
        if [[ $? != 0 ]] ; then
            warn "Could not download ${SETUPTOOLS_URL}"
            return 1
        fi
    fi

    if ! test -f ${IMAGESIZE_FILE} ; then
        download_file ${IMAGESIZE_FILE} "${IMAGESIZE_URL}"
        if [[ $? != 0 ]] ; then
            warn "Could not download ${IMAGESIZE_FILE}"
            return 1
        fi
    fi

    if ! test -f ${ALABASTER_FILE} ; then
        download_file ${ALABASTER_FILE} "${ALABASTER_URL}"
        if [[ $? != 0 ]] ; then
            warn "Could not download ${ALABASTER_FILE}"
            return 1
        fi
    fi

    if ! test -f ${BABEL_FILE} ; then
        download_file ${BABEL_FILE} "${BABEL_URL}"
        if [[ $? != 0 ]] ; then
            warn "Could not download ${BABEL_FILE}"
            return 1
        fi
    fi

    if ! test -f ${SNOWBALLSTEMMER_FILE} ; then
        download_file ${SNOWBALLSTEMMER_FILE} "${SNOWBALLSTEMMER_URL}"
        if [[ $? != 0 ]] ; then
            warn "Could not download ${SNOWBALLSTEMMER_FILE}"
            return 1
        fi
    fi

    if ! test -f ${DOCUTILS_FILE} ; then
        download_file ${DOCUTILS_FILE} "${DOCUTILS_URL}"
        if [[ $? != 0 ]] ; then
            warn "Could not download ${DOCUTILS_FILE}"
            return 1
        fi
    fi

    if ! test -f ${PYGMENTS_FILE} ; then
        download_file ${PYGMENTS_FILE} "${PYGMENTS_URL}"
        if [[ $? != 0 ]] ; then
            warn "Could not download ${PYGMENTS_FILE}"
            return 1
        fi
    fi

    if ! test -f ${JINJA2_FILE} ; then
        download_file ${JINJA2_FILE} "${JINJA2_URL}"
        if [[ $? != 0 ]] ; then
            warn "Could not download ${JINJA2_FILE}"
            return 1
        fi
    fi

    if ! test -f ${SPHINXCONTRIB_QTHELP_FILE} ; then
        download_file ${SPHINXCONTRIB_QTHELP_FILE} "${SPHINXCONTRIB_QTHELP_URL}"
        if [[ $? != 0 ]] ; then
            warn "Could not download ${SPHINXCONTRIB_QTHELP_FILE}"
            return 1
        fi
    fi

    if ! test -f ${SPHINXCONTRIB_SERIALIZINGHTML_FILE} ; then
        download_file ${SPHINXCONTRIB_SERIALIZINGHTML_FILE} "${SPHINXCONTRIB_SERIALIZINGHTML_URL}"
        if [[ $? != 0 ]] ; then
            warn "Could not download ${SPHINXCONTRIB_SERIALIZINGHTML_FILE}"
            return 1
        fi
    fi

    if ! test -f ${SPHINXCONTRIB_HTMLHELP_FILE} ; then
        download_file ${SPHINXCONTRIB_HTMLHELP_FILE} "${SPHINXCONTRIB_HTMLHELP_URL}"
        if [[ $? != 0 ]] ; then
            warn "Could not download ${SPHINXCONTRIB_HTMLHELP_FILE}"
            return 1
        fi
    fi

    if ! test -f ${SPHINXCONTRIB_JSMATH_FILE} ; then
        download_file ${SPHINXCONTRIB_JSMATH_FILE} "${SPHINXCONTRIB_JSMATH_URL}"
        if [[ $? != 0 ]] ; then
            warn "Could not download ${SPHINXCONTRIB_JSMATH_FILE}"
            return 1
        fi
    fi

    if ! test -f ${SPHINXCONTRIB_DEVHELP_FILE} ; then
        download_file ${SPHINXCONTRIB_DEVHELP_FILE} "${SPHINXCONTRIB_DEVHELP_URL}"
        if [[ $? != 0 ]] ; then
            warn "Could not download ${SPHINXCONTRIB_DEVHELP_FILE}"
            return 1
        fi
    fi

    if ! test -f ${SPHINXCONTRIB_APPLEHELP_FILE} ; then
        download_file ${SPHINXCONTRIB_APPLEHELP_FILE} "${SPHINXCONTRIB_APPLEHELP_URL}"
        if [[ $? != 0 ]] ; then
            warn "Could not download ${SPHINXCONTRIB_APPLEHELP_FILE}"
            return 1
        fi
    fi

    if ! test -f ${SIX_FILE} ; then
        download_file ${SIX_FILE} "${SIX_URL}"
        if [[ $? != 0 ]] ; then
            warn "Could not download ${SIX_FILE}"
            return 1
        fi
    fi

    if ! test -f ${PYPARSING_FILE} ; then
        download_file ${PYPARSING_FILE} "${PYPARSING_URL}"
        if [[ $? != 0 ]] ; then
            warn "Could not download ${PYPARSING_FILE}"
            return 1
        fi
    fi

    if ! test -f ${PYTZ_FILE} ; then
        download_file ${PYTZ_FILE} "${PYTZ_URL}"
        if [[ $? != 0 ]] ; then
            warn "Could not download ${PYTZ_FILE}"
            return 1
        fi
    fi

    if ! test -f ${MARKUPSAFE_FILE} ; then
        download_file ${MARKUPSAFE_FILE} "${MARKUPSAFE_URL}"
        if [[ $? != 0 ]] ; then
            warn "Could not download ${MARKUPSAFE_FILE}"
            return 1
        fi
    fi

    if ! test -f ${SPHINX_FILE} ; then
        download_file ${SPHINX_FILE} "${SPHINX_URL}"
        if [[ $? != 0 ]] ; then
            warn "Could not download ${SPHINX_FILE}"
            return 1
        fi
    fi

    # extract
    if ! test -d ${PACKAGING_BUILD_DIR} ; then
        info "Extracting packaging ..."
        uncompress_untar ${PACKAGING_FILE}
        if test $? -ne 0 ; then
            warn "Could not extract ${PACKAGING_FILE}"
            return 1
        fi
    fi

    if ! test -d ${SETUPTOOLS_BUILD_DIR} ; then
        info "Extracting setuptools ..."
        uncompress_untar ${SETUPTOOLS_FILE}
        if test $? -ne 0 ; then
            warn "Could not extract ${SETUPTOOLS_FILE}"
            return 1
        fi
    fi

    if ! test -d ${IMAGESIZE_BUILD_DIR} ; then
        info "Extracting imagesize ..."
        uncompress_untar ${IMAGESIZE_FILE}
        if test $? -ne 0 ; then
            warn "Could not extract ${IMAGESIZE_FILE}"
            return 1
        fi
    fi

    if ! test -d ${ALABASTER_BUILD_DIR} ; then
        info "Extracting alabastor ..."
        uncompress_untar ${ALABASTER_FILE}
        if test $? -ne 0 ; then
            warn "Could not extract ${ALABASTER_FILE}"
            return 1
        fi
    fi

    if ! test -d ${BABEL_BUILD_DIR} ; then
        info "Extracting babel ..."
        uncompress_untar ${BABEL_FILE}
        if test $? -ne 0 ; then
            warn "Could not extract ${BABEL_FILE}"
            return 1
        fi
    fi

    if ! test -d ${SNOWBALLSTEMMER_BUILD_DIR} ; then
        info "Extracting snowballstemmer ..."
        uncompress_untar ${SNOWBALLSTEMMER_FILE}
        if test $? -ne 0 ; then
            warn "Could not extract ${SNOWBALLSTEMMER_FILE}"
            return 1
        fi
    fi

    if ! test -d ${DOCUTILS_BUILD_DIR} ; then
        info "Extracting docutils ..."
        uncompress_untar ${DOCUTILS_FILE}
        if test $? -ne 0 ; then
            warn "Could not extract ${DOCUTILS_FILE}"
            return 1
        fi
    fi

    if ! test -d ${PYGMENTS_BUILD_DIR} ; then
        info "Extracting pygments ..."
        uncompress_untar ${PYGMENTS_FILE}
        if test $? -ne 0 ; then
            warn "Could not extract ${PYGMENTS_FILE}"
            return 1
        fi
    fi

    if ! test -d ${JINJA2_BUILD_DIR} ; then
        info "Extracting jinja2 ..."
        uncompress_untar ${JINJA2_FILE}
        if test $? -ne 0 ; then
            warn "Could not extract ${JINJA2_FILE}"
            return 1
        fi
    fi

    if ! test -d ${SPHINXCONTRIB_QTHELP_BUILD_DIR} ; then
        info "Extracting sphinxcontrib-qthelp ..."
        uncompress_untar ${SPHINXCONTRIB_QTHELP_FILE}
        if test $? -ne 0 ; then
            warn "Could not extract ${SPHINXCONTRIB_QTHELP_FILE}"
            return 1
        fi
    fi

    if ! test -d ${SPHINXCONTRIB_SERIALIZINGHTML_BUILD_DIR} ; then
        info "Extracting sphinxcontrib-serializinghtml ..."
        uncompress_untar ${SPHINXCONTRIB_SERIALIZINGHTML_FILE}
        if test $? -ne 0 ; then
            warn "Could not extract ${SPHINXCONTRIB_SERIALIZINGHTML_FILE}"
            return 1
        fi
    fi

    if ! test -d ${SPHINXCONTRIB_HTMLHELP_BUILD_DIR} ; then
        info "Extracting sphinxcontrib-htmlhelp ..."
        uncompress_untar ${SPHINXCONTRIB_HTMLHELP_FILE}
        if test $? -ne 0 ; then
            warn "Could not extract ${SPHINXCONTRIB_HTMLHELP_FILE}"
            return 1
        fi
    fi

    if ! test -d ${SPHINXCONTRIB_JSMATH_BUILD_DIR} ; then
        info "Extracting sphinxcontrib-jsmath ..."
        uncompress_untar ${SPHINXCONTRIB_JSMATH_FILE}
        if test $? -ne 0 ; then
            warn "Could not extract ${SPHINXCONTRIB_JSMATH_FILE}"
            return 1
        fi
    fi

    if ! test -d ${SPHINXCONTRIB_DEVHELP_BUILD_DIR} ; then
        info "Extracting sphinxcontrib-devhelp ..."
        uncompress_untar ${SPHINXCONTRIB_DEVHELP_FILE}
        if test $? -ne 0 ; then
            warn "Could not extract ${SPHINXCONTRIB_DEVHELP_FILE}"
            return 1
        fi
    fi

    if ! test -d ${SPHINXCONTRIB_APPLEHELP_BUILD_DIR} ; then
        info "Extracting sphinxcontrib-applehelp ..."
        uncompress_untar ${SPHINXCONTRIB_APPLEHELP_FILE}
        if test $? -ne 0 ; then
            warn "Could not extract ${SPHINXCONTRIB_APPLEHELP_FILE}"
            return 1
        fi
    fi

    if ! test -d ${SIX_BUILD_DIR} ; then
        info "Extracting six ..."
        uncompress_untar ${SIX_FILE}
        if test $? -ne 0 ; then
            warn "Could not extract ${SIX_FILE}"
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

    if ! test -d ${PYTZ_BUILD_DIR} ; then
        info "Extracting pytz ..."
        uncompress_untar ${PYTZ_FILE}
        if test $? -ne 0 ; then
            warn "Could not extract ${PYTZ_FILE}"
            return 1
        fi
    fi

    if ! test -d ${MARKUPSAFE_BUILD_DIR} ; then
        info "Extracting markupsafe ..."
        uncompress_untar ${MARKUPSAFE_FILE}
        if test $? -ne 0 ; then
            warn "Could not extract ${MARKUPSAFE_FILE}"
            return 1
        fi
    fi

    if ! test -d ${SPHINX_BUILD_DIR} ; then
        info "Extracting sphinx ..."
        uncompress_untar ${SPHINX_FILE}
        if test $? -ne 0 ; then
            warn "Could not extract ${SPHINX_FILE}"
            return 1
        fi
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

    # install
    pushd $SIX_BUILD_DIR > /dev/null
    info "Installing six ..."
    ${PYTHON_COMMAND} ./setup.py install --prefix="${PYHOME}"
    if test $? -ne 0 ; then
        popd > /dev/null
        warn "Could not install six"
        return 1
    fi
    popd > /dev/null

    pushd $PYPARSING_BUILD_DIR > /dev/null
    info "Installing pyparsing ..."
    ${PYTHON_COMMAND} ./setup.py install --prefix="${PYHOME}"
    if test $? -ne 0 ; then
        popd > /dev/null
        warn "Could not install pyparsing"
        return 1
    fi
    popd > /dev/null

    # Packaging depends on six, pyparsing.
    pushd $PACKAGING_BUILD_DIR > /dev/null
    info "Installing packaging ..."
    ${PYTHON_COMMAND} ./setup.py install --prefix="${PYHOME}"
    if test $? -ne 0 ; then
        popd > /dev/null
        warn "Could not install packaging"
        return 1
    fi
    popd > /dev/null

    pushd $SETUPTOOLS_BUILD_DIR > /dev/null
    info "Installing setuptools ..."
    ${PYTHON_COMMAND} ./setup.py install --prefix="${PYHOME}"
    if test $? -ne 0 ; then
        popd > /dev/null
        warn "Could not install setuptools"
        return 1
    fi
    popd > /dev/null

    pushd $IMAGESIZE_BUILD_DIR > /dev/null
    info "Installing imagesize ..."
    ${PYTHON_COMMAND} ./setup.py install --prefix="${PYHOME}"
    if test $? -ne 0 ; then
        popd > /dev/null
        warn "Could not install imagesize"
        return 1
    fi
    popd > /dev/null

    pushd $ALABASTER_BUILD_DIR > /dev/null
    info "Installing alabaster..."
    ${PYTHON_COMMAND} ./setup.py install --prefix="${PYHOME}"
    if test $? -ne 0 ; then
        popd > /dev/null
        warn "Could not install alabaster"
        return 1
    fi
    popd > /dev/null

    pushd $PYTZ_BUILD_DIR > /dev/null
    info "Installing pytz ..."
    ${PYTHON_COMMAND} ./setup.py install --prefix="${PYHOME}"
    if test $? -ne 0 ; then
        popd > /dev/null
        warn "Could not install pytz"
        return 1
    fi
    popd > /dev/null

    pushd $BABEL_BUILD_DIR > /dev/null
    info "Installing babel ..."
    ${PYTHON_COMMAND} ./setup.py install --prefix="${PYHOME}"
    if test $? -ne 0 ; then
        popd > /dev/null
        warn "Could not install babel"
        return 1
    fi
    popd > /dev/null

    pushd $SNOWBALLSTEMMER_BUILD_DIR > /dev/null
    info "Installing snowballstemmer ..."
    ${PYTHON_COMMAND} ./setup.py install --prefix="${PYHOME}"
    if test $? -ne 0 ; then
        popd > /dev/null
        warn "Could not install snowballstemmer"
        return 1
    fi
    popd > /dev/null

    pushd $DOCUTILS_BUILD_DIR > /dev/null
    info "Installing docutils ..."
    ${PYTHON_COMMAND} ./setup.py install --prefix="${PYHOME}"
    if test $? -ne 0 ; then
        popd > /dev/null
        warn "Could not install docutils"
        return 1
    fi
    popd > /dev/null

    pushd $PYGMENTS_BUILD_DIR > /dev/null
    info "Installing pygments ..."
    ${PYTHON_COMMAND} ./setup.py install --prefix="${PYHOME}"
    if test $? -ne 0 ; then
        popd > /dev/null
        warn "Could not install pygments"
        return 1
    fi
    popd > /dev/null

    pushd $MARKUPSAFE_BUILD_DIR > /dev/null
    info "Installing markupsafe ..."
    ${PYTHON_COMMAND} ./setup.py install --prefix="${PYHOME}"
    if test $? -ne 0 ; then
        popd > /dev/null
        warn "Could not install markupsafe"
        return 1
    fi
    popd > /dev/null

    pushd $JINJA2_BUILD_DIR > /dev/null
    info "Installing jinja2 ..."
    ${PYTHON_COMMAND} ./setup.py install --prefix="${PYHOME}"
    if test $? -ne 0 ; then
        popd > /dev/null
        warn "Could not install jinja2"
        return 1
    fi
    popd > /dev/null

    pushd $SPHINXCONTRIB_QTHELP_BUILD_DIR > /dev/null
    info "Installing sphinxcontrib-qthelp ..."
    ${PYTHON_COMMAND} ./setup.py install --prefix="${PYHOME}"
    if test $? -ne 0 ; then
        popd > /dev/null
        warn "Could not install sphinxcontrib-qthelp"
        return 1
    fi
    popd > /dev/null

    pushd $SPHINXCONTRIB_SERIALIZINGHTML_BUILD_DIR > /dev/null
    info "Installing sphinxcontrib-serializinghtml ..."
    ${PYTHON_COMMAND} ./setup.py install --prefix="${PYHOME}"
    if test $? -ne 0 ; then
        popd > /dev/null
        warn "Could not install sphinxcontrib-serializinghtml"
        return 1
    fi
    popd > /dev/null

    pushd $SPHINXCONTRIB_HTMLHELP_BUILD_DIR > /dev/null
    info "Installing sphinxcontrib-htmlhelp ..."
    ${PYTHON_COMMAND} ./setup.py install --prefix="${PYHOME}"
    if test $? -ne 0 ; then
        popd > /dev/null
        warn "Could not install sphinxcontrib-htmlhelp"
        return 1
    fi
    popd > /dev/null

    pushd $SPHINXCONTRIB_JSMATH_BUILD_DIR > /dev/null
    info "Installing sphinxcontrib-jsmath ..."
    ${PYTHON_COMMAND} ./setup.py install --prefix="${PYHOME}"
    if test $? -ne 0 ; then
        popd > /dev/null
        warn "Could not install sphinxcontrib-jsmath"
        return 1
    fi
    popd > /dev/null

    pushd $SPHINXCONTRIB_DEVHELP_BUILD_DIR > /dev/null
    info "Installing sphinxcontrib-devhelp ..."
    ${PYTHON_COMMAND} ./setup.py install --prefix="${PYHOME}"
    if test $? -ne 0 ; then
        popd > /dev/null
        warn "Could not install sphinxcontrib-devhelp"
        return 1
    fi
    popd > /dev/null

    pushd $SPHINXCONTRIB_APPLEHELP_BUILD_DIR > /dev/null
    info "Installing sphinxcontrib-applehelp ..."
    ${PYTHON_COMMAND} ./setup.py install --prefix="${PYHOME}"
    if test $? -ne 0 ; then
        popd > /dev/null
        warn "Could not install sphinxcontrib-applehelp"
        return 1
    fi
    popd > /dev/null

    pushd $SPHINX_BUILD_DIR > /dev/null
    info "Installing sphinx ..."
    ${PYTHON_COMMAND} ./setup.py install --prefix="${PYHOME}"
    if test $? -ne 0 ; then
        popd > /dev/null
        warn "Could not install sphinx"
        return 1
    fi
    popd > /dev/null

    # fix the perms
    if [[ "$DO_GROUP" == "yes" ]] ; then
        chmod -R ug+w,a+rX "$VISITDIR/python"
        chgrp -R ${GROUP} "$VISITDIR/python"
    fi

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
    # download
    if ! test -f ${SPHINX_RTD_FILE} ; then
        download_file ${SPHINX_RTD_FILE} "${SPHINX_RTD_URL}"
        if [[ $? != 0 ]] ; then
            warn "Could not download ${SPHINX_RTD_FILE}"
            return 1
        fi
    fi

    # extract
    if ! test -d ${SPHINX_RTD_BUILD_DIR} ; then
        info "Extracting sphinx_rtd ..."
        uncompress_untar ${SPHINX_RTD_FILE}
        if test $? -ne 0 ; then
            warn "Could not extract ${SPHINX_RTD_FILE}"
            return 1
        fi
    fi

    # install
    pushd $SPHINX_RTD_BUILD_DIR > /dev/null
    info "Installing sphinx_rtd ..."
    ${PYTHON_COMMAND} ./setup.py install --prefix="${PYHOME}"
    if test $? -ne 0 ; then
        popd > /dev/null
        warn "Could not install sphinx_rtd"
        return 1
    fi
    popd > /dev/null

    # fix the perms
    if [[ "$DO_GROUP" == "yes" ]] ; then
        chmod -R ug+w,a+rX "$VISITDIR/python"
        chgrp -R ${GROUP} "$VISITDIR/python"
    fi

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

            if [[ "$DO_PYTHON2" == "yes" ]]; then
                export PYTHON_COMMAND="${PYHOME}/bin/python"
            else
                export PYTHON_COMMAND="${PYHOME}/bin/python3"
            fi

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

                if [[ "$DO_PYTHON2" == "yes" ]]; then
                    error "sphinx requires python 3 (but DO_PYTHON2=yes). Bailing out."
                fi

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
            fi
        fi
    fi
}
