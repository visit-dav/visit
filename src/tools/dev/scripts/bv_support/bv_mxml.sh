function bv_mxml_initialize
{
    export DO_MXML="no"
}

function bv_mxml_enable
{
    DO_MXML="yes"
}

function bv_mxml_disable
{
    DO_MXML="no"
}

function bv_mxml_depends_on
{
    echo ""
}

function bv_mxml_info
{
    export MXML_FILE=${MXML_FILE:-"mxml-2.6.tar.gz"}
    export MXML_VERSION=${MXML_VERSION:-"2.6"}
    export MXML_COMPATIBILITY_VERSION=${MXML_COMPATIBILITY_VERSION:-"2.6"}
    export MXML_BUILD_DIR=${MXML_BUILD_DIR:-"mxml-2.6"}
    export MXML_MD5_CHECKSUM="68977789ae64985dddbd1a1a1652642e"
    export MXML_SHA256_CHECKSUM="b0d347da1a0d5a8c9e82f66087d55cfe499728dacae563740d7e733648c69795"
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
    printf "%-20s %s [%s]\n" "--mxml" "Build Mxml" "$DO_MXML"
}

function bv_mxml_ensure
{
    if [[ "$DO_MXML" == "yes" ]] ; then
        ensure_built_or_ready "mxml" $MXML_VERSION $MXML_BUILD_DIR $MXML_FILE
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
        echo "Dry run option not set for MXML."
    fi
}

function apply_mxml_26_darwin_patch
{
    info "Patching MXML for darwin build"
    patch -p0 << \EOF
diff -c mxml-2.6/Makefile.in mxml-2.6/Makefile.in.new
*** mxml-2.6/Makefile.in	2016-06-21 14:05:57.000000000 -0600
--- mxml-2.6/Makefile.in.new	2016-06-21 14:07:38.000000000 -0600
***************
*** 344,353 ****
--- 344,355 ----
  			--header doc/docset.header --intro doc/docset.intro \
  			--css doc/docset.css --title "Mini-XML API Reference" \
  			mxml.xml || exit 1; \
+ 	    if test -e /Developer/usr/bin/docsetutil; then \
  		/Developer/usr/bin/docsetutil package --output org.minixml.xar \
  			--atom org.minixml.atom \
  			--download-url http://www.minixml.org/org.minixml.xar \
  			org.minixml.docset || exit 1; \
+ 	    fi \
  	fi
EOF
    if [[ $? != 0 ]] ; then
        warn "Unable to patch MXML. Wrong version?"
        return 1
    fi

    return 0
}

function apply_mxml_26_patch
{
    if [[ "$OPSYS" == "Darwin" ]]; then
        apply_mxml_26_darwin_patch
        if [[ $? != 0 ]] ; then
            return 1
        fi
    fi

    return 0
}

function apply_mxml_patch
{
    if [[ ${MXML_VERSION} == 2.6 ]] ; then
        apply_mxml_26_patch
        if [[ $? != 0 ]] ; then
            return 1
        fi
    fi

    return 0
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
    # 0, already exists, 1 untarred src, 2 error

    if [[ $untarred_mxml == -1 ]] ; then
        warn "Unable to prepare MXML Build Directory. Giving Up"
        return 1
    fi

    #
    # Apply patches
    #
    apply_mxml_patch
    if [[ $? != 0 ]] ; then
        if [[ $untarred_mxml == 1 ]] ; then
            warn "Giving up on MXML build because the patch failed."
            return 1
        else
            warn "Patch failed, but continuing.  I believe that this script\n" \
                 "tried to apply a patch to an existing directory that had\n" \
                 "already been patched ... that is, the patch is\n" \
                 "failing harmlessly on a second application."
        fi
    fi
    
    #
    # Configure MXML
    #
    cd $MXML_BUILD_DIR || error "Can't cd to MXML build dir."

    info "Configuring MXML . . ."
    ./configure ${OPTIONAL} CXX="$CXX_COMPILER" \
                CC="$C_COMPILER" CFLAGS="$CFLAGS $C_OPT_FLAGS" CXXFLAGS="$CXXFLAGS $CXX_OPT_FLAGS" \
                --prefix="$VISITDIR/mxml/$MXML_VERSION/$VISITARCH" --disable-threads
    if [[ $? != 0 ]] ; then
        warn "mxml configure failed.  Giving up"
        return 1
    fi

    #
    # Build MXML
    #
    info "Building MXML . . . (~1 minutes)"

    $MAKE $MAKE_OPT_FLAGS
    if [[ $? != 0 ]] ; then
        warn "mxml build failed.  Giving up"
        return 1
    fi
    info "Installing MXML . . ."

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
    info "Done with MXML"
    return 0
}

function bv_mxml_is_enabled
{
    if [[ $DO_MXML == "yes" ]]; then
        return 1    
    fi
    return 0
}

function bv_mxml_is_installed
{
    check_if_installed "mxml" $MXML_VERSION
    if [[ $? == 0 ]] ; then
        return 1
    fi
    return 0
}

function bv_mxml_build
{
    if [[ "$DO_MXML" == "yes" ]] ; then
        check_if_installed "mxml" $MXML_VERSION
        if [[ $? == 0 ]] ; then
            info "Skipping build of MXML"
        else
            build_mxml
            if [[ $? != 0 ]] ; then
                error "Unable to build or install MXML.  Bailing out."
            fi
            info "Done building MXML"
        fi
    fi
}

