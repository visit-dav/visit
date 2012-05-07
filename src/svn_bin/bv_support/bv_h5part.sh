function bv_h5part_initialize
{
export DO_H5PART="no"
export ON_H5PART="off"
}

function bv_h5part_enable
{
DO_H5PART="yes"
ON_H5PART="on"
DO_HDF5="yes"
ON_HDF5="on"
DO_SZIP="yes" 
ON_SZIP="on"
}

function bv_h5part_disable
{
DO_H5PART="no"
ON_H5PART="off"
}

function bv_h5part_depends_on
{
echo "szip hdf5"
}

function bv_h5part_info
{
export H5PART_VERSION=${H5PART_VERSION:-"1.6.6"}
export H5PART_FILE=${H5PART_FILE:-"H5Part-${H5PART_VERSION}.tar.gz"}
export H5PART_COMPATIBILITY_VERSION=${H5PART_COMPATIBILITY_VERSION:-"1.6"}
export H5PART_URL=${H5PART_URL:-"https://codeforge.lbl.gov/frs/download.php/387"}
export H5PART_BUILD_DIR=${H5PART_BUILD_DIR:-"H5Part-${H5PART_VERSION}"}
export H5PART_MD5_CHECKSUM="327c63d198e38a12565b74cffdf1f9d7"
export H5PART_SHA256_CHECKSUM="10347e7535d1afbb08d51be5feb0ae008f73caf889df08e3f7dde717a99c7571"
}

function bv_h5part_print
{
  printf "%s%s\n" "H5PART_FILE=" "${H5PART_FILE}"
  printf "%s%s\n" "H5PART_VERSION=" "${H5PART_VERSION}"
  printf "%s%s\n" "H5PART_COMPATIBILITY_VERSION=" "${H5PART_COMPATIBILITY_VERSION}"
  printf "%s%s\n" "H5PART_BUILD_DIR=" "${H5PART_BUILD_DIR}"
}

function bv_h5part_print_usage
{
printf "%-15s %s [%s]\n" "--h5part" "Build H5Part" "$DO_H5PART"
}

function bv_h5part_graphical
{
local graphical_out="H5Part   $H5PART_VERSION($H5PART_FILE)    $ON_H5PART"
echo $graphical_out
}

function bv_h5part_host_profile
{
    if [[ "$DO_H5PART" == "yes" ]] ; then
        echo >> $HOSTCONF
        echo "##" >> $HOSTCONF
        echo "## H5Part" >> $HOSTCONF
        echo "##" >> $HOSTCONF
        echo \
        "VISIT_OPTION_DEFAULT(VISIT_H5PART_DIR \${VISITHOME}/h5part/$H5PART_VERSION/\${VISITARCH})" \
        >> $HOSTCONF
        echo \
        "VISIT_OPTION_DEFAULT(VISIT_H5PART_LIBDEP HDF5_LIBRARY_DIR hdf5 \${VISIT_HDF5_LIBDEP} TYPE STRING)" \
        >> $HOSTCONF

    fi

}

function bv_h5part_ensure
{
   if [[ "$DO_H5PART" == "yes" ]] ; then
        ensure_built_or_ready "h5part" $H5PART_VERSION $H5PART_BUILD_DIR $H5PART_FILE $H5PART_URL
        if [[ $? != 0 ]] ; then
            ANY_ERRORS="yes"
            DO_H5PART="no"
            error "Unable to build H5Part.  ${H5PART_FILE} not found."
        fi
    fi
}

function bv_h5part_dry_run
{
  if [[ "$DO_H5PART" == "yes" ]] ; then
    echo "Dry run option not set for h5part."
  fi
}

# ***************************************************************************
#                         Function 8.10, build_h5part
#
# Modifications:
#
#  Mark C. Miller, Tue Oct 28 11:10:36 PDT 2008
#  Added -DH5_USE_16_API to CFLAGS for configuring H5Part. This should be
#  harmless when building H5Part against versions of HDF5 before 1.8 and
#  necessary when building against versions of HDF5 1.8 or later. It tells
#  HDF5 which version of the HDF5 API H5Part was implemented with.
#
#  Gunther H. Weber, Wed Jul 27 14:48:12 PDT 2011
#  Adapted to H5Part 1.6.3 which can correctly build shared libraries, does
#  not require -DH5_USE_16_API in CFLAGS and has a new way to pass path to
#  HDF5.
#
# ***************************************************************************

