function bv_itaps_initialize
{
export DO_ITAPS="no"
export ON_ITAPS="off"
}

function bv_itaps_enable
{
DO_ITAPS="yes"
ON_ITAPS="on"
}

function bv_itaps_disable
{
DO_ITAPS="no"
ON_ITAPS="off"
}

function bv_itaps_depends_on
{
echo ""
}

function bv_itaps_print
{
  printf "%s%s\n" "ITAPS_FMDB_FILE=" "${ITAPS_FMDB_FILE}"
  printf "%s%s\n" "ITAPS_FMDB_VERSION=" "${ITAPS_FMDB_VERSION}"
  printf "%s%s\n" "ITAPS_FMDB_BUILD_DIR=" "${ITAPS_FMDB_BUILD_DIR}"

  printf "%s%s\n" "ITAPS_GRUMMP_FILE=" "${ITAPS_GRUMMP_FILE}"
  printf "%s%s\n" "ITAPS_GRUMMP_VERSION=" "${ITAPS_GRUMMP_VERSION}"
  printf "%s%s\n" "ITAPS_GRUMMP_BUILD_DIR=" "${ITAPS_GRUMMP_BUILD_DIR}"

  printf "%s%s\n" "ITAPS_MOAB_FILE=" "${ITAPS_MOAB_FILE}"
  printf "%s%s\n" "ITAPS_MOAB_VERSION=" "${ITAPS_MOAB_VERSION}"
  printf "%s%s\n" "ITAPS_MOAB_BUILD_DIR=" "${ITAPS_MOAB_BUILD_DIR}"

}

function bv_itaps_print_usage
{
printf "%-15s %s [%s]\n" "--itaps" "Build ITAPS" "${DO_ITAPS}"
}

function bv_itaps_graphical
{
local graphical_out="ITAPS    $ITAPS_VERSION(GRUMMP/FMDB/MOAB) $ON_ITAPS"
echo "$graphical_out"
}

function bv_itaps_host_profile
{
    if [[ "$DO_ITAPS" == "yes" ]] ; then
        echo >> $HOSTCONF
        echo "##" >> $HOSTCONF
        echo "## ITAPS" >> $HOSTCONF
        echo "##" >> $HOSTCONF
        echo "## MOAB implementation" >> $HOSTCONF
        echo "ITAPS_INCLUDE_DIRECTORIES(MOAB \${VISITHOME}/itaps/$ITAPS_VERSION/MOAB/$ITAPS_MOAB_VERSION/\${VISITARCH}/include)" \
        >> $HOSTCONF
        echo "ITAPS_FILE_PATTERNS(MOAB *.cub)" >> $HOSTCONF
        echo "ITAPS_LINK_LIBRARIES(MOAB iMesh MOAB hdf5 sz z netcdf_c++ netcdf vtkGraphics)" \
        >> $HOSTCONF
        echo "ITAPS_LINK_DIRECTORIES(MOAB " \
             "\${VISITHOME}/itaps/$ITAPS_VERSION/MOAB/$ITAPS_MOAB_VERSION/\${VISITARCH}/lib " \
             "\${VISITHOME}/hdf5/${HDF5_VERSION}/\${VISITARCH}/lib " \
             "\${VISITHOME}/szip/${SZIP_VERSION}/\${VISITARCH}/lib " \
             "\${VISITHOME}/netcdf/${NETCDF_VERSION}/\${VISITARCH}/lib)" >> $HOSTCONF
        echo "## FMDB implementation" >> $HOSTCONF
        echo "ITAPS_INCLUDE_DIRECTORIES(FMDB \${VISITHOME}/itaps/$ITAPS_VERSION/FMDB/$ITAPS_FMDB_VERSION/\${VISITARCH}/include)" \
        >> $HOSTCONF
        echo "ITAPS_FILE_PATTERNS(FMDB *.sms)" >> $HOSTCONF
        echo "ITAPS_LINK_LIBRARIES(FMDB FMDB SCORECModel SCORECUtil vtkGraphics)" >> $HOSTCONF
        echo "ITAPS_LINK_DIRECTORIES(FMDB \${VISITHOME}/itaps/$ITAPS_VERSION/FMDB/$ITAPS_FMDB_VERSION/\${VISITARCH}/lib)" >> $HOSTCONF
        echo "## GRUMMP implementation" >> $HOSTCONF
        echo "ITAPS_INCLUDE_DIRECTORIES(GRUMMP \${VISITHOME}/itaps/$ITAPS_VERSION/GRUMMP/$ITAPS_GRUMMP_VERSION/\${VISITARCH}/include)" \
        >> $HOSTCONF
        echo "ITAPS_FILE_PATTERNS(GRUMMP *.bdry *.smesh *.vmesh)" >> $HOSTCONF
        echo "ITAPS_LINK_LIBRARIES(GRUMMP iMesh_GRUMMP GR_3D GR_surf GR_2D GR_geom GR_base SUMAAlog_lite OptMS vtkGraphics cgm dl)" \
        >> $HOSTCONF
        echo "ITAPS_LINK_DIRECTORIES(GRUMMP \${VISITHOME}/itaps/$ITAPS_VERSION/GRUMMP/$ITAPS_GRUMMP_VERSION/\${VISITARCH}/lib)" \
        >> $HOSTCONF
    fi
}



