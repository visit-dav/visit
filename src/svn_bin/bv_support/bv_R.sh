function bv_R_initialize
{
export DO_R="no"
export ON_R="off"
}

function bv_R_enable
{
DO_R="yes"
ON_R="on"
VTK_INSTALL_DIR="vtk-R"
}

function bv_R_disable
{
DO_R="no"
ON_R="off"
}

function bv_R_depends_on
{
return ""
}

function bv_R_info
{
export R_FILE=${R_FILE:-"R-2.13.2.tar.gz"}
export R_VERSION=${R_VERSION:-"2.13.2"}
export R_COMPATIBILITY_VERSION=${R_COMPATIBILITY_VERSION:-"2.13.2"}
export R_BUILD_DIR=${R_BUILD_DIR:-"R-2.13.2"}
}

function bv_R_print
{
  printf "%s%s\n" "R_FILE=" "${R_FILE}"
  printf "%s%s\n" "R_VERSION=" "${R_VERSION}"
  printf "%s%s\n" "R_COMPATIBILITY_VERSION=" "${R_COMPATIBILITY_VERSION}"
  printf "%s%s\n" "R_BUILD_DIR=" "${R_BUILD_DIR}"
}

function bv_R_print_usage
{
printf "%-15s %s [%s]\n" "--R"   "Build R" "$DO_R"
}

function bv_R_graphical
{
local graphical_out="R    $R_VERSION($R_FILE)    $ON_R"
echo $graphical_out
}

function bv_R_host_profile
{
    if [[ "$DO_R" == "yes" ]] ; then
        echo >> $HOSTCONF
        echo "##" >> $HOSTCONF
        echo "## R" >> $HOSTCONF
        echo "##" >> $HOSTCONF
        echo \
        "VISIT_OPTION_DEFAULT(VISIT_R_DIR \${VISITHOME}/R/$R_VERSION/\${VISITARCH}/lib/R)" \
        >> $HOSTCONF
    fi
}

function bv_R_ensure
{
    if [[ "$DO_R" == "yes" ]] ; then
        ensure_built_or_ready "R" $R_VERSION $R_BUILD_DIR $R_FILE
        if [[ $? != 0 ]] ; then
            ANY_ERRORS="yes"
            DO_R="no"
            error "Unable to build R.  ${R_FILE} not found."
        fi
    fi
}

function bv_R_dry_run
{
  if [[ "$DO_R" == "yes" ]] ; then
    echo "Dry run option not set for R."
  fi
}

# ***************************************************************************
#                         Function 8.22, build_R
#
# Modifications:
#
# ***************************************************************************

function build_R
{
    #
    # Prepare build dir
    #
    prepare_build_dir $R_BUILD_DIR $R_FILE
    untarred_R=$?
    if [[ $untarred_R == -1 ]] ; then
       warn "Unable to prepare R Build Directory. Giving Up"
       return 1
    fi

    #
    info "Configuring R . . ."
    cd $R_BUILD_DIR || error "Can't cd to R build dir."
    info "Invoking command to configure R"
    env LIBnn=lib ./configure ${OPTIONAL} CXX="$CXX_COMPILER" \
        CC="$C_COMPILER" CFLAGS="$CFLAGS $C_OPT_FLAGS" CXXFLAGS="$CXXFLAGS $CXX_OPT_FLAGS" \
        --without-readline --enable-R-shlib --without-recommended-packages \
        --prefix="$VISITDIR/R/$R_VERSION/$VISITARCH"
    if [[ $? != 0 ]] ; then
       warn "R configure failed.  Giving up"
       return 1
    fi

    #
    # Build R
    #
    info "Building R . . . (~5 minutes)"

    $MAKE $MAKE_OPT_FLAGS
    if [[ $? != 0 ]] ; then
       warn "R build failed.  Giving up"
       return 1
    fi

    info "Installing R . . ."
    $MAKE $MAKE_OPT_FLAGS install
    if [[ $? != 0 ]] ; then
       warn "R install failed.  Giving up"
       return 1
    fi

    info "Installing R . . ."
    $MAKE $MAKE_OPT_FLAGS install

    #
    # TODO, install name tool for OSX
    #

    if [[ "$DO_GROUP" == "yes" ]] ; then
       chmod -R ug+w,a+rX "$VISITDIR/R"
       chgrp -R ${GROUP} "$VISITDIR/R"
    fi
    cd "$START_DIR"
    return 0
}


function bv_R_build
{
cd "$START_DIR"
if [[ "$DO_R" == "yes" ]] ; then
    check_if_installed "R" $R_VERSION
    if [[ $? == 0 ]] ; then
        info "Skipping R build.  R is already installed."
    else
        info "Building R (~10 minutes)"
        build_R
        if [[ $? != 0 ]] ; then
            error "Unable to build or install R.  Bailing out."
        fi
        info "Done building R"
   fi
fi
}

