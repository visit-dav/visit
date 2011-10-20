function bv_adios_initialize
{
export DO_ADIOS="no"
export ON_ADIOS="off"
}

function bv_adios_enable
{
DO_ADIOS="yes"
ON_ADIOS="on"
#TODO: temporary until I get dependencies working
DO_MXML="yes"
ON_MXML="on"
}

function bv_adios_disable
{
DO_ADIOS="no"
ON_ADIOS="off"
}

function bv_adios_depends_on
{
return "mxml"
}

function bv_adios_info
{
export ADIOS_FILE=${ADIOS_FILE:-"adios-1.3.tar.gz"}
export ADIOS_VERSION=${ADIOS_VERSION:-"1.3"}
export ADIOS_COMPATIBILITY_VERSION=${ADIOS_COMPATIBILITY_VERSION:-"1.3"}
export ADIOS_BUILD_DIR=${ADIOS_BUILD_DIR:-"adios-1.3"}
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
printf "%-15s %s [%s]\n" "--adios"   "Build ADIOS" "$DO_ADIOS"
}

function bv_adios_graphical
{
local graphical_out="ADIOS    $ADIOS_VERSION($ADIOS_FILE)    $ON_ADIOS"
echo $graphical_out
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
        echo \
        "VISIT_OPTION_DEFAULT(VISIT_ADIOS_DIR \${VISITHOME}/ADIOS/$ADIOS_VERSION/\${VISITARCH})" \
        >> $HOSTCONF
    fi
}

function bv_adios_ensure
{
    if [[ "$DO_ADIOS" == "yes" ]] ; then
        ensure_built_or_ready "ADIOS" $ADIOS_VERSION $ADIOS_BUILD_DIR $ADIOS_FILE
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

function build_ADIOS
{
    #
    # Prepare build dir
    #
    prepare_build_dir $ADIOS_BUILD_DIR $ADIOS_FILE
    untarred_ADIOS=$?
    if [[ $untarred_ADIOS == -1 ]] ; then
       warn "Unable to prepare ADIOS Build Directory. Giving Up"
       return 1
    fi

    #
    info "Configuring ADIOS . . ."
    cd $ADIOS_BUILD_DIR || error "Can't cd to ADIOS build dir."
    info "Invoking command to configure ADIOS"
    if [[ "$VISIT_MPI_COMPILER" != "" ]] ; then
        ./configure ${OPTIONAL} CXX="$CXX_COMPILER" \
            CC="$C_COMPILER" CFLAGS="$CFLAGS $C_OPT_FLAGS" CXXFLAGS="$CXXFLAGS $CXX_OPT_FLAGS" \
            MPICXX="$VISIT_MPI_COMPILER" \
            --with-mxml="$VISITDIR/mxml/$MXML_VERSION/$VISITARCH" \
            --prefix="$VISITDIR/ADIOS/$ADIOS_VERSION/$VISITARCH"
    else
        ./configure ${OPTIONAL} CXX="$CXX_COMPILER" \
            CC="$C_COMPILER" CFLAGS="$CFLAGS $C_OPT_FLAGS" CXXFLAGS="$CXXFLAGS $CXX_OPT_FLAGS" \
            --without-mpi --disable-fortran\
            --with-mxml="$VISITDIR/mxml/$MXML_VERSION/$VISITARCH" \
            --prefix="$VISITDIR/ADIOS/$ADIOS_VERSION/$VISITARCH"
    fi
        
    if [[ $? != 0 ]] ; then
       warn "ADIOS configure failed.  Giving up"
       return 1
    fi

    #
    # Build ADIOS
    #
    info "Building ADIOS . . . (~2 minutes)"

    $MAKE $MAKE_OPT_FLAGS
    if [[ $? != 0 ]] ; then
       warn "ADIOSbuild failed.  Giving up"
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


function bv_adios_build
{
cd "$START_DIR"
if [[ "$DO_ADIOS" == "yes" ]] ; then
    check_if_installed "ADIOS" $ADIOS_VERSION
    if [[ $? == 0 ]] ; then
        info "Skipping ADIOS build.  ADIOS is already installed."
    else
        check_if_installed "mxml" $MXML_VERSION
        if [[ $? == 0 ]] ; then
            info "Skipping build of MXML"
        else
            build_mxml
            if [[ $? != 0 ]] ; then
                 error "Unable to build or install mxml.  Bailing out."
            fi
            info "Done building mxml"
        fi

        info "Building ADIOS (~1 minutes)"
        build_ADIOS
        if [[ $? != 0 ]] ; then
            error "Unable to build or install ADIOS.  Bailing out."
        fi
        info "Done building ADIOS"
   fi
fi
}

