function bv_cgns_initialize
{
export DO_CGNS="no"
export ON_CGNS="off"
}

function bv_cgns_enable
{
DO_CGNS="yes"
ON_CGNS="on"
}

function bv_cgns_disable
{
DO_CGNS="no"
ON_CGNS="off"
}

function bv_cgns_depends_on
{
echo ""
}

function bv_cgns_info
{
export CGNS_FILE=${CGNS_FILE:-"cgns-3.0.8-Source.tar.gz"}
export CGNS_VERSION=${CGNS_VERSION:-"3.0.8"}
export CGNS_COMPATIBILITY_VERSION=${CGNS_COMPATIBILITY_VERSION:-"3.0"}
export CGNS_BUILD_DIR=${CGNS_BUILD_DIR:-"cgns-3.0.8-Source/src"}
export CGNS_MD5_CHECKSUM="812a9a49c98ef32b7cae9954808d6f3e"
export CGNS_SHA256_CHECKSUM=""

}

function bv_cgns_print
{
  printf "%s%s\n" "CGNS_FILE=" "${CGNS_FILE}"
  printf "%s%s\n" "CGNS_VERSION=" "${CGNS_VERSION}"
  printf "%s%s\n" "CGNS_COMPATIBILITY_VERSION=" "${CGNS_COMPATIBILITY_VERSION}"
  printf "%s%s\n" "CGNS_BUILD_DIR=" "${CGNS_BUILD_DIR}"
}

function bv_cgns_print_usage
{
printf "%-15s %s [%s]\n" "--cgns"    "Build CGNS" "$DO_CGNS" 
}

function bv_cgns_graphical
{
local graphical_out="CGNS     $CGNS_VERSION($CGNS_FILE)      $ON_CGNS"
echo $graphical_out
}

function bv_cgns_host_profile
{
    if [[ "$DO_CGNS" == "yes" ]] ; then
        echo >> $HOSTCONF
        echo "##" >> $HOSTCONF
        echo "## CGNS" >> $HOSTCONF
        echo "##" >> $HOSTCONF
        echo \
        "VISIT_OPTION_DEFAULT(VISIT_CGNS_DIR \${VISITHOME}/cgns/$CGNS_VERSION/\${VISITARCH})" \
        >> $HOSTCONF
        if [[ "$DO_HDF5" == "yes" ]] ; then
            echo \
            "VISIT_OPTION_DEFAULT(VISIT_CGNS_LIBDEP HDF5_LIBRARY_DIR hdf5 \${VISIT_HDF5_LIBDEP} TYPE STRING)" \
            >> $HOSTCONF
        fi
    fi

}

function bv_cgns_ensure
{
    if [[ "$DO_CGNS" == "yes" ]] ; then
        ensure_built_or_ready "cgns" $CGNS_VERSION $CGNS_BUILD_DIR $CGNS_FILE
        if [[ $? != 0 ]] ; then
            ANY_ERRORS="yes"
            DO_CGNS="no"
            error "Unable to build CGNS.  ${CGNS_FILE} not found."
        fi
    fi
}

function bv_cgns_dry_run
{
  if [[ "$DO_CGNS" == "yes" ]] ; then
    echo "Dry run option not set for cgns."
  fi
}

# *************************************************************************** #
#                         Function 8.5, build_cgns                            #
# *************************************************************************** #

