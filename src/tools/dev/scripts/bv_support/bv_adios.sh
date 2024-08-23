function bv_adios_initialize
{
    export DO_ADIOS="no"
    export USE_SYSTEM_ADIOS="no"
    add_extra_commandline_args "adios" "alt-adios-dir" 1 "Use alternative directory for adios"

}

function bv_adios_enable
{
    DO_ADIOS="yes"
}

function bv_adios_disable
{
    DO_ADIOS="no"
}

function bv_adios_alt_adios_dir
{
    echo "Using alternate Adios directory"

    # Check to make sure the directory or a particular include file exists.
    #    [ ! -e "$1" ] && error "Adios not found in $1"

    bv_adios_enable
    USE_SYSTEM_ADIOS="yes"
    ADIOS_INSTALL_DIR="$1"
}

function bv_adios_depends_on
{
    if [[ "$USE_SYSTEM_ADIOS" == "yes" ]]; then
        echo ""
    else
        depends_on=""

        if [[ "$DO_MPICH" == "yes" ]] ; then
            depends_on="$depends_on mpich"
        fi

        if [[ "$DO_HDF5" == "yes" ]] ; then
            depends_on="$depends_on hdf5"
        fi

        if [[ "$DO_BLOSC" == "yes" ]] ; then
            depends_on="$depends_on blosc"
        fi

        echo $depends_on
    fi
}

function bv_adios_initialize_vars
{
    if [[ "$parallel" == "no" ]]; then
        bv_adios_disable
        warn "Adios requested by default but the parallel flag has not been set. Adios will not be built."
        return
    fi

    if [[ "$USE_SYSTEM_ADIOS" == "no" ]]; then
        ADIOS_INSTALL_DIR="${VISITDIR}/adios/$ADIOS_VERSION/$VISITARCH"
    fi
}

function bv_adios_info
{
    export ADIOS_VERSION=${ADIOS_VERSION:-"1.13.1"}
    export ADIOS_FILE=${ADIOS_FILE:-"adios-${ADIOS_VERSION}.tar.gz"}
    export ADIOS_COMPATIBILITY_VERSION=${ADIOS_COMPATIBILITY_VERSION:-"${ADIOS_VERSION}"}
    export ADIOS_URL=${ADIOS_URL:-"http://users.nccs.gov/~pnorbert"}
    export ADIOS_BUILD_DIR=${ADIOS_BUILD_DIR:-"adios-${ADIOS_VERSION}"}
    export ADIOS_SHA256_CHECKSUM="684096cd7e5a7f6b8859601d4daeb1dfaa416dfc2d9d529158a62df6c5bcd7a0"
}

function bv_adios_print
{
    printf "%s%s\n" "ADIOS_FILE=" "${ADIOS_FILE}"
    printf "%s%s\n" "ADIOS_VERSION=" "${ADIOS_VERSION}"
    printf "%s%s\n" "ADIOS_COMPATIBILITY_VERSION=" "${ADIOS_COMPATIBILITY_VERSION}"
    printf "%s%s\n" "ADIOS_BUILD_DIR=" "${ADIOS_BUILD_DIR}"
}

function bv_adios_print_usage
{
    printf "%-20s %s [%s]\n" "--adios" "Build ADIOS" "$DO_ADIOS"
    printf "%-20s %s [%s]\n" "--alt-adios-dir" "Use ADIOS from an alternative directory"
}

function bv_adios_host_profile
{
    if [[ "$DO_ADIOS" == "yes" ]] ; then
        echo >> $HOSTCONF
        echo "##" >> $HOSTCONF
        echo "## ADIOS" >> $HOSTCONF
        if [[ "$VISIT_MPI_COMPILER" != "" ]] ; then
            echo "## (configured w/ mpi compiler wrapper)" >> $HOSTCONF
        fi
        echo "##" >> $HOSTCONF

        if [[ "$USE_SYSTEM_ADIOS" == "yes" ]]; then
            warn "Assuming version 1.11.0 for Adios"
            echo "SETUP_APP_VERSION(ADIOS 1.11.0)" >> $HOSTCONF
            echo "VISIT_OPTION_DEFAULT(VISIT_ADIOS_DIR $ADIOS_INSTALL_DIR)" >> $HOSTCONF
        else
            echo "SETUP_APP_VERSION(ADIOS $ADIOS_VERSION)" >> $HOSTCONF
            echo \
                "VISIT_OPTION_DEFAULT(VISIT_ADIOS_DIR \${VISITHOME}/adios/\${ADIOS_VERSION}/\${VISITARCH})" \
                >> $HOSTCONF
        fi
    fi
}

function bv_adios_ensure
{
    if [[ "$DO_ADIOS" == "yes" && "$USE_SYSTEM_ADIOS" == "no" ]] ; then
        ensure_built_or_ready "adios" $ADIOS_VERSION $ADIOS_BUILD_DIR $ADIOS_FILE
        if [[ $? != 0 ]] ; then
            ANY_ERRORS="yes"
            DO_ADIOS="no"
            error "Unable to build ADIOS.  ${ADIOS_FILE} not found."
        fi
    fi
}

