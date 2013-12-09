function bv_uintah_initialize
{
export DO_UINTAH="no"
export ON_UINTAH="off"
export USE_SYSTEM_UINTAH="no"
add_extra_commandline_args "uintah" "alt-uintah-dir" 1 "Use alternative directory for uintah"
}

function bv_uintah_enable
{
DO_UINTAH="yes"
ON_UINTAH="on"
}

function bv_uintah_disable
{
DO_UINTAH="no"
ON_UINTAH="off"
}

function bv_uintah_alt_uintah_dir
{
    bv_uintah_enable
    USE_SYSTEM_UINTAH="yes"
    UINTAH_INSTALL_DIR="$1"
}

function bv_uintah_depends_on
{
    if [[ "$USE_SYSTEM_UINTAH" == "yes" ]]; then
        echo ""
    else
        local depends_on="szip"

        if [[ "$DO_ZLIB" == "yes" ]] ; then
           depends_on="$depends_on zlib"    
        fi

        echo $depends_on
    fi
}

function bv_uintah_initialize_vars
{
    if [[ "$USE_SYSTEM_UINTAH" == "no" ]]; then
        UINTAH_INSTALL_DIR="${VISITDIR}/uintah/$UINTAH_VERSION/${VISITARCH}"
    fi
}

function bv_uintah_info
{
export UINTAH_VERSION=${UINTAH_VERSION:-"1.5.0"}
export UINTAH_FILE=${UINTAH_FILE:-"uintah-${UINTAH_VERSION}.tar.gz"}
export UINTAH_COMPATIBILITY_VERSION=${UINTAH_COMPATIBILITY_VERSION:-"1.8"}
export UINTAH_BUILD_DIR=${UINTAH_BUILD_DIR:-"uintah-${UINTAH_VERSION}"}
#export UINTAH_URL=${UINTAH_URL:-"http://www.sci.utah.edu/ftp/UINTAH/prev-releases/uintah-${UINTAH_VERSION}/src"}
export UINTAH_MD5_CHECKSUM=""
export UINTAH_SHA256_CHECKSUM=""
}

function bv_uintah_print
{
  printf "%s%s\n" "UINTAH_FILE=" "${UINTAH_FILE}"
  printf "%s%s\n" "UINTAH_VERSION=" "${UINTAH_VERSION}"
  printf "%s%s\n" "UINTAH_COMPATIBILITY_VERSION=" "${UINTAH_COMPATIBILITY_VERSION}"
  printf "%s%s\n" "UINTAH_BUILD_DIR=" "${UINTAH_BUILD_DIR}"
}

function bv_uintah_print_usage
{
printf "%-15s %s [%s]\n" "--uintah" "Build UINTAH" "${DO_UINTAH}"
}

function bv_uintah_graphical
{
local graphical_out="UINTAH     $UINTAH_VERSION($UINTAH_FILE)      $ON_UINTAH"
echo $graphical_out
}

function bv_uintah_host_profile
{
    if [[ "$DO_UINTAH" == "yes" ]] ; then
        echo >> $HOSTCONF
        echo "##" >> $HOSTCONF
        echo "## UINTAH" >> $HOSTCONF
        echo "##" >> $HOSTCONF

        if [[ "$USE_SYSTEM_UINTAH" == "yes" ]]; then
            echo \
            "VISIT_OPTION_DEFAULT(VISIT_UINTAH_DIR $UINTAH_INSTALL_DIR)" \
            >> $HOSTCONF 
        else
            echo \
            "VISIT_OPTION_DEFAULT(VISIT_UINTAH_DIR \${VISITHOME}/uintah/$UINTAH_VERSION/\${VISITARCH})" \
            >> $HOSTCONF 

            if [[ "$DO_ZLIB" == "yes" ]] ; then
               ZLIB_LIBDEP="\${VISITHOME}/zlib/$ZLIB_VERSION/\${VISITARCH}/lib z"
            else
               ZLIB_LIBDEP="/usr/lib z"
               #moving global patch to have limited effect
               if [[ -d /usr/lib/x86_64-linux-gnu ]]; then
                ZLIB_LIBDEP="/usr/lib/x86_64-linux-gnu z"
               fi
            fi

            if [[ "$DO_SZIP" == "yes" ]] ; then
                echo \
                "VISIT_OPTION_DEFAULT(VISIT_UINTAH_LIBDEP \${VISITHOME}/szip/$SZIP_VERSION/\${VISITARCH}/lib sz $ZLIB_LIBDEP TYPE STRING)" \
                >> $HOSTCONF
            else
                echo \
                "VISIT_OPTION_DEFAULT(VISIT_UINTAH_LIBDEP $ZLIB_LIBDEP TYPE STRING)" \
                >> $HOSTCONF
            fi
        fi
    fi
}

function bv_uintah_ensure
{
    if [[ "$DO_UINTAH" == "yes" && "$USE_SYSTEM_UINTAH" == "no" ]] ; then
        ensure_built_or_ready "uintah" $UINTAH_VERSION $UINTAH_BUILD_DIR $UINTAH_FILE $UINTAH_URL 
        if [[ $? != 0 ]] ; then
            ANY_ERRORS="yes"
            DO_UINTAH="no"
            error "Unable to build UINTAH.  ${UINTAH_FILE} not found."
        fi
    fi
}

