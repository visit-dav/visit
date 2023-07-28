#
# Example build_visit command line to build stripack
# env FCFLAGS="-fdefault-real-8 -fdefault-double-8 -shared -fPIC" STRIPACK_INSTALL_DIR=<visit-install-home-dir>/lib ../build_visit --fortran --no-visit --no-thirdparty --thirdparty-path /dev/null --no-zlib --stripack
#
function bv_stripack_initialize
{
    export DO_STRIPACK="no"
}

function bv_stripack_enable
{
    DO_STRIPACK="yes"
}

function bv_stripack_disable
{
    DO_STRIPACK="no"
}

function bv_stripack_depends_on
{
    echo ""
}

function bv_stripack_info
{
    export STRIPACK_FILE=${STRIPACK_FILE:-"stripack-ACM.RJRenka.Sep97.tar.gz"}
    export STRIPACK_VERSION=${STRIPACK_VERSION:-"ACM.RJRenka.Sep97"}
    export STRIPACK_COMPATIBILITY_VERSION=${STRIPACK_COMPATIBILITY_VERSION:-"ACM.RJRenka.Sep97"}
    export STRIPACK_BUILD_DIR=${STRIPACK_BUILD_DIR:-"stripack-ACM.RJRenka.Sep97"}
    export STRIPACK_MD5_CHECKSUM="364761b3c42d65b274f703b4be576d20"
    export STRIPACK_SHA256_CHECKSUM="4ba03dab1592850339327f8ba50b1dfecac4abd09f7757f50a022914d1353d73"
}

function bv_stripack_print
{
    printf "%s%s\n" "STRIPACK_FILE=" "${STRIPACK_FILE}"
    printf "%s%s\n" "STRIPACK_VERSION=" "${STRIPACK_VERSION}"
    printf "%s%s\n" "STRIPACK_COMPATIBILITY_VERSION=" "${STRIPACK_COMPATIBILITY_VERSION}"
    printf "%s%s\n" "STRIPACK_BUILD_DIR=" "${STRIPACK_BUILD_DIR}"
}

function bv_stripack_print_usage
{
    printf "%-20s %s [%s]\n" "--stripack" "Build ACM-Licensed stripack library for ffp plugin" "$DO_STRIPACK"  
}

function bv_stripack_host_profile
{
    echo ""
}

function bv_stripack_ensure
{
    if [[ "$DO_STRIPACK" == "yes" ]] ; then
        ensure_built_or_ready "stripack" $STRIPACK_VERSION $STRIPACK_BUILD_DIR $STRIPACK_FILE
        if [[ $? != 0 ]] ; then
            warn "Unable to build stripack.  ${STRIPACK_FILE} not found."
            ANY_ERRORS="yes"
            DO_STRIPACK="no"
        fi
    fi
}

function stripack_license_prompt
{
    STRIPACK_LIC_MSG="This script will build the stripack library. stripack is ACM\
            Licensed software. Please read the terms of the ACM License,\
            https://www.acm.org/publications/policies/software-copyright-notice\
            and respond either \"yes\" to accept the terms or \"no\" to decline\
            the terms."
    info $STRIPACK_LIC_MSG
    read RESPONSE
    if [[ "$RESPONSE" != "yes" ]] ; then
        error "Stripack requires compliance with ACM License terms."
    fi

    return 0
}

# *************************************************************************** #
#                         Function 8.12, build_stripack                       #
# *************************************************************************** #

function build_stripack
{
    stripack_license_prompt

    if [[ "$FC_COMPILER" = "no" ]] ; then
        error "FC_COMPILER env. variable must be set to a fortran compiler for stripack."
    fi
    if [[ "$DO_STATIC_BUILD" != "no" ]]; then 
        error "stripack works only in non-static builds."
    fi
    if [[ "$STRIPACK_INSTALL_DIR" = "" ]] ; then
        error "STRIPACK_INSTALL_DIR env. variable must be set"
    fi
    if [ ! -d $STRIPACK_INSTALL_DIR -o ! -w $STRIPACK_INSTALL_DIR -o ! -x $STRIPACK_INSTALL_DIR ]; then
        error "$STRIPACK_INSTALL_DIR either does not exist, is not writeable or is not executable."
    fi 

    #
    # Prepare build dir
    #
    prepare_build_dir $STRIPACK_BUILD_DIR $STRIPACK_FILE
    untarred_stripack=$?
    if [[ $untarred_stripack == -1 ]] ; then
        warn "Unable to prepare stripack Build Directory. Giving Up"
        return 1
    fi

    info "Building stripack. . ."
    cd $STRIPACK_BUILD_DIR || error "Can't cd to stripack build dir."
    $FC_COMPILER $FCFLAGS -o libstripack.so stripack.f
    if [[ $? != 0 ]] ; then
        error "Unable to compile stripack"
    fi
   
    info "Installing stripack. . ."
    cp libstripack.so $STRIPACK_INSTALL_DIR
    if [[ "$DO_GROUP" == "yes" ]] ; then
        chmod -R ug+w,a+rX "$STRIPACK_INSTALL_DIR/libstripack.so"
        chgrp -R ${GROUP} "$STRIPACK_INSTALL_DIR/libstripack.so"
    fi

    cd "$START_DIR"
    echo "Done with stripack"
    return 0
}

function bv_stripack_is_enabled
{
    if [[ $DO_STRIPACK == "yes" ]]; then
        return 1    
    fi
    return 0
}

function bv_stripack_is_installed
{
    check_if_installed "stripack"
    if [[ $? == 0 ]] ; then
        return 1
    fi
    return 0
}

function bv_stripack_build
{
    cd "$START_DIR"
    if [[ "$DO_STRIPACK" == "yes" ]] ; then
        check_if_installed "stripack"
        if [[ $? == 0 ]] ; then
            info "Skipping stripack build.  stripack is already installed."
        else
            info "Building stripack (~1 minutes)"
            build_stripack
            if [[ $? != 0 ]] ; then
                error "Unable to build or install stripack.  Bailing out."
            fi
            info "Done building stripack"
        fi
    fi
}
