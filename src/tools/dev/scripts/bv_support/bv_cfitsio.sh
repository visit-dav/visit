function bv_cfitsio_initialize
{
    export DO_CFITSIO="no"
}

function bv_cfitsio_enable
{
    DO_CFITSIO="yes"
}

function bv_cfitsio_disable
{
    DO_CFITSIO="no"
}

function bv_cfitsio_depends_on
{
    echo ""
}

function bv_cfitsio_info
{
    export CFITSIO_FILE=${CFITSIO_FILE:-"cfitsio3006.tar.gz"}
    export CFITSIO_VERSION=${CFITSIO_VERSION:-"3006"}
    export CFITSIO_COMPATIBILITY_VERSION=${CFITSIO_COMPATIBILITY_VERSION:-"3.0"}
    export CFITSIO_BUILD_DIR=${CFITSIO_BUILD_DIR:-"cfitsio"}
    export CFITSIO_SHA256_CHECKSUM="c156ee0becee8987a14229e705f0f9f39dd2b73bbc9e73bc5d69f43896cb9a63"
}

function bv_cfitsio_print
{
    printf "%s%s\n" "CFITSIO_FILE=" "${CFITSIO_FILE}"
    printf "%s%s\n" "CFITSIO_VERSION=" "${CFITSIO_VERSION}"
    printf "%s%s\n" "CFITSIO_COMPATIBILITY_VERSION=" "${CFITSIO_COMPATIBILITY_VERSION}"
    printf "%s%s\n" "CFITSIO_BUILD_DIR=" "${CFITSIO_BUILD_DIR}"
}

function bv_cfitsio_print_usage
{
    printf "%-20s %s [%s]\n" "--cfitsio" "Build CFITSIO" "$DO_CFITSIO"
}

function bv_cfitsio_host_profile
{
    if [[ "$DO_CFITSIO" == "yes" ]] ; then
        echo >> $HOSTCONF
        echo "##" >> $HOSTCONF
        echo "## CFITSIO" >> $HOSTCONF
        echo "##" >> $HOSTCONF
        echo \
            "VISIT_OPTION_DEFAULT(VISIT_CFITSIO_DIR \${VISITHOME}/cfitsio/$CFITSIO_VERSION/\${VISITARCH})" \
            >> $HOSTCONF
    fi

}
function bv_cfitsio_ensure
{
    if [[ "$DO_CFITSIO" == "yes" ]] ; then
        ensure_built_or_ready "cfitsio" $CFITSIO_VERSION $CFITSIO_BUILD_DIR $CFITSIO_FILE
        if [[ $? != 0 ]] ; then
            ANY_ERRORS="yes"
            DO_CFITSIO="no"
            error "Unable to build CFITSIO.  ${CFITSIO_FILE} not found."
        fi
    fi
}
# *************************************************************************** #
#                         Function 8.9, build_cfitsio                         #
# *************************************************************************** #

function apply_mv_vs_cp_patch
{
    patch -p0 << \EOF
--- cfitsio/Makefile.in	2024-02-20 16:26:11.776100000 -0800
+++ cfitsio_patched/Makefile.in	2024-02-20 16:34:10.879376000 -0800
@@ -88,10 +88,10 @@
 
 install:	libcfitsio.a ${CFITSIO_PREFIX} ${CFITSIO_LIB} ${CFITSIO_INCLUDE}
 		@if [ -f libcfitsio.a ]; then \
-			/bin/mv libcfitsio.a ${CFITSIO_LIB}; \
+			/bin/cp libcfitsio.a ${CFITSIO_LIB}; \
 		fi; \
 		if [ -f libcfitsio${SHLIB_SUFFIX} ]; then \
-			/bin/mv libcfitsio${SHLIB_SUFFIX} ${CFITSIO_LIB}; \
+			/bin/cp libcfitsio${SHLIB_SUFFIX} ${CFITSIO_LIB}; \
 		fi; \
 		/bin/cp fitsio.h fitsio2.h longnam.h drvrsmem.h ${CFITSIO_INCLUDE}/

EOF
    if [[ $? != 0 ]] ; then
        warn "CFITSIO Makefile patch failed."
        return 1
    fi

    return 0;

}


function apply_cfitsio_patch
{
    apply_mv_vs_cp_patch
    if [[ $? != 0 ]]; then
        return 1
    fi

    return 0;
}

