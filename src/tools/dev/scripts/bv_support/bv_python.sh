function bv_python_initialize
{
    export DO_PYTHON="yes"
    export FORCE_PYTHON="no"
    export USE_SYSTEM_PYTHON="no"
    export BUILD_MPI4PY="no"
    export VISIT_PYTHON_DIR=${VISIT_PYTHON_DIR:-""}
    add_extra_commandline_args "python" "system-python" 0 "Using system python"
    add_extra_commandline_args "python" "alt-python-dir" 1 "Using alternate python directory"
    add_extra_commandline_args "python" "mpi4py" 0 "Build mpi4py"
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
    PYTHON_LIBRARY=`"$PYTHON_CONFIG_COMMAND" --libs`
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
    TEST=`which python-config`
    [ $? != 0 ] && error "System python-config not found, cannot configure python"

    bv_python_enable
    USE_SYSTEM_PYTHON="yes"
    PYTHON_COMMAND="python"
    PYTHON_CONFIG_COMMAND="python-config"
    PYTHON_FILE=""
    python_set_vars_helper #set vars..
}

function bv_python_mpi4py
{
    echo "configuring for building mpi4py"
    export BUILD_MPI4PY="yes"
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
    # we always need openssl b/c of requests.
    echo "openssl zlib"
}

