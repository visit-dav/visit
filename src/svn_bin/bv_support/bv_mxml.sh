function bv_mxml_initialize
{
export DO_MXML="no"
export ON_MXML="off"
}

function bv_mxml_enable
{
DO_MXML="yes"
ON_MXML="on"
}

function bv_mxml_disable
{
DO_MXML="no"
ON_MXML="off"
}

function bv_mxml_depends_on
{
return ""
}

function bv_mxml_info
{
export MXML_FILE=${MXML_FILE:-"mxml-2.6.tar.gz"}
export MXML_VERSION=${MXML_VERSION:-"2.6"}
export MXML_COMPATIBILITY_VERSION=${MXML_COMPATIBILITY_VERSION:-"2.6"}
export MXML_BUILD_DIR=${MXML_BUILD_DIR:-"mxml-2.6"}
}

function bv_mxml_print
{
  printf "%s%s\n" "MXML_FILE=" "${MXML_FILE}"
  printf "%s%s\n" "MXML_VERSION=" "${MXML_VERSION}"
  printf "%s%s\n" "MXML_COMPATIBILITY_VERSION=" "${MXML_COMPATIBILITY_VERSION}"
  printf "%s%s\n" "MXML_BUILD_DIR=" "${MXML_BUILD_DIR}"
}

function bv_mxml_host_profile
{
#nothing to be done for now..
echo "##" >> $HOSTCONF
}

function bv_mxml_print_usage
{
#mxml does not have an option, it is only dependent on mxml.
printf "%-15s %s [%s]\n" "--mxml" "Build Mxml" "$DO_MXML"
}

function bv_mxml_ensure
{
    if [[ "$DO_MXML" == "yes" ]] ; then
        ensure_built_or_ready "MXML" $MXML_VERSION $MXML_BUILD_DIR $MXML_FILE
            
        if [[ $? != 0 ]] ; then
            ANY_ERRORS="yes"
            DO_MXML="no"
            error "Unable to build MXML.  ${MXML_FILE} not found."
        fi
    fi
}

function bv_mxml_dry_run
{
  if [[ "$DO_MXML" == "yes" ]] ; then
    echo "Dry run option not set for mxml."
  fi
}

# ***************************************************************************
#                         Function 8.21, build_mxml
# Required by ADIOS.
#
# Modifications:
#
# ***************************************************************************

function build_mxml
{
    #
    # Prepare build dir
    #
    prepare_build_dir $MXML_BUILD_DIR $MXML_FILE
    untarred_mxml=$?
    if [[ $untarred_mxml == -1 ]] ; then
       warn "Unable to prepare mxml Build Directory. Giving Up"
       return 1
    fi

    #
    info "Configuring mxml . . ."
    cd $MXML_BUILD_DIR || error "Can't cd to mxml build dir."
    info "Invoking command to configure mxml"
    ./configure ${OPTIONAL} CXX="$CXX_COMPILER" \
       CC="$C_COMPILER" CFLAGS="$CFLAGS $C_OPT_FLAGS" CXXFLAGS="$CXXFLAGS $CXX_OPT_FLAGS" \
       --prefix="$VISITDIR/mxml/$MXML_VERSION/$VISITARCH"
    if [[ $? != 0 ]] ; then
       warn "mxml configure failed.  Giving up"
       return 1
    fi

    #
    # Build mxml
    #
    info "Building mxml . . . (~1 minutes)"

    $MAKE $MAKE_OPT_FLAGS
    if [[ $? != 0 ]] ; then
       warn "mxml build failed.  Giving up"
       return 1
    fi
    info "Installing ADIOS . . ."

    $MAKE install
    if [[ $? != 0 ]] ; then
       warn "mxml build (make install) failed.  Giving up"
       return 1
    fi

    if [[ "$DO_GROUP" == "yes" ]] ; then
       chmod -R ug+w,a+rX "$VISITDIR/mxml"
       chgrp -R ${GROUP} "$VISITDIR/mxml"
    fi
    cd "$START_DIR"
    info "Done with mxml"
    return 0
}

function bv_mxml_build
{
    echo "mxml not build directly"
}

function bv_mxml_graphical
{
local graphical_out="MXML    $MXML_VERSION($MXML_FILE)     $ON_MXML"
#echo "$graphical_out"
echo ""
}