# ***************************************************************************
#                         Function 8.22, build_ADIOS
#
# Modifications:
#
# ***************************************************************************

function apply_adios_1_6_0_patch
{
    # fix for osx -- malloc.h doesn't exist (examples/C/schema includes this file)
    info "Patching ADIOS"
    patch -p0 << \EOF
diff -rcN adios-1.6.0-orig/examples/C/schema/rectilinear2d.c adios-1.6.0/examples/C/schema/rectilinear2d.c
*** adios-1.6.0-orig/examples/C/schema/rectilinear2d.c  2013-12-05 08:15:37.000000000 -0800
--- adios-1.6.0/examples/C/schema/rectilinear2d.c       2014-06-02 15:27:23.000000000 -0700
***************
*** 10,16 ****
  #include <stdio.h>
  #include <stdlib.h>
  #include <string.h>
! #include <malloc.h>
  #include <unistd.h>
  #include <fcntl.h>
  #include <errno.h>
--- 10,18 ----
  #include <stdio.h>
  #include <stdlib.h>
  #include <string.h>
! #if !defined(__APPLE__)
!  #include <malloc.h>
! #endif
  #include <unistd.h>
  #include <fcntl.h>
  #include <errno.h>
diff -rcN adios-1.6.0-orig/examples/C/schema/structured2d.c adios-1.6.0/examples/C/schema/structured2d.c
*** adios-1.6.0-orig/examples/C/schema/structured2d.c   2013-12-05 08:15:37.000000000 -0800
--- adios-1.6.0/examples/C/schema/structured2d.c        2014-06-02 15:27:23.000000000 -0700
***************
*** 10,16 ****
  #include <stdio.h>
  #include <stdlib.h>
  #include <string.h>
! #include <malloc.h>
  #include <unistd.h>
  #include <fcntl.h>
  #include <errno.h>
--- 10,18 ----
  #include <stdio.h>
  #include <stdlib.h>
  #include <string.h>
! #if !defined(__APPLE__)
!  #include <malloc.h>
! #endif
  #include <unistd.h>
  #include <fcntl.h>
  #include <errno.h>
diff -rcN adios-1.6.0-orig/examples/C/schema/tri2d.c adios-1.6.0/examples/C/schema/tri2d.c
*** adios-1.6.0-orig/examples/C/schema/tri2d.c  2013-12-05 08:15:37.000000000 -0800
--- adios-1.6.0/examples/C/schema/tri2d.c       2014-06-02 15:27:23.000000000 -0700
***************
*** 10,16 ****
  #include <stdio.h>
  #include <stdlib.h>
  #include <string.h>
! #include <malloc.h>
  #include <unistd.h>
  #include <fcntl.h>
  #include <errno.h>
--- 10,18 ----
  #include <stdio.h>
  #include <stdlib.h>
  #include <string.h>
! #if !defined(__APPLE__)
!  #include <malloc.h>
! #endif
  #include <unistd.h>
  #include <fcntl.h>
  #include <errno.h>
diff -rcN adios-1.6.0-orig/examples/C/schema/uniform2d.c adios-1.6.0/examples/C/schema/uniform2d.c
*** adios-1.6.0-orig/examples/C/schema/uniform2d.c      2013-12-05 08:15:37.000000000 -0800
--- adios-1.6.0/examples/C/schema/uniform2d.c   2014-06-02 15:27:23.000000000 -0700
***************
*** 10,16 ****
  #include <stdio.h>
  #include <stdlib.h>
  #include <string.h>
! #include <malloc.h>
  #include <unistd.h>
  #include <fcntl.h>
  #include <errno.h>
--- 10,18 ----
  #include <stdio.h>
  #include <stdlib.h>
  #include <string.h>
! #if !defined(__APPLE__)
!  #include <malloc.h>
! #endif
  #include <unistd.h>
  #include <fcntl.h>
  #include <errno.h>

EOF
    if [[ $? != 0 ]] ; then
        warn "ADIOS patch failed."
        return 1
    fi

    return 0;
}

function apply_adios_patch
{
    if [[ ${ADIOS_VERSION} == 1.6.0 ]] ; then
        apply_adios_1_6_0_patch
        if [[ $? != 0 ]] ; then
            return 1
        fi
    fi

    return 0
}

