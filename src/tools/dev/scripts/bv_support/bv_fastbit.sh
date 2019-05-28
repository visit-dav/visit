function bv_fastbit_initialize
{
    export DO_FASTBIT="no"
}

function bv_fastbit_enable
{
    DO_FASTBIT="yes"
}

function bv_fastbit_disable
{
    DO_FASTBIT="no"
}

function bv_fastbit_depends_on
{
    echo ""
}

function bv_fastbit_info
{
    export FASTBIT_VERSION=${FASTBIT_VERSION:-"2.0.3"}
    export FASTBIT_FILE=${FASTBIT_FILE:-"fastbit-${FASTBIT_VERSION}.tar.gz"}
    # Note: last 3-digit field in URL changes with version.
    export FASTBIT_URL=${FASTBIT_URL:-"https://code.lbl.gov/frs/download.php/file/426"}
    export FASTBIT_BUILD_DIR=${FASTBIT_BUILD_DIR:-"fastbit-${FASTBIT_VERSION}"}
    export FASTBIT_MD5_CHECKSUM="54825b1d19f6c6a3844b368facc26a9e"
    export FASTBIT_SHA256_CHECKSUM="1ddb16d33d869894f8d8cd745cd3198974aabebca68fa2b83eb44d22339466ec"
}

function bv_fastbit_print
{
    printf "%s%s\n" "FASTBIT_FILE=" "${FASTBIT_FILE}"
    printf "%s%s\n" "FASTBIT_VERSION=" "${FASTBIT_VERSION}"
    printf "%s%s\n" "FASTBIT_BUILD_DIR=" "${FASTBIT_BUILD_DIR}"
}

function bv_fastbit_print_usage
{
    printf "%-20s %s [%s]\n" "--fastbit" "Build FastBit" "$DO_FASTBIT"
    printf "%-20s %s\n" "" "NOTE: FastBit not available for download from web" 
}

function bv_fastbit_host_profile
{
    if [[ "$DO_FASTBIT" == "yes" ]] ; then
        echo >> $HOSTCONF
        echo "##" >> $HOSTCONF
        echo "## FastBit" >> $HOSTCONF
        echo "##" >> $HOSTCONF
        echo "SETUP_APP_VERSION(FASTBIT $FASTBIT_VERSION)" >> $HOSTCONF
        echo \
            "VISIT_OPTION_DEFAULT(VISIT_FASTBIT_DIR \${VISITHOME}/fastbit/\${FASTBIT_VERSION}/\${VISITARCH})" \
            >> $HOSTCONF
    fi

}

function bv_fastbit_ensure
{
    if [[ "$DO_FASTBIT" == "yes" ]] ; then
        ensure_built_or_ready "fastbit" $FASTBIT_VERSION $FASTBIT_BUILD_DIR $FASTBIT_FILE $FASTBIT_URL
        if [[ $? != 0 ]] ; then
            warn "Unable to build FastBit.  ${FASTBIT_FILE} not found."
            warn "FastBit is not available for download from the VisIt build site"
            ANY_ERRORS="yes"
            DO_FASTBIT="no"
            error "Try going to https://codeforge.lbl.gov/frs/?group_id=44"
        fi
    fi
}

function bv_fastbit_dry_run
{
    if [[ "$DO_FASTBIT" == "yes" ]] ; then
        echo "Dry run option not set for fastbit."
    fi
}

function apply_fastbit_2_0_3_patch
{
    info "Patching FastBit"
    patch -p0 << \EOF
diff -rcN fastbit-2.0.3/configure-orig.ac fastbit-2.0.3/configure.ac
*** fastbit-2.0.3/configure-orig.ac	2016-12-14 08:57:26.000000000 -0700
--- fastbit-2.0.3/configure.ac	2016-12-14 08:58:42.000000000 -0700
***************
*** 916,922 ****
  AC_SUBST(BUILD_CONTRIB)
  
  AC_DEFINE_UNQUOTED(FASTBIT_IBIS_INT_VERSION,
!  `echo $PACKAGE_VERSION | awk 'BEGIN { FS="." } { printf("0x%d%.2d%.2d%.2d\n", substr($1, 5), $2, $3, $4); }'`,
  [Define an integer version of FastBit IBIS version number])
  OBJDIR=$objdir
  AC_SUBST(BUILD_JAVA_INTERFACE)
--- 916,922 ----
  AC_SUBST(BUILD_CONTRIB)
  
  AC_DEFINE_UNQUOTED(FASTBIT_IBIS_INT_VERSION,
!  `echo $PACKAGE_VERSION | awk 'BEGIN { FS="." } { printf("0x%d%.2d%.2d%.2d\n", $1, $2, $3, $4); }'`,
  [Define an integer version of FastBit IBIS version number])
  OBJDIR=$objdir
  AC_SUBST(BUILD_JAVA_INTERFACE)

diff -rcN fastbit-2.0.3/configure-orig fastbit-2.0.3/configure
*** fastbit-2.0.3/configure-orig	2016-12-14 09:00:26.000000000 -0700
--- fastbit-2.0.3/configure	2016-12-14 08:54:49.000000000 -0700
***************
*** 17350,17356 ****
  
  
  cat >>confdefs.h <<_ACEOF
! #define FASTBIT_IBIS_INT_VERSION `echo $PACKAGE_VERSION | awk 'BEGIN { FS="." } { printf("0x%d%.2d%.2d%.2d\n", substr($1, 5), $2, $3, $4); }'`
  _ACEOF
  
  OBJDIR=$objdir
--- 17350,17356 ----
  
  
  cat >>confdefs.h <<_ACEOF
! #define FASTBIT_IBIS_INT_VERSION `echo $PACKAGE_VERSION | awk 'BEGIN { FS="." } { printf("0x%d%.2d%.2d%.2d\n", $1, $2, $3, $4); }'`
  _ACEOF
  
  OBJDIR=$objdir

EOF
}

