function bv_advio_initialize
{
    export DO_ADVIO="no"
}

function bv_advio_enable
{
    DO_ADVIO="yes"
}

function bv_advio_disable
{
    DO_ADVIO="no"
}

function bv_advio_depends_on
{
    echo ""
}

function bv_advio_info
{
    export ADVIO_FILE=${ADVIO_FILE:-"AdvIO-1.2.tar.gz"}
    export ADVIO_VERSION=${ADVIO_VERSION:-"1.2"}
    export ADVIO_COMPATIBILITY_VERSION=${ADVIO_COMPATIBILITY_VERSION:-"1.2"}
    export ADVIO_BUILD_DIR=${ADVIO_BUILD_DIR:-AdvIO-1.2}
    export ADVIO_MD5_CHECKSUM="db6def939a2d5dd4d3d6203ba5d3ec7e"
    export ADVIO_SHA256_CHECKSUM="cd89d8a7f1fe94c1bd2d04888028d8b2b98a37853c4a8d5b2b7417b83ea1e803"
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
    printf "%-20s %s [%s]\n" "--advio"   "Build AdvIO" "$DO_ADVIO"
}

function bv_advio_host_profile
{
    if [[ "$DO_ADVIO" == "yes" ]] ; then
        echo >> $HOSTCONF
        echo "##" >> $HOSTCONF
        echo "## AdvIO" >> $HOSTCONF
        echo "##" >> $HOSTCONF
        echo \
            "VISIT_OPTION_DEFAULT(VISIT_ADVIO_DIR \${VISITHOME}/AdvIO/$ADVIO_VERSION/\${VISITARCH})"\
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

function apply_advio_12_mavericks_patch
{
    patch -p0 << \EOF
diff -c AdvIO-1.2/Base/AdvTypes.h.orig AdvIO-1.2/Base/AdvTypes.h
*** AdvIO-1.2/Base/AdvTypes.h.orig      2014-11-19 17:29:43.000000000 -0800
--- AdvIO-1.2/Base/AdvTypes.h   2014-11-19 17:30:32.000000000 -0800
***************
*** 30,53 ****
  #error  c++ support is disabled. Please recompile AdvIO!!
  #endif
  
- #if SIZEOF_BOOL != 0
- 
- /* use C++-builtin boolean (do nothing) */
- 
- #elif USE_STL
- 
- /* use stl's boolean */
- #include <stl.h>
- 
- #else
- 
- /* use own boolean */
- typedef int bool;
- const bool false = 0;
- const bool true = !false;
- 
- #endif
- 
  #else
  /*---------- C ----------*/
  
--- 30,35 ----

EOF
    if [[ $? != 0 ]] ; then
        return 1
    fi

    return 0 
}

function apply_advio_12_patch
{
    if [[ "$OPSYS" == "Darwin" ]] ; then
        if [[ `sw_vers -productVersion` == 10.9.[0-9]* || `sw_vers -productVersion` == 10.1*.[0-9]* ]] ; then
            info "Applying OS X patch . . ."
            apply_advio_12_mavericks_patch
        fi
    fi

    return $?
}

function apply_advio_patch
{
    if [[ ${ADVIO_VERSION} == 1.2 ]] ; then
        apply_advio_12_patch
        if [[ $? != 0 ]] ; then
            return 1
        fi
    fi

    return 0
}

# *************************************************************************** #
#                         Function 8.18, build_advio                          #
#                                                                             #
# Kevin Griffin, Tue Nov 18 18:25:38 PST 2014                                 #
# Added patch for OS X 10.9 Mavericks.                                        #
#                                                                             #
# Kevin Griffin, Mon Aug 8 17:34:52 PDT 2016                                  #
# Updated patch for OS X 10.1*.                                                 #
# *************************************************************************** #

function build_advio
{
    #
    # Prepare build dir
    #
    prepare_build_dir $ADVIO_BUILD_DIR $ADVIO_FILE
    untarred_ADVIO=$?
    # 0, already exists, 1 untarred src, 2 error

    if [[ $untarred_ADVIO == -1 ]] ; then
        warn "Unable to prepare AdvIO Build Directory. Giving up"
        return 1
    fi

    #
    # Apply patches
    #
    info "Patching AdvIO . . ."
    apply_advio_patch
    if [[ $? != 0 ]] ; then
        if [[ $untarred_ADVIO == 1 ]] ; then
            warn "Giving up on AdvIO build because the patch failed."
            return 1
        else 
            warn "Patch failed, but continuing.  I believe that this script\n" \
                 "tried to apply a patch to an existing directory that had\n" \
                 "already been patched ... that is, the patch is\n" \
                 "failing harmlessly on a second application."
        fi
    fi

    #
    # Configure AdvIO
    #
    info "Configuring AdvIO . . ."
    cd $ADVIO_BUILD_DIR || error "Can't cd to AdvIO build dir."
    # Remove IDL dependencies from the build process
    sed "s%@idldir@%%g" Makefile.in > m2
    mv m2 Makefile.in
    sed "s%FileIO IDL DocIO%FileIO DocIO%g" configure > c2
    mv c2 configure
    chmod 750 ./configure
    # configure has a number of broken try-run code-blocks
    if [[ "$OPSYS" == "Darwin" ]]; then
        sed -i orig -e 's/^main()/int main()/' configure
    else
        sed -i.orig -e 's/^main()/int main()/' configure
    fi
    info "Invoking command to configure AdvIO"
    ADVIO_DARWIN=""
    if [[ "$OPSYS" == "Darwin" ]]; then
        ADVIO_DARWIN="--host=darwin"
    fi
    ADVIO_DEBUG=""
    if [[ "$VISIT_BUILD_MODE" == "Debug" ]]; then
        ADVIO_DEBUG="--enable-debug"
    fi
    set -x
    env CXX="$CXX_COMPILER" CC="$C_COMPILER" \
        CFLAGS="$CFLAGS $C_OPT_FLAGS" CXXFLAGS="$CXXFLAGS $CXX_OPT_FLAGS -Wno-error=implicit-function-declaration" \
        ./configure --prefix="$VISITDIR/AdvIO/$ADVIO_VERSION/$VISITARCH" --disable-gtktest $ADVIO_DARWIN $ADVIO_DEBUG
    set +x
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

function bv_advio_is_enabled
{
    if [[ $DO_ADVIO == "yes" ]]; then
        return 1    
    fi
    return 0
}

function bv_advio_is_installed
{
    check_if_installed "AdvIO" $ADVIO_VERSION
    if [[ $? == 0 ]] ; then
        return 1
    fi
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