function build_cfitsio
{
    #
    # Prepare build dir
    #
    prepare_build_dir $CFITSIO_BUILD_DIR $CFITSIO_FILE
    untarred_cfitsio=$?
    if [[ $untarred_cfitsio == -1 ]] ; then
        warn "Unable to prepare CFITSIO Build Directory. Giving Up"
        return 1
    fi

    #
    # Apply patches
    #
    info "Patching cfitsio . . ."
    apply_cfitsio_patch
    if [[ $? != 0 ]] ; then
        if [[ $untarred_cfitsio == 1 ]] ; then
            warn "Giving up on CFITSIO build because the patch failed."
            return 1
        else
            warn "Patch failed, but continuing.  I believe that this script\n" \
                 "tried to apply a patch to an existing directory that had\n" \
                 "already been patched ... that is, the patch is\n" \
                 "failing harmlessly on a second application."
        fi
    fi

    #
    info "Configuring CFITSIO . . ."
    cd $CFITSIO_BUILD_DIR || error "Can't cd to cfits IO build dir."

    set -x
    env CXX="$CXX_COMPILER" CC="$C_COMPILER" \
        CFLAGS="$CFLAGS $C_OPT_FLAGS" CXXFLAGS="$CXXFLAGS $CXX_OPT_FLAGS" \
        ./configure \
        --prefix="$VISITDIR/cfitsio/$CFITSIO_VERSION/$VISITARCH"
    set +x
    if [[ $? != 0 ]] ; then
        warn "CFITSIO configure failed.  Giving up"
        return 1
    fi

    #
    # Build CFITSIO
    #
    info "Building CFITSIO . . . (~2 minutes)"

    $MAKE
    if [[ $? != 0 ]] ; then
        warn "CFITSIO build failed.  Giving up"
        return 1
    fi

    if [[ "$DO_STATIC_BUILD" == "no" && "$OPSYS" == "Darwin" ]]; then
        #
        # Make dynamic executable
        #
        info "Creating dynamic libraries for CFITSIO . . ."

        INSTALLNAMEPATH="$VISITDIR/cfitsio/${CFITSIO_VERSION}/$VISITARCH/lib"
        ## switch back to gcc "external relocation entries" restFP saveFP
        ##      /usr/bin/libtool -o libcfitsio.$SO_EXT -dynamic libcfitsio.a -lSystem \
        ##      -headerpad_max_install_names \
        ##      -install_name $INSTALLNAMEPATH/libcfitsio.$SO_EXT \
        ##      -compatibility_version $CFITSIO_COMPATIBILITY_VERSION \
        ##      -current_version $CFITSIO_VERSION
        gcc -o libcfitsio.$SO_EXT -dynamiclib *.o -lSystem \
            -Wl,-headerpad_max_install_names \
            -Wl,-install_name,$INSTALLNAMEPATH/libcfitsio.$SO_EXT \
            -Wl,-compatibility_version,$CFITSIO_COMPATIBILITY_VERSION \
            -Wl,-current_version,$CFITSIO_VERSION
        if [[ $? != 0 ]] ; then
            warn "Creating dynamic CFITSIO library failed.  Giving up"
            return 1
        fi
        #       cp libcfitsio.$SO_EXT "$VISITDIR/cfitsio/$CFITSIO_VERSION/$VISITARCH/lib"
    fi
    #
    # Install into the VisIt third party location.
    #
    info "Installing CFITSIO . . ."

    mkdir "$VISITDIR/cfitsio"
    mkdir "$VISITDIR/cfitsio/$CFITSIO_VERSION"
    mkdir "$VISITDIR/cfitsio/$CFITSIO_VERSION/$VISITARCH"

    $MAKE install
    if [[ $? != 0 ]] ; then
        warn "CFITSIO install failed.  Giving up"
        return 1
    fi

    if [[ "$DO_GROUP" == "yes" ]] ; then
        chmod -R ug+w,a+rX "$VISITDIR/cfitsio"
        chgrp -R ${GROUP} "$VISITDIR/cfitsio"
    fi
    cd "$START_DIR"
    info "Done with CFITSIO"
    return 0
}

function bv_cfitsio_is_enabled
{
    if [[ $DO_CFITSIO == "yes" ]]; then
        return 1    
    fi
    return 0
}

function bv_cfitsio_is_installed
{
    check_if_installed "cfitsio" $CFITSIO_VERSION
    if [[ $? == 0 ]] ; then
        return 1
    fi
    return 0
}

function bv_cfitsio_build
{
    cd "$START_DIR"
    if [[ "$DO_CFITSIO" == "yes" ]] ; then
        check_if_installed "cfitsio" $CFITSIO_VERSION
        if [[ $? == 0 ]] ; then
            info "Skipping CFITSIO build.  CFITSIO is already installed."
        else
            info "Building CFITSIO (~2 minutes)"
            build_cfitsio
            if [[ $? != 0 ]] ; then
                error "Unable to build or install CFITSIO.  Bailing out."
            fi
            info "Done building CFITSIO"
        fi
    fi
}