function apply_fastbit_patch
{
    if [[ ${FASTBIT_VERSION} == 2.0.3 ]] ; then
        apply_fastbit_2_0_3_patch
        if [[ $? != 0 ]] ; then
            return 1
        fi
    fi

    return 0
}

# *************************************************************************** #
#                         Function 8.14, build_fastbit                        #
# *************************************************************************** #

function build_fastbit
{
    #
    # Prepare build dir
    #
    prepare_build_dir $FASTBIT_BUILD_DIR $FASTBIT_FILE
    untarred_fastbit=$?
    # 0, already exists, 1 untarred src, 2 error

    if [[ $untarred_fastbit == -1 ]] ; then
        warn "Unable to prepare FastBit Build Directory. Giving Up"
        return 1
    fi

    #
    # Apply patches
    #
    apply_fastbit_patch
    if [[ $? != 0 ]] ; then
        if [[ $untarred_fastbit == 1 ]] ; then
            warn "Giving up on FastBit build because the patch failed."
            return 1
        else
            warn "Patch failed, but continuing.  I believe that this script\n" \
                 "tried to apply a patch to an existing directory that had\n" \
                 "already been patched ... that is, the patch is\n" \
                 "failing harmlessly on a second application."
        fi
    fi

    #
    # Apply configure
    #
    info "Configuring FastBit . . ."
    cd $FASTBIT_BUILD_DIR || error "Can't cd to FastBit build dir."
    
    info "Invoking command to configure FastBit"

    ./configure \
        CXX="$CXX_COMPILER" CC="$C_COMPILER" \
        CFLAGS="$CFLAGS $C_OPT_FLAGS" CXXFLAGS="$CXXFLAGS $CXX_OPT_FLAGS" \
        --prefix="$VISITDIR/fastbit/$FASTBIT_VERSION/$VISITARCH" \
        --disable-shared --with-java=no
    if [[ $? != 0 ]] ; then
        echo "FastBit configure failed.  Giving up"
        return 1
    fi

    #
    # Build FastBit
    #
    info "Building FastBit . . . (~7 minutes)"
    $MAKE $MAKE_OPT_FLAGS
    if [[ $? != 0 ]] ; then
        warn "FastBit build failed.  Giving up"
        return 1
    fi
    
    info "Installing FastBit . . ."
    $MAKE install
    if [[ $? != 0 ]] ; then
        warn "FastBit build (make install) failed.  Giving up"
        return 1
    fi

    if [[ "$DO_GROUP" == "yes" ]] ; then
        chmod -R ug+w,a+rX "$VISITDIR/fastbit"
        chgrp -R ${GROUP} "$VISITDIR/fastbit"
    fi

    cd "$START_DIR"
    info "Done with FastBit"
    return 0
}

function bv_fastbit_is_enabled
{
    if [[ $DO_FASTBIT == "yes" ]]; then
        return 1    
    fi
    return 0
}

function bv_fastbit_is_installed
{
    check_if_installed "fastbit" $FASTBIT_VERSION
    if [[ $? == 0 ]] ; then
        return 1
    fi
    return 0
}

function bv_fastbit_build
{
    cd "$START_DIR"
    if [[ "$DO_FASTBIT" == "yes" ]] ; then
        check_if_installed "fastbit" $FASTBIT_VERSION
        if [[ $? == 0 ]] ; then
            info "Skipping FastBit build.  FastBit is already installed."
        else
            info "Building FastBit (~7 minutes)"
            build_fastbit
            if [[ $? != 0 ]] ; then
                error "Unable to build or install FastBit.  Bailing out."
            fi
            info "Done building FastBit"
        fi
    fi
}