function build_h5part
{
    #
    # Prepare build dir
    #
    prepare_build_dir $H5PART_BUILD_DIR $H5PART_FILE
    untarred_h5part=$?
    if [[ $untarred_h5part == -1 ]] ; then
       warn "Unable to prepare H5Part Build Directory. Giving Up"
       return 1
    fi

    #
    info "Configuring H5Part . . ."
    cd $H5PART_BUILD_DIR || error "Can't cd to h5part build dir."
    if [[ "$DO_HDF5" == "yes" ]] ; then
       export HDF5ROOT="$VISITDIR/hdf5/$HDF5_VERSION/$VISITARCH"
       export SZIPROOT="$VISITDIR/szip/$SZIP_VERSION/$VISITARCH"
       WITHHDF5ARG="--with-hdf5=$HDF5ROOT"
       HDF5DYLIB="-L$HDF5ROOT/lib -L$SZIPROOT/lib -lhdf5 -lsz -lz"
    else
       WITHHDF5ARG="--with-hdf5"
       HDF5DYLIB=""
    fi
    if [[ "$OPSYS" == "Darwin" ]]; then
       export DYLD_LIBRARY_PATH="$VISITDIR/hdf5/$HDF5_VERSION/$VISITARCH/lib":\
"$VISITDIR/szip/$SZIP_VERSION/$VISITARCH/lib":\
$DYLD_LIBRARY_PATH
       SOARG="--enable-shared"
    else
       export LD_LIBRARY_PATH="$VISITDIR/hdf5/$HDF5_VERSION/$VISITARCH/lib":\
"$VISITDIR/szip/$SZIP_VERSION/$VISITARCH/lib":\
$LD_LIBRARY_PATH
       SOARG=""
    fi
    if [[ "$FC_COMPILER" == "no" ]] ; then
        FORTRANARGS=""
    else
        FORTRANARGS="FC=\"$FC_COMPILER\" F77=\"$FC_COMPILER\" FCFLAGS=\"$FCFLAGS\" FFLAGS=\"$FCFLAGS\" --enable-fortran"
    fi

    info "Invoking command to configure H5Part"
    # In order to ensure $FORTRANARGS is expanded to build the arguments to
    # configure, we wrap the invokation in 'sh -c "..."' syntax
    sh -c "./configure ${WITHHDF5ARG} ${OPTIONAL} CXX=\"$CXX_COMPILER\" \
       CC=\"$C_COMPILER\" CFLAGS=\"$CFLAGS $C_OPT_FLAGS\" CXXFLAGS=\"$CXXFLAGS $CXX_OPT_FLAGS\" \
       $FORTRANARGS \
       --prefix=\"$VISITDIR/h5part/$H5PART_VERSION/$VISITARCH\""
    if [[ $? != 0 ]] ; then
       warn "H5Part configure failed.  Giving up"
       return 1
    fi

    #
    # Build H5Part
    #
    info "Building H5Part . . . (~1 minutes)"

    $MAKE $MAKE_OPT_FLAGS
    if [[ $? != 0 ]] ; then
       warn "H5Part build failed.  Giving up"
       return 1
    fi
    info "Installing H5Part . . ."

    $MAKE install
    if [[ $? != 0 ]] ; then
       warn "H5Part build (make install) failed.  Giving up"
       return 1
    fi

    if [[ "$DO_STATIC_BUILD" == "no" && "$OPSYS" == "Darwin" ]]; then
        #
        # Make dynamic executable, need to patch up the install path and
        # version information.
        #
        info "Creating dynamic libraries for H5Part . . ."
    fi

    if [[ "$DO_GROUP" == "yes" ]] ; then
       chmod -R ug+w,a+rX "$VISITDIR/h5part"
       chgrp -R ${GROUP} "$VISITDIR/h5part"
    fi
    cd "$START_DIR"
    info "Done with H5Part"
    return 0
}

function bv_h5part_is_enabled
{
    if [[ $DO_H5PART == "yes" ]]; then
        return 1    
    fi
    return 0
}

function bv_h5part_is_installed
{
    check_if_installed "h5part" $H5PART_VERSION
    if [[ $? == 0 ]] ; then
        return 1
    fi
    return 0
}

function bv_h5part_build
{
cd "$START_DIR"
if [[ "$DO_H5PART" == "yes" ]] ; then
    check_if_installed "h5part" $H5PART_VERSION
    if [[ $? == 0 ]] ; then
        info "Skipping H5Part build.  H5Part is already installed."
    else
        info "Building H5Part (~1 minutes)"
        build_h5part
        if [[ $? != 0 ]] ; then
            error "Unable to build or install H5Part.  Bailing out."
        fi
        info "Done building H5Part"
    fi
fi

}

