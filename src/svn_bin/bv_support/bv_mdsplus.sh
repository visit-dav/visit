function bv_mdsplus_initialize
{
export DO_MDSPLUS="no"
export ON_MDSPLUS="off"
}

function bv_mdsplus_enable
{
DO_MDSPLUS="yes"
ON_MDSPLUS="on"
}

function bv_mdsplus_disable
{
DO_MDSPLUS="no"
ON_MDSPLUS="off"
}

function bv_mdsplus_depends_on
{
echo ""
}

function bv_mdsplus_info
{
export MDSPLUS_VERSION=${MDSPLUS_VERSION:-"5.0"}
export MDSPLUS_FILE=${MDSPLUS_FILE:-"mdsplus-${MDSPLUS_VERSION}.tar.gz"}
export MDSPLUS_COMPATIBILITY_VERSION=${MDSPLUS_COMPATIBILITY_VERSION:-"5.0"}
export MDSPLUS_BUILD_DIR=${MDSPLUS_BUILD_DIR:-"mdsplus-${MDSPLUS_VERSION}"}
#export MDSPLUS_BUILD_DIR=${MDSPLUS_BUILD_DIR:-"mdsplus"}
export MDSPLUS_MD5_CHECKSUM=""
export MDSPLUS_SHA256_CHECKSUM=""
}

function bv_mdsplus_print
{
  printf "%s%s\n" "MDSPLUS_FILE=" "${MDSPLUS_FILE}"
  printf "%s%s\n" "MDSPLUS_VERSION=" "${MDSPLUS_VERSION}"
  printf "%s%s\n" "MDSPLUS_COMPATIBILITY_VERSION=" "${MDSPLUS_COMPATIBILITY_VERSION}"
  printf "%s%s\n" "MDSPLUS_BUILD_DIR=" "${MDSPLUS_BUILD_DIR}"
}

function bv_mdsplus_print_usage
{
printf "%-15s %s [%s]\n" "--mdsplus" "Build MDSplus" "${DO_MDSPLUS}"
}

function bv_mdsplus_graphical
{
local graphical_out="MDSPLUS    $MDSPLUS_VERSION($MDSPLUS_FILE)     $ON_MDSPLUS"
echo $graphical_out
}

function bv_mdsplus_host_profile
{
    if [[ "$DO_MDSPLUS" == "yes" ]] ; then
        echo >> $HOSTCONF
        echo "##" >> $HOSTCONF
        echo "## Mdsplus" >> $HOSTCONF
        echo "##" >> $HOSTCONF
        echo \
        "VISIT_OPTION_DEFAULT(VISIT_MDSPLUS_DIR \${VISITHOME}/mdsplus/$MDSPLUS_VERSION/\${VISITARCH})" \
        >> $HOSTCONF
        echo \
        "VISIT_OPTION_DEFAULT(VISIT_MDSPLUS_LIBDEP HDF5_LIBRARY_DIR hdf5 \${VISIT_HDF5_LIBDEP} TYPE STRING)" \
        >> $HOSTCONF
    fi
}

function bv_mdsplus_ensure
{
    if [[ "$DO_MDSPLUS" == "yes" ]] ; then
        ensure_built_or_ready "mdsplus" $MDSPLUS_VERSION $MDSPLUS_BUILD_DIR $MDSPLUS_FILE
        if [[ $? != 0 ]] ; then
            ANY_ERRORS="yes"
            DO_MDSPLUS="no"
            error "Unable to build MDSplus.  ${MDSPLUS_FILE} not found."
        fi
    fi
}

function bv_mdsplus_dry_run
{
  if [[ "$DO_MDSPLUS" == "yes" ]] ; then
    echo "Dry run option not set for mdsplus."
  fi
}

# ***************************************************************************
#                         Function 8.20, build_mdsplus
#
# Modifications:
#
#  Mark C. Miller, Tue Oct 28 11:10:36 PDT 2008
#  Added -DH5_USE_16_API to CFLAGS for configuring MDSplus. This should be
#  harmless when building MDSplus against versions of HDF5 before 1.8 and
#  necessary when building against versions of HDF5 1.8 or later. It tells
#  HDF5 which version of the HDF5 API MDSplus was implemented with.
# ***************************************************************************

function build_mdsplus
{
    #
    # Prepare build dir
    #
    prepare_build_dir $MDSPLUS_BUILD_DIR $MDSPLUS_FILE
    untarred_mdsplus=$?
    if [[ $untarred_mdsplus == -1 ]] ; then
       warn "Unable to prepare MDSplus Build Directory. Giving Up"
       return 1
    fi

    #
    info "Configuring MDSplus . . ."
    cd $MDSPLUS_BUILD_DIR || error "Can't cd to mdsplus build dir."
    info "Invoking command to configure MDSplus"
    ./configure ${OPTIONAL} --disable-java CXX="$CXX_COMPILER" \
       CC="$C_COMPILER" CFLAGS="$CFLAGS $C_OPT_FLAGS" CXXFLAGS="$CXXFLAGS $CXX_OPT_FLAGS" \
       --prefix="$VISITDIR/mdsplus/$MDSPLUS_VERSION/$VISITARCH"
    if [[ $? != 0 ]] ; then
       warn "MDSplus configure failed.  Giving up"
       return 1
    fi

    #
    # Build MDSplus
    #
    info "Building MDSplus . . . (~1 minutes)"

    $MAKE $MAKE_OPT_FLAGS
    if [[ $? != 0 ]] ; then
       warn "MDSplus build failed.  Giving up"
       return 1
    fi
    info "Installing MDSplus . . ."

    $MAKE install
    if [[ $? != 0 ]] ; then
       warn "MDSplus build (make install) failed.  Giving up"
       return 1
    fi

    if [[ "$DO_GROUP" == "yes" ]] ; then
       chmod -R ug+w,a+rX "$VISITDIR/mdsplus"
       chgrp -R ${GROUP} "$VISITDIR/mdsplus"
    fi
    cd "$START_DIR"
    info "Done with MDSplus"
    return 0
}

function bv_mdsplus_is_enabled
{
    if [[ $DO_MDSPLUS == "yes" ]]; then
        return 1    
    fi
    return 0
}

function bv_mdsplus_is_installed
{
    check_if_installed "mdsplus" $MDSPLUS_VERSION
    if [[ $? == 0 ]] ; then
        return 1
    fi
    return 0
}

function bv_mdsplus_build
{
cd "$START_DIR"
if [[ "$DO_MDSPLUS" == "yes" ]] ; then
    check_if_installed "mdsplus" $MDSPLUS_VERSION
    if [[ $? == 0 ]] ; then
        info "Skipping MDSplus build.  MDSplus is already installed."
    else
        info "Building MDSplus (~1 minutes)"
        build_mdsplus
        if [[ $? != 0 ]] ; then
            error "Unable to build or install MDSplus.  Bailing out."
        fi
        info "Done building MDSplus"
    fi
fi
}
