function bv_tcmalloc_initialize
{
export DO_TCMALLOC="no"
export ON_TCMALLOC="off"
}

function bv_tcmalloc_enable
{
DO_TCMALLOC="yes"
ON_TCMALLOC="on"
}

function bv_tcmalloc_disable
{
DO_TCMALLOC="no"
ON_TCMALLOC="off"
}

function bv_tcmalloc_depends_on
{
echo ""
}

function bv_tcmalloc_info
{
export TCMALLOC_FILE=${TCMALLOC_FILE:-"google-perftools-0.97.tar.gz"}
export TCMALLOC_VERSION=${TCMALLOC_VERSION:-"0.97"}
export TCMALLOC_COMPATIBILITY_VERSION=${TCMALLOC_COMPATIBILITY_VERSION:-"0.97"}
export TCMALLOC_BUILD_DIR=${TCMALLOC_BUILD_DIR:-"google-perftools-0.97"}
export TCMALLOC_MD5_CHECKSUM="5168bdca5557bc5630a866f132f8f7c1"
export TCMALLOC_SHA256_CHECKSUM=""
}

function bv_tcmalloc_print
{
  printf "%s%s\n" "TCMALLOC_FILE=" "${TCMALLOC_FILE}"
  printf "%s%s\n" "TCMALLOC_VERSION=" "${TCMALLOC_VERSION}"
  printf "%s%s\n" "TCMALLOC_COMPATIBILITY_VERSION=" "${TCMALLOC_COMPATIBILITY_VERSION}"
  printf "%s%s\n" "TCMALLOC_BUILD_DIR=" "${TCMALLOC_BUILD_DIR}"
}

function bv_tcmalloc_print_usage
{
printf "%-15s %s [%s]\n" "--tcmalloc" "Build tcmalloc from Google's perftools" "$DO_TCMALLOC"  
}

function bv_tcmalloc_graphical
{
local graphical_out="TCMALLOC    $TCMALLOC_VERSION($TCMALLOC_FILE)     $ON_TCMALLOC"
echo "$graphical_out"
}

function bv_tcmalloc_host_profile
{
    if [[ "$DO_TCMALLOC" == "yes" ]] ; then
        echo >> $HOSTCONF
        echo "##" >> $HOSTCONF
        echo "## Tcmalloc" >> $HOSTCONF
        echo "##" >> $HOSTCONF
        echo \
        "VISIT_OPTION_DEFAULT(VISIT_TCMALLOC_DIR \${VISITHOME}/google-perftools/$TCMALLOC_VERSION/\${VISITARCH})" \
        >> $HOSTCONF
    fi
}

function bv_tcmalloc_ensure
{
    if [[ "$DO_TCMALLOC" == "yes" ]] ; then
        ensure_built_or_ready "google-perftools" $TCMALLOC_VERSION $TCMALLOC_BUILD_DIR $TCMALLOC_FILE
        if [[ $? != 0 ]] ; then
            warn "Unable to build google perftools.  ${TCMALLOC_FILE} not found."
            ANY_ERRORS="yes"
            DO_TCMALLOC="no"
            if [[ "$DO_SVN" != "yes" ]] ; then
                warn "You have requested to build the google perftools library."
                warn "This is not currently available for download from the VisIt website and" 
                warn "is only available through Subversion access."
            fi
            error
        fi
    fi
}

function bv_tcmalloc_dry_run
{
  if [[ "$DO_TCMALLOC" == "yes" ]] ; then
    echo "Dry run option not set for tcmalloc."
  fi
}
# *************************************************************************** #
#                         Function 8.12, build_tcmalloc                       #
# *************************************************************************** #

function build_tcmalloc
{
    #
    # Prepare build dir
    #
    prepare_build_dir $TCMALLOC_BUILD_DIR $TCMALLOC_FILE
    untarred_tcmalloc=$?
    if [[ $untarred_tcmalloc == -1 ]] ; then
       warn "Unable to prepare google-perftools Build Directory. Giving Up"
       return 1
    fi


    info "Configuring google-perftools . . ."
    cd $TCMALLOC_BUILD_DIR || error "Can't cd to tcmalloc build dir."

    #
    # Build TCMALLOC
    #
    info "Building google-perftools . . . (~1 minutes)"
    if [[ "$DO_STATIC_BUILD" == "no" ]]; then 
        ./configure
    else
        ./configure --enable-static --disable-shared
    fi
    make

    info "Installing google-perftools . . ."
    mkdir $VISITDIR/google-perftools
    mkdir $VISITDIR/google-perftools/${TCMALLOC_VERSION}
    mkdir $VISITDIR/google-perftools/${TCMALLOC_VERSION}/$VISITARCH
    mkdir $VISITDIR/google-perftools/${TCMALLOC_VERSION}/$VISITARCH/lib
    cp .libs/libtcmalloc.* $VISITDIR/google-perftools/${TCMALLOC_VERSION}/$VISITARCH/lib

    if [[ "$DO_GROUP" == "yes" ]] ; then
       chmod -R ug+w,a+rX "$VISITDIR/google-perftools"
       chgrp -R ${GROUP} "$VISITDIR/google-perftools"
    fi

    cd "$START_DIR"
    echo "Done with google-perftools"
    return 0
}

function bv_tcmalloc_is_enabled
{
    if [[ $DO_TCMALLOC == "yes" ]]; then
        return 1    
    fi
    return 0
}

function bv_tcmalloc_is_installed
{
    check_if_installed "google-perftools"
    if [[ $? == 0 ]] ; then
        return 1
    fi
    return 0
}

function bv_tcmalloc_build
{
cd "$START_DIR"
if [[ "$DO_TCMALLOC" == "yes" ]] ; then
    check_if_installed "google-perftools"
    if [[ $? == 0 ]] ; then
        info "Skipping google-perftools build.  google-perftools is already installed."
    else
        info "Building google-perftools (~2 minutes)"
        build_tcmalloc
        if [[ $? != 0 ]] ; then
            error "Unable to build or install google-perftools.  Bailing out."
        fi
        info "Done building google-perftools"
    fi
fi
}

