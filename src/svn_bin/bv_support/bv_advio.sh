function bv_advio_initialize
{
export DO_ADVIO="no"
export ON_ADVIO="off"
}

function bv_advio_enable
{
DO_ADVIO="yes"
ON_ADVIO="on"
}

function bv_advio_disable
{
DO_ADVIO="no"
ON_ADVIO="off"
}

function bv_advio_depends_on
{
return ""
}

function bv_advio_info
{
export ADVIO_FILE=${ADVIO_FILE:-"AdvIO-1.2.tar.gz"}
export ADVIO_VERSION=${ADVIO_VERSION:-"1.2"}
export ADVIO_COMPATIBILITY_VERSION=${ADVIO_COMPATIBILITY_VERSION:-"1.2"}
export ADVIO_BUILD_DIR=${ADVIO_BUILD_DIR:-AdvIO-1.2}
}

function bv_advio_print
{
  printf "%s%s\n" "ADVIO_FILE=" "${ADVIO_FILE}"
  printf "%s%s\n" "ADVIO_VERSION=" "${ADVIO_VERSION}"
  printf "%s%s\n" "ADVIO_COMPATIBILITY_VERSION=" "${ADVIO_COMPATIBILITY_VERSION}"
  printf "%s%s\n" "ADVIO_BUILD_DIR=" "${ADVIO_BUILD_DIR}"
}

function bv_advio_print_usage
{
printf "%-15s %s [%s]\n" "--advio"   "Build AdvIO" "$DO_ADVIO"
}

function bv_advio_graphical
{
local graphical_out="ADVIO    $ADVIO_VERSION($ADVIO_FILE)     $ON_ADVIO"
echo $graphical_out
}

function bv_advio_host_profile
{
    if [[ "$DO_ADVIO" == "yes" ]] ; then
        echo >> $HOSTCONF
        echo "##" >> $HOSTCONF
        echo "## AdvIO" >> $HOSTCONF
        echo "##" >> $HOSTCONF
        echo \
        "VISIT_OPTION_DEFAULT(VISIT_ADVIO_DIR \${VISITHOME}/AdvIO/$ADVIO_VERSION/\${VISITARCH}/)"\
        >> $HOSTCONF
    fi

}

function bv_advio_ensure
{
    if [[ "$DO_ADVIO" == "yes" ]] ; then
        ensure_built_or_ready "AdvIO" $ADVIO_VERSION $ADVIO_BUILD_DIR $ADVIO_FILE
        if [[ $? != 0 ]] ; then
            ANY_ERRORS="yes"
            DO_ADVIO="no"
            error "Unable to build AdvIO. ${ADVIO_FILE} not found. You can register and download it from: http://adventure.sys.t.u-tokyo.ac.jp/download/IO.html"
        fi
    fi
}
# *************************************************************************** #
#                         Function 8.18, build_advio                          #
# *************************************************************************** #

function build_advio
{
    #
    # Prepare build dir
    #
    prepare_build_dir $ADVIO_BUILD_DIR $ADVIO_FILE
    untarred_ADVIO=$?
    if [[ $untarred_ADVIO == -1 ]] ; then
       warn "Unable to prepare AdvIO Build Directory. Giving up"
       return 1
    fi

    # Configure AdvIO
    info "Configuring AdvIO . . ."
    cd $ADVIO_BUILD_DIR || error "Can't cd to AdvIO build dir."
    # Remove IDL dependencies from the build process
    sed "s%@idldir@%%g" Makefile.in > m2
    mv m2 Makefile.in
    sed "s%FileIO IDL DocIO%FileIO DocIO%g" configure > c2
    mv c2 configure
    chmod 750 ./configure
    info "Invoking command to configure AdvIO"
    ADVIO_DARWIN=""
    if [[ "$OPSYS" == "Darwin" ]]; then
        ADVIO_DARWIN="--host=darwin"
    fi
    env CXX="$CXX_COMPILER" CC="$C_COMPILER" \
       CFLAGS="$CFLAGS $C_OPT_FLAGS" CXXFLAGS="$CXXFLAGS $CXX_OPT_FLAGS" \
       ./configure --prefix="$VISITDIR/AdvIO/$ADVIO_VERSION/$VISITARCH" --disable-gtktest $ADVIO_DARWIN
    if [[ $? != 0 ]] ; then
       warn "AdvIO configure failed.  Giving up"
       return 1
    fi

    #
    # Build AdvIO
    #
    info "Building AdvIO . . . (~1 minute)"

    $MAKE
    if [[ $? != 0 ]] ; then
       warn "AdvIO build failed.  Giving up"
       return 1
    fi

    # Install AdvIO
    info "Installing AdvIO"
    $MAKE install
    if [[ $? != 0 ]] ; then
       warn "AdvIO install failed.  Giving up"
       return 1
    fi

    if [[ "$DO_GROUP" == "yes" ]] ; then
       chmod -R ug+w,a+rX "$VISITDIR/AdvIO"
       chgrp -R ${GROUP} "$VISITDIR/AdvIO"
    fi

    cd "$START_DIR"
    info "Done with AdvIO"
    return 0
}

function bv_advio_build
{
cd "$START_DIR"
if [[ "$DO_ADVIO" == "yes" ]] ; then
    check_if_installed "AdvIO" $ADVIO_VERSION
    if [[ $? == 0 ]] ; then
        info "Skipping AdvIO build.  AdvIO is already installed."
    else
        info "Building AdvIO (~1 minutes)"
        build_advio
        if [[ $? != 0 ]] ; then
            error "Unable to build or install AdvIO.  Bailing out."
        fi
        info "Done building AdvIO"
    fi
fi
}