function bv_uintah_dry_run
{
  if [[ "$DO_UINTAH" == "yes" ]] ; then
    echo "Dry run option not set for uintah."
  fi
}

# *************************************************************************** #
#                          Function 8.1, build_uintah                           #
# *************************************************************************** #

function build_uintah
{
    #
    # Prepare build dir
    #
    prepare_build_dir $UINTAH_BUILD_DIR $UINTAH_FILE
    untarred_uintah=$?
    if [[ $untarred_uintah == -1 ]] ; then
       warn "Unable to prepare UINTAH Build Directory. Giving Up"
       return 1
    fi

    #
    cd $UINTAH_BUILD_DIR || error "Can't cd to UINTAH build dir."
    if [[ $? != 0 ]]; then
        warn "Patch failed, but continuing."
    fi
    info "Configuring UINTAH . . ."
    cf_darwin=""
    if [[ "$OPSYS" == "Darwin" ]]; then
        export DYLD_LIBRARY_PATH="$VISITDIR/szip/$SZIP_VERSION/$VISITARCH/lib":$DYLD_LIBRARY_PATH
    else
        export LD_LIBRARY_PATH="$VISITDIR/szip/$SZIP_VERSION/$VISITARCH/lib":$LD_LIBRARY_PATH
    fi
    if [[ "$DO_STATIC_BUILD" == "yes" ]]; then
            cf_build_type="--disable-shared --enable-static"
        else
            cf_build_type="--enable-shared --disable-static"
    fi
    cf_szip=""
    if test "x${DO_SZIP}" = "xyes"; then
        info "SZip requested.  Configuring UINTAH with SZip support."
        sz_dir="${VISITDIR}/szip/${SZIP_VERSION}/${VISITARCH}"
        cf_szip="--with-szlib=${sz_dir}"
    fi

    if [[ "$FC_COMPILER" == "no" ]] ; then
        FORTRANARGS=""
    else
        FORTRANARGS="FC=\"$FC_COMPILER\" F77=\"$FC_COMPILER\" FCFLAGS=\"$FCFLAGS\" FFLAGS=\"$FCFLAGS\" --enable-fortran"
    fi

    # In order to ensure $FORTRANARGS is expanded to build the arguments to
    # configure, we wrap the invokation in 'sh -c "..."' syntax
    info "Invoking command to configure UINTAH"
    info "./configure CXX=\"$CXX_COMPILER\" CC=\"$C_COMPILER\" \
        CFLAGS=\"$CFLAGS $C_OPT_FLAGS\" CXXFLAGS=\"$CXXFLAGS $CXX_OPT_FLAGS\" \
        $FORTRANARGS \
        --prefix=\"$VISITDIR/uintah/$UINTAH_VERSION/$VISITARCH\" \
        ${cf_szip} ${cf_darwin}"
    sh -c "./configure CXX=\"$CXX_COMPILER\" CC=\"$C_COMPILER\" \
        CFLAGS=\"$CFLAGS $C_OPT_FLAGS\" CXXFLAGS=\"$CXXFLAGS $CXX_OPT_FLAGS\" \
        $FORTRANARGS \
        --prefix=\"$VISITDIR/uintah/$UINTAH_VERSION/$VISITARCH\" \
        ${cf_szip} ${cf_build_type}"
    if [[ $? != 0 ]] ; then
       warn "UINTAH configure failed.  Giving up"
       return 1
    fi

    #
    # Build UINTAH
    #
    info "Making UINTAH . . ."
    $MAKE $MAKE_OPT_FLAGS
    if [[ $? != 0 ]] ; then
       warn "UINTAH build failed.  Giving up"
       return 1
    fi
    #
    # Install into the VisIt third party location.
    #
    info "Installing UINTAH . . ."

    $MAKE install
    if [[ $? != 0 ]] ; then
       warn "UINTAH install failed.  Giving up"
       return 1
    fi

    if [[ "$DO_STATIC_BUILD" == "no" && "$OPSYS" == "Darwin" ]]; then
        #
        # Make dynamic executable, need to patch up the install path and
        # version information.
        #
        info "Creating dynamic libraries for UINTAH . . ."
    fi

    if [[ "$DO_GROUP" == "yes" ]] ; then
       chmod -R ug+w,a+rX "$VISITDIR/uintah"
       chgrp -R ${GROUP} "$VISITDIR/uintah"
    fi
    cd "$START_DIR"
    info "Done with UINTAH"
    return 0
}

function bv_uintah_is_enabled
{
    if [[ $DO_UINTAH == "yes" ]]; then
        return 1    
    fi
    return 0
}

function bv_uintah_is_installed
{

    if [[ "$USE_SYSTEM_UINTAH" == "yes" ]]; then
        return 1
    fi

    check_if_installed "uintah" $UINTAH_VERSION
    if [[ $? == 0 ]] ; then
        return 1
    fi
    return 0
}

function bv_uintah_build
{
cd "$START_DIR"

if [[ "$DO_UINTAH" == "yes" && "$USE_SYSTEM_UINTAH" == "no" ]] ; then
    check_if_installed "uintah" $UINTAH_VERSION
    if [[ $? == 0 ]] ; then
        info "Skipping UINTAH build.  UINTAH is already installed."
    else
        info "Building UINTAH (~15 minutes)"
        build_uintah
        if [[ $? != 0 ]] ; then
            error "Unable to build or install UINTAH.  Bailing out."
        fi
        info "Done building UINTAH"
    fi
fi
}