function build_cgns
{
    #
    # Prepare build dir
    #
    prepare_build_dir $CGNS_BUILD_DIR $CGNS_FILE
    untarred_cgns=$?
    if [[ $untarred_cgns == -1 ]] ; then
       warn "Unable to prepare CGNS Build Directory. Giving Up"
       return 1
    fi

    #
    info "Configuring CGNS . . ."
    cd $CGNS_BUILD_DIR || error "Can't cd to CGNS build dir."
    info "Invoking command to configure CGNS"
    # optionally add HDF5 and szip to the configure.
    H5ARGS=""
    if [[ "$DO_HDF5" == "yes" ]] ; then
        H5ARGS="--with-hdf5=$VISITDIR/hdf5/$HDF5_VERSION/$VISITARCH"
        if [[ "$DO_SZIP" == "yes" ]] ; then
            H5ARGS="$H5ARGS --with-szlib=$VISITDIR/szip/$SZIP_VERSION/$VISITARCH"
        fi
    fi
    info "    env CXX=\"$CXX_COMPILER\" CC=\"$C_COMPILER\" \
       CFLAGS=\"$C_OPT_FLAGS\" CXXFLAGS=\"$CXX_OPT_FLAGS\" \
       ./configure $H5ARGS --prefix=\"$VISITDIR/cgns/$CGNS_VERSION/$VISITARCH\""

    env CXX="$CXX_COMPILER" CC="$C_COMPILER" \
       CFLAGS="$CFLAGS $C_OPT_FLAGS" CXXFLAGS="$CXXFLAGS $CXX_OPT_FLAGS" \
       ./configure $H5ARGS --prefix="$VISITDIR/cgns/$CGNS_VERSION/$VISITARCH"

    if [[ $? != 0 ]] ; then
       warn "CGNS configure failed.  Giving up"
       return 1
    fi

    #
    # Build CGNS
    #
    info "Building CGNS . . . (~2 minutes)"

    $MAKE
    if [[ $? != 0 ]] ; then
       warn "CGNS build failed.  Giving up"
       return 1
    fi
    #
    # Install into the VisIt third party location.
    #
    info "Installing CGNS . . ."

    mkdir "$VISITDIR/cgns"
    mkdir "$VISITDIR/cgns/$CGNS_VERSION"
    mkdir "$VISITDIR/cgns/$CGNS_VERSION/$VISITARCH"
    mkdir "$VISITDIR/cgns/$CGNS_VERSION/$VISITARCH/include"
    mkdir "$VISITDIR/cgns/$CGNS_VERSION/$VISITARCH/lib"
    $MAKE install
    if [[ $? != 0 ]] ; then
       warn "CGNS install failed.  Giving up"
       return 1
    fi

    if [[ "$DO_STATIC_BUILD" == "no" && "$OPSYS" == "Darwin" ]]; then
        #
        # Make dynamic executable
        #
        info "Creating dynamic libraries for CGNS . . ."

        # Check for version >= 8.0.0 (MacOS 10.4, Tiger) for SystemStubs
        VER=$(uname -r)
        if (( ${VER%%.*} > 7 && ${VER%%.*} < 12)) ; then
           USESTUBS="-lSystemStubs"
        else
           USESTUBS=""
        fi

        INSTALLNAMEPATH="$VISITDIR/cgns/${CGNS_VERSION}/$VISITARCH/lib"

        H5LINK=""
        if [[ "$DO_HDF5" == "yes" ]] ; then
            H5LINK="$VISITDIR/hdf5/$HDF5_VERSION/$VISITARCH/lib/libhdf5.dylib"
            if [[ "$DO_SZIP" == "yes" ]] ; then
                H5LINK="$H5LINK $VISITDIR/szip/$SZIP_VERSION/$VISITARCH/lib/libsz.dylib"
            fi
        fi
        /usr/bin/libtool -o libcgns.${SO_EXT} -dynamic DARWIN/libcgns.a \
           -lSystem $USESTUBS $H5LINK -headerpad_max_install_names \
           -install_name $INSTALLNAMEPATH/libcgns.${SO_EXT} \
           -compatibility_version $CGNS_COMPATIBILITY_VERSION \
           -current_version $CGNS_VERSION
        if [[ $? != 0 ]] ; then
           warn "CGNS dynamic library creation failed.  Giving up"
           return 1
        fi
        cp libcgns.${SO_EXT} "$VISITDIR/cgns/$CGNS_VERSION/$VISITARCH/lib"
        rm "$VISITDIR/cgns/$CGNS_VERSION/$VISITARCH/lib/libcgns.a"
    fi

    if [[ "$DO_GROUP" == "yes" ]] ; then
       chmod -R ug+w,a+rX "$VISITDIR/cgns"
       chgrp -R ${GROUP} "$VISITDIR/cgns"
    fi
    cd "$START_DIR"
    info "Done with CGNS"
    return 0
}

function bv_cgns_is_enabled
{
    if [[ $DO_CGNS == "yes" ]]; then
        return 1    
    fi
    return 0
}

function bv_cgns_is_installed
{
    check_if_installed "cgns" $CGNS_VERSION
    if [[ $? == 0 ]] ; then
        return 1
    fi
    return 0
}

function bv_cgns_build
{
cd "$START_DIR"
if [[ "$DO_CGNS" == "yes" ]] ; then
    check_if_installed "cgns" $CGNS_VERSION
    if [[ $? == 0 ]] ; then
        info "Skipping CGNS build.  CGNS is already installed."
    else
        info "Building CGNS (~2 minutes)"
        build_cgns
        if [[ $? != 0 ]] ; then
            error "Unable to build or install CGNS.  Bailing out."
        fi
        info "Done building CGNS"
   fi
fi
}