function bv_itaps_info
{
export ITAPS_VERSION=${ITAPS_VERSION:-"1.2"}
export ITAPS_MOAB_VERSION=${ITAPS_MOAB_VERSION:-"4.0.1RC2"}
export ITAPS_MOAB_FILE=${ITAPS_MOAB_FILE:-"moab-${ITAPS_MOAB_VERSION}.tar.gz"}
export ITAPS_MOAB_URL=${ITAPS_MOAB_URL:-http://gnep.mcs.anl.gov:8010}
export ITAPS_MOAB_BUILD_DIR=${ITAPS_MOAB_BUILD_DIR:-"moab-4.0.1"}

export ITAPS_FMDB_VERSION=${ITAPS_FMDB_VERSION:-"1.2"}
export ITAPS_FMDB_FILE=${ITAPS_FMDB_FILE:-"FMDB-${ITAPS_FMDB_VERSION}.tar.gz"}
export ITAPS_FMDB_URL=${ITAPS_FMDB_URL:-https://redmine.scorec.rpi.edu/anonsvn/itaps/software/branches/1.2RC/release_distros/FMDB}
export ITAPS_FMDB_BUILD_DIR=${ITAPS_FMDB_BUILD_DIR:-"${ITAPS_FMDB_FILE%.tar*}"}

# CGM is an indirect dependency introduced by ITAPS_GRUMMP. No other part
# of VisIt uses CGM, presently. That is likely to change in the future
# if we add an iGeom reader to VisIt.
export ITAPS_GRUMMP_CGM_VERSION=${ITAPS_GRUMMP_CGM_VERSION:-"10.2.3"}
export ITAPS_GRUMMP_CGM_FILE=${ITAPS_GRUMMP_CGM_FILE:-"cgma-${ITAPS_GRUMMP_CGM_VERSION}.tar.gz"}
export ITAPS_GRUMMP_CGM_URL=${ITAPS_GRUMMP_CGM_URL:-https://redmine.scorec.rpi.edu/anonsvn/itaps/software/tags/1.2/release_distros}
export ITAPS_GRUMMP_CGM_BUILD_DIR=${ITAPS_GRUMMP_CGM_BUILD_DIR:-"${ITAPS_GRUMMP_CGM_FILE%.tar*}"}

export ITAPS_GRUMMP_VERSION=${ITAPS_GRUMMP_VERSION:-"0.6.3"}
export ITAPS_GRUMMP_FILE=${ITAPS_GRUMMP_FILE:-"GRUMMP-${ITAPS_GRUMMP_VERSION}.tar.gz"}
export ITAPS_GRUMMP_URL=${ITAPS_GRUMMP_URL:-https://redmine.scorec.rpi.edu/anonsvn/itaps/software/tags/1.2/release_distros}
export ITAPS_GRUMMP_BUILD_DIR=${ITAPS_GRUMMP_BUILD_DIR:-"${ITAPS_GRUMMP_FILE%.tar*}"}
export ITAPS_MD5_CHECKSUM=""
export ITAPS_SHA256_CHECKSUM=""
}

function bv_itaps_ensure
{
    if [[ "$DO_ITAPS" == "yes" ]] ; then
        ensure_built_or_ready "itaps"/$ITAPS_VERSION/MOAB $ITAPS_MOAB_VERSION $ITAPS_MOAB_BUILD_DIR  $ITAPS_MOAB_FILE $ITAPS_MOAB_URL
        if [[ $? != 0 ]] ; then
            ANY_ERRORS="yes"
            DO_ITAPS="no"
            error "Unable to build ITAPS_MOAB.  ${ITAPS_MOAB_FILE} not found."
        fi
        ensure_built_or_ready "itaps"/$ITAPS_VERSION/FMDB $ITAPS_FMDB_VERSION $ITAPS_FMDB_BUILD_DIR  $ITAPS_FMDB_FILE $ITAPS_FMDB_URL
        if [[ $? != 0 ]] ; then
            ANY_ERRORS="yes"
            DO_ITAPS="no"
            error "Unable to build ITAPS_FMDB.  ${ITAPS_FMDB_FILE} not found."
        fi
        ensure_built_or_ready "itaps"/$ITAPS_VERSION/GRUMMP $ITAPS_GRUMMP_VERSION $ITAPS_GRUMMP_CGM_BUILD_DIR  $ITAPS_GRUMMP_CGM_FILE $ITAPS_GRUMMP_CGM_URL
        if [[ $? != 0 ]] ; then
            ANY_ERRORS="yes"
            DO_ITAPS="no"
            error "Unable to build ITAPS_GRUMMP_CGM.  ${ITAPS_GRUMMP_CGM_FILE} not found."
        fi
        ensure_built_or_ready "itaps"/$ITAPS_VERSION/GRUMMP $ITAPS_GRUMMP_VERSION $ITAPS_GRUMMP_BUILD_DIR  $ITAPS_GRUMMP_FILE $ITAPS_GRUMMP_URL
        if [[ $? != 0 ]] ; then
            ANY_ERRORS="yes"
            DO_ITAPS="no"
            error "Unable to build ITAPS_GRUMMP.  ${ITAPS_GRUMMP_FILE} not found."
        fi
    fi
}


function bv_itaps_dry_run
{
  if [[ "$DO_ITAPS" == "yes" ]] ; then
    echo "Dry run option not set for itaps."
  fi
}

# *************************************************************************** #
#                          Function 8.15, build_itaps_moab                    #
# *************************************************************************** #

function build_itaps_moab
{
    #
    # Prepare build dir
    #
    prepare_build_dir $ITAPS_MOAB_BUILD_DIR $ITAPS_MOAB_FILE
    untarred_itaps_moab=$?
    if [[ $untarred_itaps_moab == -1 ]] ; then
       warn "Unable to prepare ITAPS_MOAB build directory. Giving Up!"
       return 1
    fi
    
    #
    # Call configure
    #
    info "Configuring ITAPS_MOAB. . ."
    cd $ITAPS_MOAB_BUILD_DIR || error "Cannot cd to $ITAPS_MOAB_BUILD_DIR build dir."
    info "Invoking command to configure ITAPS_MOAB"
    if [[ "$DO_HDF5" == "yes" ]] ; then
       WITHHDF5ARG="--with-hdf5=$VISITDIR/hdf5/$HDF5_VERSION/$VISITARCH --with-hdf5-ldflags=-lz"
    else
       WITHHDF5ARG="--without-hdf5"
    fi
    if [[ "$DO_SZIP" == "yes" ]] ; then
       WITHSZIPARG="--with-szip=$VISITDIR/szip/$SZIP_VERSION/$VISITARCH"
    else
       WITHSZIPARG="--without-szip"
    fi
    if [[ "$DO_NETCDF" == "yes" ]] ; then
       WITHNETCDFARG="--with-netcdf=$VISITDIR/netcdf/$NETCDF_VERSION/$VISITARCH"
    else
       WITHNETCDFARG="--without-netcdf"
    fi
    ./configure CXX="$CXX_COMPILER" CC="$C_COMPILER" \
        CFLAGS="$CFLAGS $C_OPT_FLAGS" CXXFLAGS="$CXXFLAGS $CXX_OPT_FLAGS" \
        --prefix="$VISITDIR/itaps/$ITAPS_VERSION/MOAB/$ITAPS_MOAB_VERSION/$VISITARCH" \
        --with-zlib \
        $WITHHDF5ARG "$WITHSZIPARG" "$WITHNETCDFARG"
    if [[ $? != 0 ]] ; then
       warn "ITAPS_MOAB configure failed.  Giving up"
       return 1
    fi

    #
    # Build ITAPS_MOAB
    #
    info "Building ITAPS_MOAB. . . (~10 minutes)"
    $MAKE $MAKE_OPT_FLAGS
    if [[ $? != 0 ]] ; then
       warn "ITAPS_MOAB build failed.  Giving up"
       return 1
    fi

    #
    # Install into the VisIt third party location.
    #
    info "Installing ITAPS_FMDB"

    if [[ "$OPSYS" == "Darwin" ]]; then
        warn "I do not know how to install ITAPS_MOAB on a Mac. Giving up"
    else
        $MAKE install
        if [[ $? != 0 ]] ; then
           warn "ITAPS_MOAB install failed.  Giving up"
           return 1
        fi
        # To make it easier to package this build up, ensure we don't wind up
        # liking to shared libraries.
        find $VISITDIR/itaps/$ITAPS_VERSION/MOAB/$ITAPS_MOAB_VERSION/$VISITARCH -name 'lib*.so*' -exec rm -f {} \;
    fi

    if [[ "$DO_GROUP" == "yes" ]] ; then
       chmod -R ug+w,a+rX "$VISITDIR/itaps/$ITAPS_VERSION/MOAB"
       chgrp -R ${GROUP} "$VISITDIR/itaps/$ITAPS_VERSION/MOAB"
    fi
    cd "$START_DIR"
    info "Done with ITAPS_MOAB"
    return 0
}

# *************************************************************************** #
#                          Function 8.16, build_itaps_fmdb                    #
# *************************************************************************** #

function build_itaps_fmdb
{
    #
    # Prepare build dir
    #
    prepare_build_dir $ITAPS_FMDB_BUILD_DIR $ITAPS_FMDB_FILE
    untarred_itaps_fmdb=$?
    if [[ $untarred_itaps_fmdb == -1 ]] ; then
       warn "Unable to prepare ITAPS_FMDB build directory. Giving Up!"
       return 1
    fi

    cd $ITAPS_FMDB_BUILD_DIR
    abs_itaps_fmdb_build_dir=$(pwd)

    if [[ ! -e GMI-1.0.tar.gz ]]; then
        download_file GMI-1.0.tar.gz $ITAPS_FMDB_URL
        [[ $? -eq 0 ]] || { error "Unable to download GMI-1.0.tar.gz for ITAPS_FMDB" && return 1; }
    fi
    gunzip < GMI-1.0.tar.gz | tar xf -
    [[ $? -eq 0 ]] || { error "Unable to untar GMI-1.0.tar.gz for ITAPS_FMDB" && return 1; }
    if [[ ! -e SCUtil.tar.gz ]]; then
        download_file SCUtil.tar.gz $ITAPS_FMDB_URL
        [[ $? -eq 0 ]] || { error "Unable to download SCUtil.tar.gz for ITAPS_FMDB" && return 1; }
    fi
    gunzip < SCUtil.tar.gz | tar xf -
    [[ $? -eq 0 ]] || { error "Unable to untar SCUtil.tar.gz for ITAPS_FMDB" && return 1; }
    pushd SCUtil >/dev/null 2>&1
    gunzip < siter.tar.gz | tar xf -
    [[ $? -eq 0 ]] || { error "Unable to untar siter.tar.gz for ITAPS_FMDB" && return 1; }
    gunzip < SCORECUtil-0.1.tar.gz | tar xf -
    [[ $? -eq 0 ]] || { error "Unable to untar SCORECUtil-0.1.tar.gz for ITAPS_FMDB" && return 1; }
    popd >/dev/null 2>&1

    #
    # Configure GMI
    #
    info "Configuring GMI for ITAPS_FMDB. . ."
    pushd GMI-1.0 >/dev/null 2>&1
    ./configure CXX="$CXX_COMPILER" CC="$C_COMPILER" \
        CFLAGS="$CFLAGS $C_OPT_FLAGS" CXXFLAGS="$CXXFLAGS $CXX_OPT_FLAGS" \
        --with-fmdb=$abs_itaps_fmdb_build_dir \
        --with-scorecutil=$abs_itaps_fmdb_build_dir/SCUtil/SCORECUtil-0.1 \
        --with-iterators=$abs_itaps_fmdb_build_dir/SCUtil/siter \
        --prefix="$VISITDIR/itaps/$ITAPS_VERSION/FMDB/$ITAPS_FMDB_VERSION/$VISITARCH"
    if [[ $? != 0 ]] ; then
       warn "ITAPS_FMBD configure failed.  Giving up"
       return 1
    fi
    info "Building GMI for ITAPS_FMDB. . ."
    $MAKE $MAKE_OPT_FLAGS
    if [[ $? != 0 ]] ; then
       warn "ITAPS_FMDB build failed.  Giving up"
       return 1
    fi
    if [[ "$OPSYS" == "Darwin" ]]; then
        warn "I do not know how to install ITAPS_FMDB on a Mac. Giving up"
    else
        $MAKE install
        if [[ $? != 0 ]] ; then
           warn "ITAPS_FMDB install failed.  Giving up"
           return 1
        fi
    fi
    popd >/dev/null 2>&1

    #
    # Configure SCORECUtil
    #
    info "Configuring SCORECUtil for ITAPS_FMDB. . ."
    pushd SCUtil/SCORECUtil-0.1 >/dev/null 2>&1
    ./configure CXX="$CXX_COMPILER" CC="$C_COMPILER" \
        CFLAGS="$CFLAGS $C_OPT_FLAGS" CXXFLAGS="$CXXFLAGS $CXX_OPT_FLAGS" \
        --with-gmi=$VISITDIR/itaps/$ITAPS_VERSION/FMDB/$ITAPS_FMDB_VERSION/$VISITARCH \
        --with-fmdb=$abs_itaps_fmdb_build_dir \
        --with-iterators=$abs_itaps_fmdb_build_dir/SCUtil/siter \
        --prefix="$VISITDIR/itaps/$ITAPS_VERSION/FMDB/$ITAPS_FMDB_VERSION/$VISITARCH"
    if [[ $? != 0 ]] ; then
       warn "ITAPS_FMBD configure failed.  Giving up"
       return 1
    fi
    info "Building SCORECUtil for ITAPS_FMDB. . ."
    $MAKE $MAKE_OPT_FLAGS
    if [[ $? != 0 ]] ; then
       warn "ITAPS_FMDB build failed.  Giving up"
       return 1
    fi
    if [[ "$OPSYS" == "Darwin" ]]; then
        warn "I do not know how to install ITAPS_FMDB on a Mac. Giving up"
    else
        $MAKE install
        if [[ $? != 0 ]] ; then
           warn "ITAPS_FMDB install failed.  Giving up"
           return 1
        fi
    fi
    popd >/dev/null 2>&1
    
    #
    # Configure FMDB
    #
    info "Invoking command to configure ITAPS_FMDB"
    ./configure CXX="$CXX_COMPILER" CC="$C_COMPILER" \
        CFLAGS="$CFLAGS $C_OPT_FLAGS" CXXFLAGS="$CXXFLAGS $CXX_OPT_FLAGS" \
        --with-scorecutil=$VISITDIR/itaps/$ITAPS_VERSION/FMDB/$ITAPS_FMDB_VERSION/$VISITARCH \
        --with-gmi=$VISITDIR/itaps/$ITAPS_VERSION/FMDB/$ITAPS_FMDB_VERSION/$VISITARCH \
        --with-iterators=$abs_itaps_fmdb_build_dir/SCUtil/siter \
        --prefix="$VISITDIR/itaps/$ITAPS_VERSION/FMDB/$ITAPS_FMDB_VERSION/$VISITARCH" \
        --enable-imesh
    if [[ $? != 0 ]] ; then
       warn "ITAPS_FMBD configure failed.  Giving up"
       return 1
    fi

    #
    # Build ITAPS_FMDB
    #
    info "Building ITAPS_FMDB. . . (~10 minutes)"
    $MAKE $MAKE_OPT_FLAGS
    if [[ $? != 0 ]] ; then
       warn "ITAPS_FMDB build failed.  Giving up"
       return 1
    fi

    #
    # Install into the VisIt third party location.
    #
    info "Installing ITAPS_FMDB"

    if [[ "$OPSYS" == "Darwin" ]]; then
        warn "I do not know how to install ITAPS_FMDB on a Mac. Giving up"
    else
        $MAKE install
        if [[ $? != 0 ]] ; then
           warn "ITAPS_FMDB install failed.  Giving up"
           return 1
        fi
        # To make it easier to package this build up, ensure we don't wind up
        # liking to shared libraries.
        find $VISITDIR/itaps/$ITAPS_VERSION/FMDB/$ITAPS_FMDB_VERSION/$VISITARCH -name 'lib*.so*' -exec rm -f {} \;
    fi

    if [[ "$DO_GROUP" == "yes" ]] ; then
       chmod -R ug+w,a+rX "$VISITDIR/itaps/$ITAPS_VERSION/FMDB"
       chgrp -R ${GROUP} "$VISITDIR/itaps/$ITAPS_VERSION/FMDB"
    fi
    cd "$START_DIR"
    info "Done with ITAPS_FMDB"
    return 0
}

# *************************************************************************** #
#                       Function 8.16, build_itaps_grummp                     #
# *************************************************************************** #

function build_itaps_grummp
{
    #
    # Prepare build dir for CGM
    #
    prepare_build_dir $ITAPS_GRUMMP_CGM_BUILD_DIR $ITAPS_GRUMMP_CGM_FILE
    untarred_itaps_cgm_grummp=$?
    if [[ $untarred_itaps_cgm_grummp == -1 ]] ; then
       warn "Unable to prepare ITAPS_GRUMMP_CGM build directory. Giving Up!"
       return 1
    fi

    #
    # Call configure for CGM
    #
    info "Configuring ITAPS_GRUMMP_CGM. . ."
    cd $ITAPS_GRUMMP_CGM_BUILD_DIR || error "Cannot cd to $ITAPS_GRUMMP_CGM_BUILD_DIR build dir."
    info "Invoking command to configure ITAPS_GRUMMP_CGM"
    ./configure CXX="$CXX_COMPILER" CC="$C_COMPILER" \
        CFLAGS="$CFLAGS $C_OPT_FLAGS" CXXFLAGS="$CXXFLAGS $CXX_OPT_FLAGS" \
        --prefix="$VISITDIR/itaps/$ITAPS_VERSION/GRUMMP/$ITAPS_GRUMMP_VERSION/$VISITARCH"
    if [[ $? != 0 ]] ; then
       warn "ITAPS_GRUMMP_CGM configure failed.  Giving up"
       return 1
    fi

    #
    # Build ITAPS_GRUMMP_CGM
    #
    info "Building ITAPS_GRUMMP_CGM. . . (~10 minutes)"
    $MAKE $MAKE_OPT_FLAGS
    if [[ $? != 0 ]] ; then
       warn "ITAPS_GRUMMP_CGM build failed.  Giving up"
       return 1
    fi

    #
    # Install into the VisIt third party location.
    #
    info "Installing ITAPS_GRUMMP_CGM"

    if [[ "$OPSYS" == "Darwin" ]]; then
        warn "I do not know how to install ITAPS_GRUMMP_CGM on a Mac. Giving up"
    else
        $MAKE install
        if [[ $? != 0 ]] ; then
           warn "ITAPS_GRUMMP_CGM install failed.  Giving up"
           return 1
        fi
        # To make it easier to package this build up, ensure we don't wind up
        # liking to shared libraries.
        find $VISITDIR/itaps/$ITAPS_VERSION/GRUMMP/$ITAPS_GRUMMP_VERSION/$VISITARCH -name 'lib*.so*' -exec rm -f {} \;
    fi

    if [[ "$DO_GROUP" == "yes" ]] ; then
       chmod -R ug+w,a+rX "$VISITDIR/itaps/$ITAPS_VERSION/GRUMMP"
       chgrp -R ${GROUP} "$VISITDIR/itaps/$ITAPS_VERSION/GRUMMP"
    fi
    cd "$START_DIR"
    info "Done with ITAPS_GRUMMP_CGM"

    #
    # Prepare build dir
    #
    prepare_build_dir $ITAPS_GRUMMP_BUILD_DIR $ITAPS_GRUMMP_FILE
    untarred_itaps_grummp=$?
    if [[ $untarred_itaps_grummp == -1 ]] ; then
       warn "Unable to prepare ITAPS_GRUMMP build directory. Giving Up!"
       return 1
    fi
    
    #
    # Call configure
    #
    info "Configuring ITAPS_GRUMMP. . ."
    cd $ITAPS_GRUMMP_BUILD_DIR || error "Cannot cd to $ITAPS_GRUMMP_BUILD_DIR build dir."
    info "Invoking command to configure ITAPS_GRUMMP"
    ./configure CXX="$CXX_COMPILER" CC="$C_COMPILER" \
        CFLAGS="$CFLAGS $C_OPT_FLAGS" CXXFLAGS="$CXXFLAGS $CXX_OPT_FLAGS" \
        --prefix="$VISITDIR/itaps/$ITAPS_VERSION/GRUMMP/$ITAPS_GRUMMP_VERSION/$VISITARCH" \
        --with-CGM-path="$VISITDIR/itaps/$ITAPS_VERSION/GRUMMP/$ITAPS_GRUMMP_VERSION/$VISITARCH" \
        --enable-imesh
    if [[ $? != 0 ]] ; then
       warn "ITAPS_GRUMMP configure failed.  Giving up"
       return 1
    fi

    #
    # Build ITAPS_GRUMMP
    #
    info "Building ITAPS_GRUMMP. . . (~10 minutes)"
    $MAKE $MAKE_OPT_FLAGS
    if [[ $? != 0 ]] ; then
       warn "ITAPS_GRUMMP build failed.  Giving up"
       return 1
    fi

    #
    # Install into the VisIt third party location.
    #
    info "Installing ITAPS_GRUMMP"

    if [[ "$OPSYS" == "Darwin" ]]; then
        warn "I do not know how to install ITAPS_GRUMMP on a Mac. Giving up"
    else
        $MAKE install
        if [[ $? != 0 ]] ; then
           warn "ITAPS_GRUMMP install failed.  Giving up"
           return 1
        fi
        # To make it easier to package this build up, ensure we don't wind up
        # liking to shared libraries.
        find $VISITDIR/itaps/$ITAPS_VERSION/GRUMMP/$ITAPS_GRUMMP_VERSION/$VISITARCH -name 'lib*.so*' -exec rm -f {} \;
    fi

    if [[ "$DO_GROUP" == "yes" ]] ; then
       chmod -R ug+w,a+rX "$VISITDIR/itaps/$ITAPS_VERSION/GRUMMP"
       chgrp -R ${GROUP} "$VISITDIR/itaps/$ITAPS_VERSION/GRUMMP"
    fi
    cd "$START_DIR"
    info "Done with ITAPS_GRUMMP"
    return 0
}

function bv_itaps_is_enabled
{
    if [[ $DO_ITAPS == "yes" ]]; then
        return 1    
    fi
    return 0
}

function bv_itaps_is_installed
{
    #TODO: check other versions if necessary
    check_if_installed "itaps/$ITAPS_VERSION/MOAB" $ITAPS_MOAB_VERSION
    if [[ $? == 0 ]] ; then
        return 1
    fi
    return 0
}

function bv_itaps_build
{
cd "$START_DIR"
if [[ "$DO_ITAPS" == "yes" ]] ; then

    # handle MOAB implementation of ITAPS
    check_if_installed "itaps/$ITAPS_VERSION/MOAB" $ITAPS_MOAB_VERSION
    if [[ $? == 0 ]] ; then
        info "Skipping ITAPS_MOAB build.  ITAPS_MOAB is already installed."
    else
        info "Building ITAPS_MOAB (~10 minutes)"
        build_itaps_moab
        if [[ $? != 0 ]] ; then
            error "Unable to build or install ITAPS_MOAB.  Bailing out."
        fi
        info "Done building ITAPS_MOAB"
    fi

    # handle FMDB implementation of ITAPS
    check_if_installed "itaps/$ITAPS_VERSION/FMDB" $ITAPS_FMDB_VERSION
    if [[ $? == 0 ]] ; then
        info "Skipping ITAPS_FMDB build.  ITAPS_FMDB is already installed."
    else
        info "Building ITAPS_FMDB (~10 minutes)"
        build_itaps_fmdb
        if [[ $? != 0 ]] ; then
            error "Unable to build or install ITAPS_FMDB.  Bailing out."
        fi
        info "Done building ITAPS_FMDB"
    fi

    # handle GRUMMP implementation ot ITAPS
    check_if_installed "itaps/$ITAPS_VERSION/GRUMMP" $ITAPS_GRUMMP_VERSION
    if [[ $? == 0 ]] ; then
        info "Skipping ITAPS_GRUMMP build.  ITAPS_GRUMMP is already installed."
    else
        info "Building ITAPS_GRUMMP (~10 minutes)"
        build_itaps_grummp
        if [[ $? != 0 ]] ; then
            error "Unable to build or install ITAPS_GRUMMP.  Bailing out."
        fi
        info "Done building ITAPS_GRUMMP"
    fi
fi

}
