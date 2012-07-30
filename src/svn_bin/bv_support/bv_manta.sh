function bv_manta_initialize
{
export DO_MANTA="no"
export ON_MANTA="off"
}

function bv_manta_enable
{
DO_MANTA="yes"
ON_MANTA="on"
}

function bv_manta_disable
{
DO_MANTA="no"
ON_MANTA="off"
}

function bv_manta_depends_on
{
echo ""
}

function bv_manta_info
{
export MANTA_VERSION=${MANTA_VERSION:-"2540"}
export MANTA_FILE=${MANTA_FILE:-"manta-${MANTA_VERSION}.tar.gz"}
export MANTA_COMPATIBILITY_VERSION=${MANTA_COMPATIBILITY_VERSION:-"2540"}
export MANTA_BUILD_DIR=${MANTA_BUILD_DIR:-"manta-${MANTA_VERSION}"}
export MANTA_MD5_CHECKSUM=""
export MANTA_SHA256_CHECKSUM=""
}

function bv_manta_print
{
  printf "%s%s\n" "MANTA_FILE=" "${MANTA_FILE}"
  printf "%s%s\n" "MANTA_VERSION=" "${MANTA_VERSION}"
  printf "%s%s\n" "MANTA_COMPATIBILITY_VERSION=" "${MANTA_COMPATIBILITY_VERSION}"
  printf "%s%s\n" "MANTA_BUILD_DIR=" "${MANTA_BUILD_DIR}"
}

function bv_manta_print_usage
{
printf "%-15s %s [%s]\n" "--manta" "Build Manta" "${DO_MANTA}"
}

function bv_manta_graphical
{
local graphical_out="MANTA    $MANTA_VERSION($MANTA_FILE)     $ON_MANTA"
echo $graphical_out
}

function bv_manta_host_profile
{
    if [[ "$DO_MANTA" == "yes" ]] ; then
        echo >> $HOSTCONF
        echo "##" >> $HOSTCONF
        echo "## MANTA" >> $HOSTCONF
        echo "##" >> $HOSTCONF
        echo \
        "VISIT_OPTION_DEFAULT(VISIT_MANTA_DIR \${VISITHOME}/manta/$MANTA_VERSION/\${VISITARCH})" \
        >> $HOSTCONF
        echo \
        "VISIT_OPTION_DEFAULT(VISIT_MANTA ON TYPE BOOL)" \
        >> $HOSTCONF
        echo \
        "VISIT_OPTION_DEFAULT(MANTA_SOURCE_DIR \${VISIT_MANTA_DIR}/include)" \
        >> $HOSTCONF
        echo \
        "VISIT_OPTION_DEFAULT(MANTA_BUILD_PREFIX \${VISIT_MANTA_DIR})" \
        >> $HOSTCONF
    fi
}

function bv_manta_ensure
{
    if [[ "$DO_MANTA" == "yes" ]] ; then
        ensure_built_or_ready "manta" $MANTA_VERSION $MANTA_BUILD_DIR $MANTA_FILE
        if [[ $? != 0 ]] ; then
            ANY_ERRORS="yes"
            DO_MANTA="no"
            error "Unable to build Manta.  ${MANTA_FILE} not found."
        fi
    fi
}

function bv_manta_dry_run
{
  if [[ "$DO_MANTA" == "yes" ]] ; then
    echo "Dry run option not set for manta."
  fi
}

# ***************************************************************************
#                         Function build_manta
#
# ***************************************************************************

function build_manta
{
    #
    # Prepare build dir
    #
    prepare_build_dir $MANTA_BUILD_DIR $MANTA_FILE
    untarred_manta=$?
    if [[ $untarred_manta == -1 ]] ; then
       warn "Unable to prepare Manta Build Directory. Giving Up"
       return 1
    fi

    #
    info "MANTA_BUILD_DIR:"
    info $MANTA_BUILD_DIR
    cd ${MANTA_BUILD_DIR} || error "Can't cd to Manta build dir."
    #apply_manta_patch
    #if [[ $? != 0 ]] ; then
    #    warn "Patch failed, but continuing."
    #fi

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
    MANTA_OPT="$cFlags"
    MANTA_LDFLAGS=""
    MANTA_PREFIX_DIR="$VISITDIR/manta/$MANTA_VERSION/$VISITARCH"
    if [[ "$DO_STATIC_BUILD" == "no" ]]; then
        MANTA_SHARED="--enable-shared"
        if [[ "$C_COMPILER" == "gcc" ]]; then
            #
            # python's --enable-shared configure flag doesn't link
            # the exes it builds correclty when installed to a non standard
            # prefix. To resolve this we need to add a rpath linker flags.
            #
            #mkdir -p ${MANTA_PREFIX_DIR}/build
            #cmake .
            cmake -DCMAKE_INSTALL_PREFIX=$MANTA_PREFIX_DIR .
            if [[ $? != 0 ]] ; then
                 warn "Manta configure failed.  Giving up"
            return 1
            fi

            if [[ "$OPSYS" != "Darwin" || ${VER%%.*} -ge 9 ]]; then
                MANTA_LDFLAGS="-Wl,-rpath,${MANTA_PREFIX_DIR}/lib/ -pthread"
            fi
        fi
    fi
    #
    #  Manta CMake
    #
    if [[ $? != 0 ]] ; then
       warn "Manta cmake failed.  Giving up"
       return 1
    fi

    #
    # Build Manta.
    #
    info "Building Manta . . . (~2 minutes)"
    $MAKE $MAKE_OPT_FLAGS
    if [[ $? != 0 ]] ; then
       warn "Manta build failed.  Giving up"
       return 1
    fi
    info "Installing Manta . . ."
    $MAKE install
    if [[ $? != 0 ]] ; then
       warn "Manta build (make install) failed.  Giving up"
       return 1
    fi

    cd "$START_DIR"
    info "Done with Manta"

    return 0
}

function bv_manta_is_enabled
{
    if [[ $DO_MANTA == "yes" ]]; then
        return 1    
    fi
    return 0
}

function bv_manta_is_installed
{
    check_if_installed "manta" $MANTA_VERSION
    if [[ $? == 0 ]] ; then
        return 1
    fi
    return 0
}

function bv_manta_build
{
cd "$START_DIR"
if [[ "$DO_MANTA" == "yes" ]] ; then
    check_if_installed "manta" $MANTA_VERSION
    if [[ $? == 0 ]] ; then
        info "Skipping Manta build.  Manta is already installed."
    else
        info "Building Manta (~1 minutes)"
        build_manta
        if [[ $? != 0 ]] ; then
            error "Unable to build or install Manta.  Bailing out."
        fi
        info "Done building Manta"
    fi
fi
}