function build_adios
{
    #
    # Prepare build dir
    #
    prepare_build_dir $ADIOS_BUILD_DIR $ADIOS_FILE
    untarred_ADIOS=$?
    # 0, already exists, 1 untarred src, 2 error

    if [[ $untarred_ADIOS == -1 ]] ; then
        warn "Unable to prepare ADIOS Build Directory. Giving Up"
        return 1
    fi

    #
    # Apply patches
    #
    apply_adios_patch
    if [[ $? != 0 ]] ; then
        if [[ $untarred_ADIOS == 1 ]] ; then
            warn "Giving up on ADIOS build because the patch failed."
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
    info "Configuring ADIOS . . ."
    cd $ADIOS_BUILD_DIR || error "Can't cd to ADIOS build dir."

    info "Invoking command to configure ADIOS"

    # MPI support
    if [[ "$VISIT_MPI_COMPILER" != "" ]] ; then
        WITH_MPI_ARGS="MPICC=\"$VISIT_MPI_COMPILER\" MPICXX=\"$VISIT_MPI_COMPILER_CXX\" LDFLAGS=\"-lpthread $PAR_LINKER_FLAGS\""
        WITH_MPI_INC="$PAR_INCLUDE"

    else
        WITH_MPI_ARGS="--without-mpi"
    fi

    # HDF5 support
    if [[ "$DO_HDF5" == "yes" ]] ; then
        export HDF5ROOT="$VISITDIR/hdf5/$HDF5_VERSION/$VISITARCH"
        export SZIPROOT="$VISITDIR/szip/$SZIP_VERSION/$VISITARCH"
        WITH_HDF5_ARGS="--with-hdf5=$HDF5ROOT"
        #HDF5_DYLIB="-L$HDF5ROOT/lib -L$SZIPROOT/lib -lhdf5 -lsz -lz"
    else
        WITH_HDF5_ARGS="--without-hdf5"
        #HDF5_DYLIB=""
    fi

    # blosc support
    if [[ "$DO_BLOSC" == "yes" ]]; then
        WITH_BLOSC_ARGS="--with-blosc=$BLOSCDIR"
    else
        WITH_BLOSC_ARGS="--without-blosc"
    fi

    set -x
    sh -c "./configure ${OPTIONAL} CXX=\"$CXX_COMPILER\" CC=\"$C_COMPILER\" \
           CFLAGS=\"$CFLAGS $C_OPT_FLAGS $WITH_MPI_INC\" \
           CXXFLAGS=\"$CXXFLAGS $CXX_OPT_FLAGS $WITH_MPI_INC\" \
           $WITH_MPI_ARGS $WITH_HDF5_ARGS $WITH_BLOSC_ARGS \
           --disable-fortran \
           --without-netcdf --without-nc4par --without-phdf5 --without-mxml \
           --prefix=\"$VISITDIR/adios/$ADIOS_VERSION/$VISITARCH\""

    set +x
    if [[ $? != 0 ]] ; then
        warn "ADIOS configure failed.  Giving up"
        return 1
    fi

    #
    # Apply patches
    #
    apply_adios_patch
    if [[ $? != 0 ]] ; then
        if [[ $untarred_adios == 1 ]] ; then
            warn "Giving up on Adios build because the patch failed."
            return 1
        else
            warn "Patch failed, but continuing.  I believe that this script\n" \
                 "tried to apply a patch to an existing directory that had\n" \
                 "already been patched ... that is, the patch is\n" \
                 "failing harmlessly on a second application."
        fi
    fi

    #
    # Build ADIOS
    #
    info "Building ADIOS . . . (~2 minutes)"
    $MAKE $MAKE_OPT_FLAGS
    if [[ $? != 0 ]] ; then
        warn "ADIOS build failed.  Giving up"
        return 1
    fi

    info "Installing ADIOS . . ."
    $MAKE install
    if [[ $? != 0 ]] ; then
        warn "ADIOS build (make install) failed.  Giving up"
        return 1
    fi

    if [[ "$DO_GROUP" == "yes" ]] ; then
        chmod -R ug+w,a+rX "$VISITDIR/ADIOS"
        chgrp -R ${GROUP} "$VISITDIR/ADIOS"
    fi

    cd "$START_DIR"
    info "Done with ADIOS"
    return 0
}

function bv_adios_is_enabled
{
    if [[ $DO_ADIOS == "yes" ]]; then
        return 1
    fi
    return 0
}

function bv_adios_is_installed
{
    if [[ "$USE_SYSTEM_ADIOS" == "yes" ]]; then
        return 1
    fi

    check_if_installed "adios" $ADIOS_VERSION
    if [[ $? == 0 ]] ; then
        return 1
    fi
    return 0
}

function bv_adios_build
{
    cd "$START_DIR"

    if [[ "$DO_ADIOS" == "yes" && "$USE_SYSTEM_ADIOS" == "no" ]] ; then
        check_if_installed "adios" $ADIOS_VERSION
        if [[ $? == 0 ]] ; then
            info "Skipping ADIOS build.  ADIOS is already installed."
        else
            info "Building ADIOS (~1 minutes)"
            build_adios
            if [[ $? != 0 ]] ; then
                error "Unable to build or install ADIOS.  Bailing out."
            fi
            info "Done building ADIOS"
        fi
    fi
}