function bv_python_info
{
    export PYTHON_FILE_SUFFIX="tgz"
    export PYTHON_VERSION=${PYTHON_VERSION:-"2.7.14"}
    export PYTHON_COMPATIBILITY_VERSION=${PYTHON_COMPATIBILITY_VERSION:-"2.7"}
    export PYTHON_FILE="Python-$PYTHON_VERSION.$PYTHON_FILE_SUFFIX"
    export PYTHON_BUILD_DIR="Python-$PYTHON_VERSION"
    export PYTHON_MD5_CHECKSUM="cee2e4b33ad3750da77b2e85f2f8b724"
    export PYTHON_SHA256_CHECKSUM="304c9b202ea6fbd0a4a8e0ad3733715fbd4749f2204a9173a58ec53c32ea73e8"

    export PIL_URL=${PIL_URL:-"http://effbot.org/media/downloads"}
    export PIL_FILE=${PIL_FILE:-"Imaging-1.1.7.tar.gz"}
    export PIL_BUILD_DIR=${PIL_BUILD_DIR:-"Imaging-1.1.7"}
    export PIL_MD5_CHECKSUM="fc14a54e1ce02a0225be8854bfba478e"
    export PIL_SHA256_CHECKSUM="895bc7c2498c8e1f9b99938f1a40dc86b3f149741f105cf7c7bd2e0725405211"

    export PYPARSING_FILE=${PYPARSING_FILE:-"pyparsing-1.5.2.tar.gz"}
    export PYPARSING_BUILD_DIR=${PYPARSING_BUILD_DIR:-"pyparsing-1.5.2"}
    export PYPARSING_MD5_CHECKSUM="13aed3cb21a427f8aeb0fe7ca472ba42"
    export PYPARSING_SHA256_CHECKSUM="1021fd2cfdf9c3b6ac0191b018c15d591501b77d977baded59d8ef76d375f21c"

    export PYREQUESTS_FILE=${PYREQUESTS_FILE:-"requests-2.5.1.tar.gz"}
    export PYREQUESTS_BUILD_DIR=${PYREQUESTS_BUILD_DIR:-"requests-2.5.1"}
    export PYREQUESTS_MD5_CHECKSUM="a89558d5dd35a5cb667e9a6e5d4f06f1"
    export PYREQUESTS_SHA256_CHECKSUM="1e5ea203d49273be90dcae2b98120481b2ecfc9f2ae512ce545baab96f57b58c"

    export SEEDME_URL=${SEEDME_URL:-"https://seedme.org/sites/seedme.org/files/downloads/clients/"}
    export SEEDME_FILE=${SEEDME_FILE:-"seedme-python-client-v1.2.4.zip"}
    export SEEDME_BUILD_DIR=${SEEDME_BUILD_DIR:-"seedme-python-client-v1.2.4"}
    export SEEDME_MD5_CHECKSUM="84960d455073fd2f51c31b7fcbc64d58"
    export SEEDME_SHA256_CHECKSUM="71fb233d3b20e95ecd14db1d9cb5deefe775c6ac8bb0ab7604240df7f0e5c5f3"

    export SETUPTOOLS_URL=${SETUPTOOLS_URL:-"https://pypi.python.org/packages/f7/94/eee867605a99ac113c4108534ad7c292ed48bf1d06dfe7b63daa51e49987/"}
    export SETUPTOOLS_FILE=${SETUPTOOLS_FILE:-"setuptools-28.0.0.tar.gz"}
    export SETUPTOOLS_BUILD_DIR=${SETUPTOOLS_BUILD_DIR:-"setuptools-28.0.0"}
    export SETUPTOOLS_MD5_CHECKSUM="9b23df90e1510c7353a5cf07873dcd22"
    export SETUPTOOLS_SHA256_CHECKSUM="e1a2850bb7ad820e4dd3643a6c597bea97a35de2909e9bf0afa3f337836b5ea3"

    export CYTHON_URL=${CYTHON_URL:-"https://pypi.python.org/packages/c6/fe/97319581905de40f1be7015a0ea1bd336a756f6249914b148a17eefa75dc/"}
    export CYTHON_FILE=${CYTHON_FILE:-"Cython-0.25.2.tar.gz"}
    export CYTHON_BUILD_DIR=${CYTHON_BUILD_DIR:-"Cython-0.25.2"}
    export CYTHON_MD5_CHECKSUM="642c81285e1bb833b14ab3f439964086"
    export CYTHON_SHA256_CHECKSUM="f141d1f9c27a07b5a93f7dc5339472067e2d7140d1c5a9e20112a5665ca60306"

    export NUMPY_URL=${NUMPY_URL:-"https://pypi.python.org/packages/a3/99/74aa456fc740a7e8f733af4e8302d8e61e123367ec660cd89c53a3cd4d70/"}
    export NUMPY_FILE=${NUMPY_FILE:-"numpy-1.14.1.zip"}
    export NUMPY_BUILD_DIR=${NUMPY_BUILD_DIR:-"numpy-1.14.1"}
    export NUMPY_MD5_CHECKSUM="b8324ef90ac9064cd0eac46b8b388674"
    export NUMPY_SHA256_CHECKSUM="fa0944650d5d3fb95869eaacd8eedbd2d83610c85e271bd9d3495ffa9bc4dc9c"

    export MPI4PY_URL=${MPI4PY_URL:-"https://pypi.python.org/pypi/mpi4py"}
    export MPI4PY_FILE=${MPI4PY_FILE:-"mpi4py-2.0.0.tar.gz"}
    export MPI4PY_BUILD_DIR=${MPI4PY_BUILD_DIR:-"mpi4py-2.0.0"}
    export MPI4PY_MD5_CHECKSUM="4f7d8126d7367c239fd67615680990e3"
    export MPI4PY_SHA256_CHECKSUM="6543a05851a7aa1e6d165e673d422ba24e45c41e4221f0993fe1e5924a00cb81"

    export PYTHON3_URL=${PYTHON3_URL:-"https://www.python.org/ftp/python/3.7.5"}
    export PYTHON3_FILE_SUFFIX="tgz"
    export PYTHON3_VERSION=${PYTHON3_VERSION:-"3.7.5"}
    export PYTHON3_COMPATIBILITY_VERSION=${PYTHON3_COMPATIBILITY_VERSION:-"3.7"}
    export PYTHON3_FILE="Python-$PYTHON3_VERSION.$PYTHON3_FILE_SUFFIX"
    export PYTHON3_BUILD_DIR="Python-$PYTHON3_VERSION"
    export PYTHON3_MD5_CHECKSUM="1cd071f78ff6d9c7524c95303a3057aa"

    export PACKAGING_URL=${PACKAGING_URL:-""}
    export PACKAGING_FILE=${PACKAGING_FILE:-"packaging-19.2.tar.gz"}
    export PACKAGING_BUILD_DIR=${PACKAGING_BUILD_DIR:-"packaging-19.2"}
    export PACKAGING_MD5_CHECKSUM="867ce70984dc7b89bbbc3cac2a72b171"
    export PACKAGING_SHA256_CHECKSUM="28b924174df7a2fa32c1953825ff29c61e2f5e082343165438812f00d3a7fc47"

    export IMAGESIZE_URL=${IMAGESIZE_URL:-""}
    export IMAGESIZE_FILE=${IMAGESIZE_FILE:-"imagesize-1.1.0.tar.gz"}
    export IMAGESIZE_BUILD_DIR=${IMAGESIZE_BUILD_DIR:-"imagesize-1.1.0"}
    export IMAGESIZE_MD5_CHECKSUM="2f89749b05e07c79c46330dbc62f1e02"
    export IMAGESIZE_SHA256_CHECKSUM="f3832918bc3c66617f92e35f5d70729187676313caa60c187eb0f28b8fe5e3b5"

    export ALABASTER_URL=${ALABASTER_URL:-""}
    export ALABASTER_FILE=${ALABASTER_FILE:-"alabaster-0.7.12.tar.gz"}
    export ALABASTER_BUILD_DIR=${ALABASTER_BUILD_DIR:-"alabaster-0.7.12"}
    export ALABASTER_MD5_CHECKSUM="3591827fde96d1dd23970fb05410ed04"
    export ALABASTER_SHA256_CHECKSUM="a661d72d58e6ea8a57f7a86e37d86716863ee5e92788398526d58b26a4e4dc02"

    export BABEL_URL=${BABEL_URL:-""}
    export BABEL_FILE=${BABEL_FILE:-"Babel-2.7.0.tar.gz"}
    export BABEL_BUILD_DIR=${BABEL_BUILD_DIR:-"Babel-2.7.0"}
    export BABEL_MD5_CHECKSUM="83c158b7dae9135750a7cf204e6e2eea"
    export BABEL_SHA256_CHECKSUM="e86135ae101e31e2c8ec20a4e0c5220f4eed12487d5cf3f78be7e98d3a57fc28"

    export SNOWBALLSTEMMER_URL=${SNOWBALLSTEMMER_URL:-""}
    export SNOWBALLSTEMMER_FILE=${SNOWBALLSTEMMER_FILE:-"snowballstemmer-2.0.0.tar.gz"}
    export SNOWBALLSTEMMER_BUILD_DIR=${SNOWBALLSTEMMER_BUILD_DIR:-"snowballstemmer-2.0.0"}
    export SNOWBALLSTEMMER_MD5_CHECKSUM=""
    export SNOWBALLSTEMMER_SHA256_CHECKSUM=""

    export DOCUTILS_URL=${DOCUTILS_URL:-""}
    export DOCUTILS_FILE=${DOCUTILS_FILE:-"docutils-0.15.2.tar.gz"}
    export DOCUTILS_BUILD_DIR=${DOCUTILS_BUILD_DIR:-"docutils-0.15.2"}
    export DOCUTILS_MD5_CHECKSUM=""
    export DOCUTILS_SHA256_CHECKSUM=""

    export PYGMENTS_URL=${PYGMENTS_URL:-""}
    export PYGMENTS_FILE=${PYGMENTS_FILE:-"Pygments-2.5.2.tar.gz"}
    export PYGMENTS_BUILD_DIR=${PYGMENTS_BUILD_DIR:-"Pygments-2.5.2"}
    export PYGMENTS_MD5_CHECKSUM=""
    export PYGMENTS_SHA256_CHECKSUM=""

    export JINJA2_URL=${JINJA2_URL:-""}
    export JINJA2_FILE=${JINJA2_FILE:-"Jinja2-2.10.3.tar.gz"}
    export JINJA2_BUILD_DIR=${JINJA2_BUILD_DIR:-"Jinja2-2.10.3"}
    export JINJA2_MD5_CHECKSUM=""
    export JINJA2_SHA256_CHECKSUM=""

    export SPINXCONTRIB-QTHELP_URL=${SPINXCONTRIB-QTHELP_URL:-""}
    export SPINXCONTRIB-QTHELP_FILE=${SPINXCONTRIB-QTHELP_FILE:-"sphinxcontrib-qthelp-1.0.2.tar.gz"}
    export SPINXCONTRIB-QTHELP_BUILD_DIR=${SPINXCONTRIB-QTHELP_BUILD_DIR:-"sphinxcontrib-qthelp-1.0.2"}
    export SPINXCONTRIB-QTHELP_MD5_CHECKSUM=""
    export SPINXCONTRIB-QTHELP_SHA256_CHECKSUM=""

    export SPINXCONTRIB-SERIALIZINGHTML_URL=${SPINXCONTRIB-SERIALIZINGHTML_URL:-""}
    export SPINXCONTRIB-SERIALIZINGHTML_FILE=${SPINXCONTRIB-SERIALIZINGHTML_FILE:-"sphinxcontrib-serializinghtml-1.1.3.tar.gz"}
    export SPINXCONTRIB-SERIALIZINGHTML_BUILD_DIR=${SPINXCONTRIB-SERIALIZINGHTML_BUILD_DIR:-"sphinxcontrib-serializinghtml-1.1.3"}
    export SPINXCONTRIB-SERIALIZINGHTML_MD5_CHECKSUM=""
    export SPINXCONTRIB-SERIALIZINGHTML_SHA256_CHECKSUM=""

    export SPINXCONTRIB-HTMLHELP_URL=${SPINXCONTRIB-HTMLHELP_URL:-""}
    export SPINXCONTRIB-HTMLHELP_FILE=${SPINXCONTRIB-HTMLHELP_FILE:-"sphinxcontrib-htmlhelp-1.0.2.tar.gz"}
    export SPINXCONTRIB-HTMLHELP_BUILD_DIR=${SPINXCONTRIB-HTMLHELP_BUILD_DIR:-"sphinxcontrib-htmlhelp-1.0.2"}
    export SPINXCONTRIB-HTMLHELP_MD5_CHECKSUM=""
    export SPINXCONTRIB-HTMLHELP_SHA256_CHECKSUM=""

    export SPINXCONTRIB-JSMATH_URL=${SPINXCONTRIB-JSMATH_URL:-""}
    export SPINXCONTRIB-JSMATH_FILE=${SPINXCONTRIB-JSMATH_FILE:-"sphinxcontrib-jsmath-1.0.1.tar.gz"}
    export SPINXCONTRIB-JSMATH_BUILD_DIR=${SPINXCONTRIB-JSMATH_BUILD_DIR:-"sphinxcontrib-jsmath-1.0.1"}
    export SPINXCONTRIB-JSMATH_MD5_CHECKSUM=""
    export SPINXCONTRIB-JSMATH_SHA256_CHECKSUM=""

    export SPINXCONTRIB-DEVHELP_URL=${SPINXCONTRIB-DEVHELP_URL:-"sphinxcontrib-devhelp-1.0.1.tar.gz"}
    export SPINXCONTRIB-DEVHELP_FILE=${SPINXCONTRIB-DEVHELP_FILE:-"sphinxcontrib-devhelp-1.0.1"}
    export SPINXCONTRIB-DEVHELP_BUILD_DIR=${SPINXCONTRIB-DEVHELP_BUILD_DIR:-""}
    export SPINXCONTRIB-DEVHELP_MD5_CHECKSUM=""
    export SPINXCONTRIB-DEVHELP_SHA256_CHECKSUM=""

    export SPINXCONTRIB-APPLEHELP_URL=${SPINXCONTRIB-APPLEHELP_URL:-"sphinxcontrib-applehelp-1.0.1.tar.gz"}
    export SPINXCONTRIB-APPLEHELP_FILE=${SPINXCONTRIB-APPLEHELP_FILE:-"sphinxcontrib-applehelp-1.0.1"}
    export SPINXCONTRIB-APPLEHELP_BUILD_DIR=${SPINXCONTRIB-APPLEHELP_BUILD_DIR:-""}
    export SPINXCONTRIB-APPLEHELP_MD5_CHECKSUM=""
    export SPINXCONTRIB-APPLEHELP_SHA256_CHECKSUM=""

    export SIX_URL=${SIX_URL:-""}
    export SIX_FILE=${SIX_FILE:-"six-1.13.0.tar.gz"}
    export SIX_BUILD_DIR=${SIX_BUILD_DIR:-"six-1.13.0"}
    export SIX_MD5_CHECKSUM=""
    export SIX_SHA256_CHECKSUM=""

    export URLLIB3_URL=${URLLIB3_URL:-""}
    export URLLIB3_FILE=${URLLIB3_FILE:-"urllib3-1.25.7.tar.gz"}
    export URLLIB3_BUILD_DIR=${URLLIB3_BUILD_DIR:-"urllib3-1.25.7"}
    export URLLIB3_MD5_CHECKSUM=""
    export URLLIB3_SHA256_CHECKSUM=""

    export IDNA_URL=${IDNA_URL:-""}
    export IDNA_FILE=${IDNA_FILE:-"idna-2.8.tar.gz"}
    export IDNA_BUILD_DIR=${IDNA_BUILD_DIR:-"idna-2.8"}
    export IDNA_MD5_CHECKSUM=""
    export IDNA_SHA256_CHECKSUM=""

    export CHARDET_URL=${CHARDET_URL:-""}
    export CHARDET_FILE=${CHARDET_FILE:-"chardet-3.0.4.tar.gz"}
    export CHARDET_BUILD_DIR=${CHARDET_BUILD_DIR:-"chardet-3.0.4"}
    export CHARDET_MD5_CHECKSUM=""
    export CHARDET_SHA256_CHECKSUM=""

    export CERTIFI_URL=${CERTIFI_URL:-""}
    export CERTIFI_FILE=${CERTIFI_FILE:-"certifi-2019.11.28.tar.gz"}
    export CERTIFI_BUILD_DIR=${CERTIFI_BUILD_DIR:-"certifi-2019.11.28"}
    export CERTIFI_MD5_CHECKSUM=""
    export CERTIFI_SHA256_CHECKSUM=""

    export PYTZ_URL=${PYTZ_URL:-""}
    export PYTZ_FILE=${PYTZ_FILE:-"pytz-2019.3.tar.gz"}
    export PYTZ_BUILD_DIR=${PYTZ_BUILD_DIR:-"pytz-2019.3"}
    export PYTZ_MD5_CHECKSUM=""
    export PYTZ_SHA256_CHECKSUM=""

    export MARKUPSAFE_URL=${MARKUPSAFE_URL:-""}
    export MARKUPSAFE_FILE=${MARKUPSAFE_FILE:-"MarkupSafe-1.1.1.tar.gz"}
    export MARKUPSAFE_BUILD_DIR=${MARKUPSAFE_BUILD_DIR:-"MarkupSafe-1.1.1"}
    export MARKUPSAFE_MD5_CHECKSUM=""
    export MARKUPSAFE_SHA256_CHECKSUM=""

    export SPHINX_URL=${SPHINX_URL:-"https://files.pythonhosted.org/packages/f6/3a/c51fc285c0c5c30bcd9426bf096187840683d9383df716a6b6a4ca0a8bde"}
    export SPHINX_FILE=${SPHINX_FILE:-"Sphinx-2.2.1.tar.gz"}
    export SPHINX_BUILD_DIR=${SPHINX_BUILD_DIR:-"Sphinx-2.2.1"}
    export SPHINX_MD5_CHECKSUM="60ea892a09b463e5ecb6ea26d2470f36"
    export SPHINX_SHA256_CHECKSUM="31088dfb95359384b1005619827eaee3056243798c62724fd3fa4b84ee4d71bd"

    export SPHINX_RTD_URL=${SPHINX_RTD_URL:-"https://files.pythonhosted.org/packages/ed/73/7e550d6e4cf9f78a0e0b60b9d93dba295389c3d271c034bf2ea3463a79f9"}
    export SPHINX_RTD_FILE=${SPHINX_RTD_FILE:-"sphinx_rtd_theme-0.4.3.tar.gz"}
    export SPHINX_RTD_BUILD_DIR=${SPHINX_RTD_BUILD_DIR:-"sphinx_rtd_theme-0.4.3"}
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
            echo "VISIT_OPTION_DEFAULT(VISIT_PYTHON3_DIR \${VISITHOME}/python/$PYTHON3_VERSION/\${VISITARCH})" \
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

    if [[ "$DO_OPENSSL" == "yes" ]]; then
        OPENSSL_INCLUDE="$VISITDIR/openssl/$OPENSSL_VERSION/$VISITARCH/include"
        OPENSSL_LIB="$VISITDIR/openssl/$OPENSSL_VERSION/$VISITARCH/lib"
        PYTHON_LDFLAGS="${PYTHON_LDFLAGS} -L${OPENSSL_LIB}"
        PYTHON_CPPFLAGS="${PYTHON_CPPFLAGS} -I${OPENSSL_INCLUDE}"
    fi

    PY_ZLIB_INCLUDE="$VISITDIR/zlib/$ZLIB_VERSION/$VISITARCH/include"
    PY_ZLIB_LIB="$VISITDIR/zlib/$ZLIB_VERSION/$VISITARCH/lib"
    PYTHON_LDFLAGS="${PYTHON_LDFLAGS} -L${PY_ZLIB_LIB}"
    PYTHON_CPPFLAGS="${PYTHON_CPPFLAGS} -I${PY_ZLIB_INCLUDE}"

    if [[ "$OPSYS" == "AIX" ]]; then
        info "Configuring Python (AIX): ./configure OPT=\"$PYTHON_OPT\" CXX=\"$cxxCompiler\" CC=\"$cCompiler\"" \
             "--prefix=\"$PYTHON_PREFIX_DIR\" --disable-ipv6"
        ./configure OPT="$PYTHON_OPT" CXX="$cxxCompiler" CC="$cCompiler" \
                    --prefix="$PYTHON_PREFIX_DIR" --disable-ipv6
    else
        info "Configuring Python : ./configure OPT=\"$PYTHON_OPT\" CXX=\"$cxxCompiler\" CC=\"$cCompiler\"" \
             "LDFLAGS=\"$PYTHON_LDFLAGS\" CPPFLAGS=\"$PYTHON_CPPFLAGS\""\
             "${PYTHON_SHARED} --prefix=\"$PYTHON_PREFIX_DIR\" --disable-ipv6"
        ./configure OPT="$PYTHON_OPT" CXX="$cxxCompiler" CC="$cCompiler" \
                    LDFLAGS="$PYTHON_LDFLAGS" \
                    CPPFLAGS="$PYTHON_CPPFLAGS" \
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
diff -c Imaging-1.1.7.orig/setup.py  Imaging-1.1.7/setup.py
*** Imaging-1.1.7.orig/setup.py Wed Jan  6 11:39:52 2016
--- Imaging-1.1.7/setup.py      Wed Jan  6 11:41:13 2016
***************
*** 211,216 ****
--- 211,220 ----
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
        if [[ $untarred_python == 1 ]] ; then
            warn "Giving up on Pyhton Pil build because the patch failed."
            return 1
        else
            warn "Patch failed, but continuing.  I believe that this script\n" \
                 "tried to apply a patch to an existing directory that had\n" \
                 "already been patched ... that is, the patch is\n" \
                 "failing harmlessly on a second application."
        fi
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
    if test $? -ne 0 ; then
        popd > /dev/null
        warn "Could not build PIL"
        return 1
    fi
    info "Installing PIL ..."
    ${PYHOME}/bin/python ./setup.py install --prefix="${PYHOME}"
    if test $? -ne 0 ; then
        popd > /dev/null
        warn "Could not install PIL"
        return 1
    fi
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
            warn "Could not extract ${SEEDME_FILE}"
            return 1
        fi
    fi

    PYHOME="${VISITDIR}/python/${PYTHON_VERSION}/${VISITARCH}"
    pushd $SEEDME_BUILD_DIR > /dev/null
    info "Installing seedme python module ..."
    ${PYHOME}/bin/python ./setup.py install --prefix="${PYHOME}"
    if test $? -ne 0 ; then
        popd > /dev/null
        warn "Could not install seedme module"
        return 1
    fi
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
    ${PYHOME}/bin/python ./setup.py install --prefix="${PYHOME}"
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
    PYHOME="${VISITDIR}/python/${PYTHON_VERSION}/${VISITARCH}"
    pushd $SETUPTOOLS_BUILD_DIR > /dev/null
    info "Installing setuptools (~1 min) ..."
    ${PYHOME}/bin/python ./setup.py install --prefix="${PYHOME}"
    if test $? -ne 0 ; then
        popd > /dev/null
        warn "Could not install setuptools"
        return 1
    fi
    popd > /dev/null

    pushd $CYTHON_BUILD_DIR > /dev/null
    info "Installing cython (~ 2 min) ..."
    ${PYHOME}/bin/python ./setup.py install --prefix="${PYHOME}"
    if test $? -ne 0 ; then
        popd > /dev/null
        warn "Could not install cython"
        return 1
    fi
    popd > /dev/null

    pushd $NUMPY_BUILD_DIR > /dev/null
    info "Installing numpy (~ 2 min) ..."
    sed -i 's#\\\\\"%s\\\\\"#%s#' numpy/distutils/system_info.py
    ${PYHOME}/bin/python ./setup.py install --prefix="${PYHOME}"
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
#                             build_python3                                   #
# *************************************************************************** #

function build_python3
{
    # download
    if ! test -f ${PYTHON3_FILE} ; then
        download_file ${PYTHON3_FILE} "${PYTHON3_URL}"
        if [[ $? != 0 ]] ; then
            warn "Could not download ${PYTHON3_FILE}"
            return 1
        fi
    fi

    prepare_build_dir $PYTHON3_BUILD_DIR $PYTHON3_FILE
    untarred_python=$?
    # 0, already exists, 1 untarred src, 2 error

    if [[ $untarred_python == -1 ]] ; then
        warn "Unable to prepare Python3 build directory. Giving Up!"
        return 1
    fi

    cd $PYTHON3_BUILD_DIR || error "Can't cd to Python3 build dir."

    #
    # Configure Python3
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
    PYTHON3_OPT="$cFlags"
    PYTHON3_CFG=""
    PYTHON3_LDFLAGS=""
    PYTHON3_CPPFLAGS=""
    PYTHON3_PREFIX_DIR="$VISITDIR/python/$PYTHON3_VERSION/$VISITARCH"
    PYTHON3_LDFLAGS="${PYTHON3_LDFLAGS} -L${PY_ZLIB_LIB}"
    PYTHON3_CPPFLAGS="${PYTHON3_CPPFLAGS} -I${PY_ZLIB_INCLUDE}"

    # python 3.7 uses the --with-openssl flag, instead of flag injection
    if [[ "$DO_OPENSSL" == "yes" ]]; then
        PYTHON3_CFG="${PYTHON3_CFG} --with-openssl=$VISITDIR/openssl/$OPENSSL_VERSION/$VISITARCH/"
    fi


    if [[ "$OPSYS" == "AIX" ]]; then
        info "Configuring Python3 (AIX): ./configure OPT=\"$PYTHON3_OPT\" CXX=\"$cxxCompiler\" CC=\"$cCompiler\"" \
             "--prefix=\"$PYTHON3_PREFIX_DIR\" --disable-ipv6"
        ./configure OPT="$PYTHON3_OPT" CXX="$cxxCompiler" CC="$cCompiler" \
                    --prefix="$PYTHON3_PREFIX_DIR" --disable-ipv6
    else
        info "Configuring Python3 : ./configure OPT=\"$PYTHON3_OPT\" CXX=\"$cxxCompiler\" CC=\"$cCompiler\"" \
             "LDFLAGS=\"$PYTHON3_LDFLAGS\" CPPFLAGS=\"$PYTHON3_CPPFLAGS\""\
             "${PYTHON3_SHARED} --prefix=\"$PYTHON3_PREFIX_DIR\" --disable-ipv6"
        ./configure OPT="$PYTHON3_OPT" CXX="$cxxCompiler" CC="$cCompiler" \
                    LDFLAGS="$PYTHON3_LDFLAGS" \
                    CPPFLAGS="$PYTHON3_CPPFLAGS" \
                    ${PYTHON3_SHARED} \
                    ${PYTHON3_CFG} \
                    --prefix="$PYTHON3_PREFIX_DIR" --disable-ipv6
    fi

    if [[ $? != 0 ]] ; then
        warn "Python3 configure failed.  Giving up"
        return 1
    fi

    #
    # Build Python.
    #
    info "Building Python3 . . . (~2 minutes)"
    $MAKE $MAKE_OPT_FLAGS
    if [[ $? != 0 ]] ; then
        warn "Python3 build failed.  Giving up"
        return 1
    fi
    info "Installing Python3 . . ."
    $MAKE install
    if [[ $? != 0 ]] ; then
        warn "Python3 build (make install) failed.  Giving up"
        return 1
    fi

    if [[ "$DO_STATIC_BUILD" == "no" && "$OPSYS" == "AIX" ]]; then
        # configure flag --enable-shared doesn't work on llnl aix5 systems
        # we need to create the shared lib manually and place it in the
        # proper loc
        mv $VISITDIR/python/$PYTHON3_VERSION/$VISITARCH/lib/libpython$PYTHON3_COMPATIBILITY_VERSION.$SO_EXT \
           $VISITDIR/python/$PYTHON3_VERSION/$VISITARCH/lib/libpython$PYTHON3_COMPATIBILITY_VERSION.static.a

        $C_COMPILER -qmkshrobj -lm \
                    $VISITDIR/python/$PYTHON3_VERSION/$VISITARCH/lib/libpython$PYTHON3_COMPATIBILITY_VERSION.static.a \
                    -o $VISITDIR/python/$PYTHON3_VERSION/$VISITARCH/lib/libpython$PYTHON3_COMPATIBILITY_VERSION.$SO_EXT

        if [[ $? != 0 ]] ; then
            warn "Python3 dynamic library build failed.  Giving up"
            return 1
        fi

        # we can safely remove this version of the static lib b/c it also exists under python3.7/config/
        rm -f $VISITDIR/python/$PYTHON3_VERSION/$VISITARCH/lib/libpython$PYTHON3_COMPATIBILITY_VERSION.static.a
    fi

    if [[ "$DO_GROUP" == "yes" ]] ; then
        chmod -R ug+w,a+rX "$VISITDIR/python"
        chgrp -R ${GROUP} "$VISITDIR/python"
    fi
    cd "$START_DIR"
    info "Done with Python3"

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

    if ! test -f ${SPINXCONTRIB-QTHELP_FILE} ; then
        download_file ${SPINXCONTRIB-QTHELP_FILE} "${SPINXCONTRIB-QTHELP_URL}"
        if [[ $? != 0 ]] ; then
            warn "Could not download ${SPINXCONTRIB-QTHELP_FILE}"
            return 1
        fi
    fi

    if ! test -f ${SPINXCONTRIB-SERIALIZINGHTML_FILE} ; then
        download_file ${SPINXCONTRIB-SERIALIZINGHTML_FILE} "${SPINXCONTRIB-SERIALIZINGHTML_URL}"
        if [[ $? != 0 ]] ; then
            warn "Could not download ${SPINXCONTRIB-SERIALIZINGHTML_FILE}"
            return 1
        fi
    fi

    if ! test -f ${SPINXCONTRIB-HTMLHELP_FILE} ; then
        download_file ${SPINXCONTRIB-HTMLHELP_FILE} "${SPINXCONTRIB-HTMLHELP_URL}"
        if [[ $? != 0 ]] ; then
            warn "Could not download ${SPINXCONTRIB-HTMLHELP_FILE}"
            return 1
        fi
    fi

    if ! test -f ${SPINXCONTRIB-JSMATH_FILE} ; then
        download_file ${SPINXCONTRIB-JSMATH_FILE} "${SPINXCONTRIB-JSMATH_URL}"
        if [[ $? != 0 ]] ; then
            warn "Could not download ${SPINXCONTRIB-JSMATH_FILE}"
            return 1
        fi
    fi

    if ! test -f ${SPINXCONTRIB-DEVHELP_FILE} ; then
        download_file ${SPINXCONTRIB-DEVHELP_FILE} "${SPINXCONTRIB-DEVHELP_URL}"
        if [[ $? != 0 ]] ; then
            warn "Could not download ${SPINXCONTRIB-DEVHELP_FILE}"
            return 1
        fi
    fi

    if ! test -f ${SPINXCONTRIB-APPLEHELP_FILE} ; then
        download_file ${SPINXCONTRIB-APPLEHELP_FILE} "${SPINXCONTRIB-APPLEHELP_URL}"
        if [[ $? != 0 ]] ; then
            warn "Could not download ${SPINXCONTRIB-APPLEHELP_FILE}"
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

    if ! test -f ${CERTIFI_FILE} ; then
        download_file ${CERTIFI_FILE} "${CERTIFI_URL}"
        if [[ $? != 0 ]] ; then
            warn "Could not download ${CERTIFI_FILE}"
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

    if ! test -d ${SPINXCONTRIB-QTHELP_BUILD_DIR} ; then
        info "Extracting sphinxcontrib-qthelp ..."
        uncompress_untar ${SPINXCONTRIB-QTHELP_FILE}
        if test $? -ne 0 ; then
            warn "Could not extract ${SPINXCONTRIB-QTHELP_FILE}"
            return 1
        fi
    fi

    if ! test -d ${SPINXCONTRIB-SERIALIZINGHTML_BUILD_DIR} ; then
        info "Extracting sphinxcontrib-serializinghtml ..."
        uncompress_untar ${SPINXCONTRIB-SERIALIZINGHTML_FILE}
        if test $? -ne 0 ; then
            warn "Could not extract ${SPINXCONTRIB-SERIALIZINGHTML_FILE}"
            return 1
        fi
    fi

    if ! test -d ${SPINXCONTRIB-HTMLHELP_BUILD_DIR} ; then
        info "Extracting sphinxcontrib-htmlhelp ..."
        uncompress_untar ${SPINXCONTRIB-HTMLHELP_FILE}
        if test $? -ne 0 ; then
            warn "Could not extract ${SPINXCONTRIB-HTMLHELP_FILE}"
            return 1
        fi
    fi

    if ! test -d ${SPINXCONTRIB-JSMATH_BUILD_DIR} ; then
        info "Extracting sphinxcontrib-jsmath ..."
        uncompress_untar ${SPINXCONTRIB-JSMATH_FILE}
        if test $? -ne 0 ; then
            warn "Could not extract ${SPINXCONTRIB-JSMATH_FILE}"
            return 1
        fi
    fi

    if ! test -d ${SPINXCONTRIB-DEVHELP_BUILD_DIR} ; then
        info "Extracting sphinxcontrib-devhelp ..."
        uncompress_untar ${SPINXCONTRIB-DEVHELP_FILE}
        if test $? -ne 0 ; then
            warn "Could not extract ${SPINXCONTRIB-DEVHELP_FILE}"
            return 1
        fi
    fi

    if ! test -d ${SPINXCONTRIB-APPLEHELP_BUILD_DIR} ; then
        info "Extracting sphinxcontrib-applehelp ..."
        uncompress_untar ${SPINXCONTRIB-APPLEHELP_FILE}
        if test $? -ne 0 ; then
            warn "Could not extract ${SPINXCONTRIB-APPLEHELP_FILE}"
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

    if ! test -d ${CERTIFI_BUILD_DIR} ; then
        info "Extracting certifi ..."
        uncompress_untar ${CERTIFI_FILE}
        if test $? -ne 0 ; then
            warn "Could not extract ${CERTIFI_FILE}"
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
                SED_CMD="sed -i \"\" "
    fi
    pushd $SPHINX_BUILD_DIR > /dev/null
    ${SED_CMD} "s/docutils>=0.12/docutils<0.16,>=0.12/" ./Sphinx.egg-info/requires.txt
    ${SED_CMD} "s/docutils>=0.12/docutils<0.16,>=0.12/" ./setup.py
    popd > /dev/null

    PY3HOME="${VISITDIR}/python/${PYTHON3_VERSION}/${VISITARCH}"
    # install
    pushd $SIX_BUILD_DIR > /dev/null
    info "Installing six ..."
    ${PY3HOME}/bin/python3 ./setup.py install --prefix="${PY3HOME}"
    if test $? -ne 0 ; then
        popd > /dev/null
        warn "Could not install six"
        return 1
    fi
    popd > /dev/null

    pushd $PACKAGING_BUILD_DIR > /dev/null
    info "Installing packaging ..."
    ${PY3HOME}/bin/python3 ./setup.py install --prefix="${PY3HOME}"
    if test $? -ne 0 ; then
        popd > /dev/null
        warn "Could not install packaging"
        return 1
    fi
    popd > /dev/null

    pushd $IMAGESIZE_BUILD_DIR > /dev/null
    info "Installing imagesize ..."
    ${PY3HOME}/bin/python3 ./setup.py install --prefix="${PY3HOME}"
    if test $? -ne 0 ; then
        popd > /dev/null
        warn "Could not install imagesize"
        return 1
    fi
    popd > /dev/null

    pushd $ALABASTER_BUILD_DIR > /dev/null
    info "Installing alabaster..."
    ${PY3HOME}/bin/python3 ./setup.py install --prefix="${PY3HOME}"
    if test $? -ne 0 ; then
        popd > /dev/null
        warn "Could not install alabaster"
        return 1
    fi
    popd > /dev/null

    pushd $PYTZ_BUILD_DIR > /dev/null
    info "Installing pytz ..."
    ${PY3HOME}/bin/python3 ./setup.py install --prefix="${PY3HOME}"
    if test $? -ne 0 ; then
        popd > /dev/null
        warn "Could not install pytz"
        return 1
    fi
    popd > /dev/null

    pushd $BABEL_BUILD_DIR > /dev/null
    info "Installing babel ..."
    ${PY3HOME}/bin/python3 ./setup.py install --prefix="${PY3HOME}"
    if test $? -ne 0 ; then
        popd > /dev/null
        warn "Could not install babel"
        return 1
    fi
    popd > /dev/null

    pushd $SNOWBALLSTEMMER_BUILD_DIR > /dev/null
    info "Installing snowballstemmer ..."
    ${PY3HOME}/bin/python3 ./setup.py install --prefix="${PY3HOME}"
    if test $? -ne 0 ; then
        popd > /dev/null
        warn "Could not install snowballstemmer"
        return 1
    fi
    popd > /dev/null

    pushd $DOCUTILS_BUILD_DIR > /dev/null
    info "Installing docutils ..."
    ${PY3HOME}/bin/python3 ./setup.py install --prefix="${PY3HOME}"
    if test $? -ne 0 ; then
        popd > /dev/null
        warn "Could not install docutils"
        return 1
    fi
    popd > /dev/null

    pushd $PYGMENTS_BUILD_DIR > /dev/null
    info "Installing pygments ..."
    ${PY3HOME}/bin/python3 ./setup.py install --prefix="${PY3HOME}"
    if test $? -ne 0 ; then
        popd > /dev/null
        warn "Could not install pygments"
        return 1
    fi
    popd > /dev/null

    pushd $MARKUPSAFE_BUILD_DIR > /dev/null
    info "Installing markupsafe ..."
    ${PY3HOME}/bin/python3 ./setup.py install --prefix="${PY3HOME}"
    if test $? -ne 0 ; then
        popd > /dev/null
        warn "Could not install markupsafe"
        return 1
    fi
    popd > /dev/null

    pushd $JINJA2_BUILD_DIR > /dev/null
    info "Installing jinja2 ..."
    ${PY3HOME}/bin/python3 ./setup.py install --prefix="${PY3HOME}"
    if test $? -ne 0 ; then
        popd > /dev/null
        warn "Could not install jinja2"
        return 1
    fi
    popd > /dev/null

    pushd $URLLIB3_BUILD_DIR > /dev/null
    info "Installing urllib3 ..."
    ${PY3HOME}/bin/python3 ./setup.py install --prefix="${PY3HOME}"
    if test $? -ne 0 ; then
        popd > /dev/null
        warn "Could not install urllib3"
        return 1
    fi
    popd > /dev/null

    pushd $IDNA_BUILD_DIR > /dev/null
    info "Installing idna ..."
    ${PY3HOME}/bin/python3 ./setup.py install --prefix="${PY3HOME}"
    if test $? -ne 0 ; then
        popd > /dev/null
        warn "Could not install idna"
        return 1
    fi
    popd > /dev/null

    pushd $CHARDET_BUILD_DIR > /dev/null
    info "Installing chardet ..."
    ${PY3HOME}/bin/python3 ./setup.py install --prefix="${PY3HOME}"
    if test $? -ne 0 ; then
        popd > /dev/null
        warn "Could not install chardet"
        return 1
    fi
    popd > /dev/null

    pushd $CERTIFI_BUILD_DIR > /dev/null
    info "Installing certifi ..."
    ${PY3HOME}/bin/python3 ./setup.py install --prefix="${PY3HOME}"
    if test $? -ne 0 ; then
        popd > /dev/null
        warn "Could not install certifi"
        return 1
    fi
    popd > /dev/null

    pushd $SPHINXCONTRIB-QTHELP_BUILD_DIR > /dev/null
    info "Installing sphinxcontrib-qthelp ..."
    ${PY3HOME}/bin/python3 ./setup.py install --prefix="${PY3HOME}"
    if test $? -ne 0 ; then
        popd > /dev/null
        warn "Could not install sphinxcontrib-qthelp"
        return 1
    fi
    popd > /dev/null

    pushd $SPHINXCONTRIB-SERIALIZING_BUILD_DIR > /dev/null
    info "Installing sphinxcontrib-serializing ..."
    ${PY3HOME}/bin/python3 ./setup.py install --prefix="${PY3HOME}"
    if test $? -ne 0 ; then
        popd > /dev/null
        warn "Could not install sphinxcontrib-serializing"
        return 1
    fi
    popd > /dev/null

    pushd $SPHINXCONTRIB-HTMLHELP_BUILD_DIR > /dev/null
    info "Installing sphinxcontrib-htmlhelp ..."
    ${PY3HOME}/bin/python3 ./setup.py install --prefix="${PY3HOME}"
    if test $? -ne 0 ; then
        popd > /dev/null
        warn "Could not install sphinxcontrib-htmlhelp"
        return 1
    fi
    popd > /dev/null

    pushd $SPHINXCONTRIB-JSMATH_BUILD_DIR > /dev/null
    info "Installing sphinxcontrib-jsmath ..."
    ${PY3HOME}/bin/python3 ./setup.py install --prefix="${PY3HOME}"
    if test $? -ne 0 ; then
        popd > /dev/null
        warn "Could not install sphinxcontrib-jsmath"
        return 1
    fi
    popd > /dev/null

    pushd $SPHINXCONTRIB-DEVHELP_BUILD_DIR > /dev/null
    info "Installing sphinxcontrib-devhelp ..."
    ${PY3HOME}/bin/python3 ./setup.py install --prefix="${PY3HOME}"
    if test $? -ne 0 ; then
        popd > /dev/null
        warn "Could not install sphinxcontrib-devhelp"
        return 1
    fi
    popd > /dev/null

    pushd $SPHINXCONTRIB-APPLEHELP_BUILD_DIR > /dev/null
    info "Installing sphinxcontrib-applehelp ..."
    ${PY3HOME}/bin/python3 ./setup.py install --prefix="${PY3HOME}"
    if test $? -ne 0 ; then
        popd > /dev/null
        warn "Could not install sphinxcontrib-applehelp"
        return 1
    fi
    popd > /dev/null

    pushd $SPHINX_BUILD_DIR > /dev/null
    info "Installing sphinx ..."
    ${PY3HOME}/bin/python3 ./setup.py install --prefix="${PY3HOME}"
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
        info "Extracting sphinx ..."
        uncompress_untar ${SPHINX_RTD_FILE}
        if test $? -ne 0 ; then
            warn "Could not extract ${SPHINX_RTD_FILE}"
            return 1
        fi
    fi

    PY3HOME="${VISITDIR}/python/${PYTHON3_VERSION}/${VISITARCH}"
    # install
    pushd $SPHINX_RTD_BUILD_DIR > /dev/null
    info "Installing sphinx ..."
    ${PY3HOME}/bin/python3 ./setup.py install --prefix="${PY3HOME}"
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

            # Do not build those packages for a static build!
            if [[ "$DO_STATIC_BUILD" == "yes" ]]; then
                return 0
            fi

            info "Building the Python Imaging Library"
            build_pil
            if [[ $? != 0 ]] ; then
                error "PIL build failed. Bailing out."
            fi
            info "Done building the Python Imaging Library"

            info "Building the numpy module"
            build_numpy
            if [[ $? != 0 ]] ; then
                error "numpy build failed. Bailing out."
            fi
            info "Done building the numpy module."

            if [[ "$BUILD_MPI4PY" == "yes" ]]; then
                info "Building the mpi4py module"
                build_mpi4py
                if [[ $? != 0 ]] ; then
                    error "mpi4py build failed. Bailing out."
                fi
                info "Done building the mpi4py module"
            fi

            info "Building the pyparsing module"
            build_pyparsing
            if [[ $? != 0 ]] ; then
                error "pyparsing build failed. Bailing out."
            fi
            info "Done building the pyparsing module."

            build_requests
            if [[ $? != 0 ]] ; then
                error "requests python module build failed. Bailing out."
            fi
            info "Done building the requests python module."

            build_seedme
            if [[ $? != 0 ]] ; then
                error "seedme python module build failed. Bailing out."
            fi
            info "Done building the seedme python module."

            #
            # Currently, we only need python3 for sphinx.
            #
            build_python3
            if [[ $? != 0 ]] ; then
                error "python3 build failed. Bailing out."
            fi
            info "Done building python3."

            build_sphinx
            if [[ $? != 0 ]] ; then
                error "sphinx python module build failed. Bailing out."
            fi
            info "Done building the sphinx python module."

            build_sphinx_rtd
            if [[ $? != 0 ]] ; then
                error "sphinx rtd python theme build failed. Bailing out."
            fi
            info "Done building the sphinx rtd python theme."

        fi
    fi
}
