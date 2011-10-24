function bv_szip_initialize
{
export DO_SZIP="no"
export ON_SZIP="off"
}

function bv_szip_enable
{
DO_SZIP="yes"
ON_SZIP="on"
}

function bv_szip_disable
{
DO_SZIP="no"
ON_SZIP="off"
}

function bv_szip_depends_on
{
return ""
}

function bv_szip_info
{
export SZIP_FILE=${SZIP_FILE:-"szip-2.1.tar.gz"}
export SZIP_VERSION=${SZIP_VERSION:-"2.1"}
export SZIP_COMPATIBILITY_VERSION=${SZIP_COMPATIBILITY_VERSION:-"2.0"}
export SZIP_BUILD_DIR=${SZIP_BUILD_DIR:-"szip-2.1"}
}

function bv_szip_print
{
  printf "%s%s\n" "SZIP_FILE=" "${SZIP_FILE}"
  printf "%s%s\n" "SZIP_VERSION=" "${SZIP_VERSION}"
  printf "%s%s\n" "SZIP_COMPATIBILITY_VERSION=" "${SZIP_COMPATIBILITY_VERSION}"
  printf "%s%s\n" "SZIP_BUILD_DIR=" "${SZIP_BUILD_DIR}"
}

function bv_szip_print_usage
{
printf "%-15s %s [%s]\n" "--szip" "Build with SZIP" "$DO_SZIP"  
}

function bv_szip_graphical
{
local graphical_out="SZip     $SZIP_VERSION($SZIP_FILE)      $ON_SZIP"
echo "$graphical_out"
}

function bv_szip_host_profile
{
    if [[ "$DO_SZIP" == "yes" ]] ; then
        echo >> $HOSTCONF
        echo "##" >> $HOSTCONF
        echo "## SZIP" >> $HOSTCONF
        echo "##" >> $HOSTCONF
        echo \
        "VISIT_OPTION_DEFAULT(VISIT_SZIP_DIR \${VISITHOME}/szip/$SZIP_VERSION/\${VISITARCH})" \
        >> $HOSTCONF
    fi
}

function bv_szip_ensure
{    
    if [[ "$DO_SZIP" == "yes" ]] ; then
        ensure_built_or_ready "szip" $SZIP_VERSION $SZIP_BUILD_DIR $SZIP_FILE
        if [[ $? != 0 ]] ; then
            ANY_ERRORS="yes"
            DO_SZIP="no"
            error "Unable to build SZIP.  ${SZIP_FILE} not found."
        fi
    fi
}

function bv_szip_dry_run
{
  if [[ "$DO_SZIP" == "yes" ]] ; then
    echo "Dry run option not set for szip."
  fi
}

# *************************************************************************** #
#                          Function 8.0, build_szip                           #
# *************************************************************************** #

function build_szip
{
    #
    # Prepare build dir
    #
    prepare_build_dir $SZIP_BUILD_DIR $SZIP_FILE
    untarred_szip=$?
    if [[ $untarred_szip == -1 ]] ; then
       warn "Unable to prepare SZip build directory. Giving Up!"
       return 1
    fi

    #
    info "Configuring SZIP . . ."
    cd ${SZIP_BUILD_DIR} || error "Can't cd to szip build dir."
    info "Invoking command to configure SZIP"
    cf_szip=""
    if [[ "$DO_STATIC_BUILD" == "yes" ]]; then
        cf_szip="--disable-shared --enable-static"
    fi

    info "./configure CXX=\"$CXX_COMPILER\" CC=\"$C_COMPILER\" LIBS=\"-lm\" \
        CFLAGS=\"$CFLAGS $C_OPT_FLAGS\" CXXFLAGS=\"$CXXFLAGS $CXX_OPT_FLAGS\" \
        --prefix=\"$VISITDIR/szip/$SZIP_VERSION/$VISITARCH\" ${cf_szip}"

    ./configure CXX="$CXX_COMPILER" CC="$C_COMPILER" LIBS="-lm" \
        CFLAGS="$CFLAGS $C_OPT_FLAGS" CXXFLAGS="$CXXFLAGS $CXX_OPT_FLAGS" \
        --prefix="$VISITDIR/szip/$SZIP_VERSION/$VISITARCH" ${cf_szip}

    if [[ $? != 0 ]] ; then
       warn "SZIP configure failed.  Giving up"
       return 1
    fi

    #
    # Build SZIP
    #
    info "Building SZIP . . . (~1 minutes)"

    $MAKE
    if [[ $? != 0 ]] ; then
       warn "SZIP build failed.  Giving up"
       return 1
    fi
    #
    # Install into the VisIt third party location.
    #
    info "Installing SZIP . . ."

    $MAKE install
    if [[ $? != 0 ]] ; then
       warn "SZIP install failed.  Giving up"
       return 1
    fi

    if [[ "$DO_STATIC_BUILD" == "no" && "$OPSYS" == "Darwin" ]]; then
        #
        # Make dynamic executable, need to patch up the install path and
        # version information.
        #
        info "Creating dynamic libraries for SZIP . . ."
        if [[ $ABS_PATH == "yes" ]]; then
           INSTALLNAMEPATH="$VISITDIR/szip/${SZIP_VERSION}/$VISITARCH/lib"
        else
           INSTALLNAMEPATH="@executable_path/../lib"
        fi
## go back to gcc bacause if "external relocation entries" restFP saveFP
##      /usr/bin/libtool -o libsz.${SO_EXT} -dynamic src/.libs/libsz.a \
##      -lSystem -lz -headerpad_max_install_names \
##      -install_name $INSTALLNAMEPATH/libsz.${SO_EXT} \
##      -compatibility_version $SZIP_COMPATIBILITY_VERSION \
##      -current_version $SZIP_VERSION
        $C_COMPILER -dynamiclib -o libsz.${SO_EXT} src/*.o \
           -Wl,-headerpad_max_install_names \
           -Wl,-twolevel_namespace,-undefined,dynamic_lookup \
           -Wl,-install_name,$INSTALLNAMEPATH/libsz.${SO_EXT} \
           -Wl,-compatibility_version,$SZIP_COMPATIBILITY_VERSION \
           -Wl,-current_version,$SZIP_VERSION -lSystem 
        if [[ $? != 0 ]] ; then
           warn "SZIP dynamic library build failed.  Giving up"
           return 1
        fi
        rm -f "$VISITDIR/szip/$SZIP_VERSION/$VISITARCH/lib/libsz.${SO_EXT}"
        cp libsz.${SO_EXT} "$VISITDIR/szip/$SZIP_VERSION/$VISITARCH/lib"
    fi

    if [[ "$DO_GROUP" == "yes" ]] ; then
       chmod -R ug+w,a+rX "$VISITDIR/szip"
       chgrp -R ${GROUP} "$VISITDIR/szip"
    fi
    cd "$START_DIR"
    info "Done with SZIP"
    return 0
}

function bv_szip_build
{
cd "$START_DIR"
if [[ "$DO_SZIP" == "yes" ]] ; then
    check_if_installed "szip" $SZIP_VERSION
    if [[ $? == 0 ]] ; then
        info "Skipping SZIP build.  SZIP is already installed."
    else
        info "Building SZIP (~2 minutes)"
        build_szip
        if [[ $? != 0 ]] ; then
            error "Unable to build or install SZIP.  Bailing out."
        fi
        info "Done building SZIP"
    fi
fi
}
