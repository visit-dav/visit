function bv_exodus_initialize
{
export DO_EXODUS="no"
export ON_EXODUS="off"
}

function bv_exodus_enable
{
DO_EXODUS="yes"
ON_EXODUS="on"
DO_NETCDF="yes"
ON_NETCDF="on"
}

function bv_exodus_disable
{
DO_EXODUS="no"
ON_EXODUS="off"
}

function bv_exodus_depends_on
{
return ""
}

function bv_exodus_info
{
export EXODUS_FILE=${EXODUS_FILE:-"exodusii-4.98.tar.gz"}
export EXODUS_VERSION=${EXODUS_VERSION:-"4.98"}
export EXODUS_COMPATIBILITY_VERSION=${EXODUS_COMPATIBILITY_VERSION:-"4.0"}
export EXODUS_BUILD_DIR=${EXODUS_BUILD_DIR:-"exodusii-4.98"}
}

function bv_exodus_print
{
  printf "%s%s\n" "EXODUS_FILE=" "${EXODUS_FILE}"
  printf "%s%s\n" "EXODUS_VERSION=" "${EXODUS_VERSION}"
  printf "%s%s\n" "EXODUS_COMPATIBILITY_VERSION=" "${EXODUS_COMPATIBILITY_VERSION}"
  printf "%s%s\n" "EXODUS_BUILD_DIR=" "${EXODUS_BUILD_DIR}"
}

function bv_exodus_print_usage
{
printf "%-15s %s [%s]\n" "--exodus" "Build Exodus (requires NetCDF)" "$DO_EXODUS"
}

function bv_exodus_graphical
{
local graphical_out="Exodus $EXODUS_VERSION($EXODUS_FILE)    $ON_EXODUS"
echo "$graphical_out"
}

function bv_exodus_host_profile
{
    if [[ "$DO_EXODUS" == "yes" ]] ; then
        echo >> $HOSTCONF
        echo "##" >> $HOSTCONF
        echo "## Exodus" >> $HOSTCONF
        echo "##" >> $HOSTCONF
        echo \
        "VISIT_OPTION_DEFAULT(VISIT_EXODUSII_DIR \${VISITHOME}/exodus/$EXODUS_VERSION/\${VISITARCH})" \
        >> $HOSTCONF
        echo \
        "VISIT_OPTION_DEFAULT(VISIT_EXODUSII_LIBDEP NETCDF_LIBRARY_DIR netcdf \${VISIT_NETCDF_LIBDEP} TYPE STRING)" \
        >> $HOSTCONF
    fi
}

function bv_exodus_ensure
{
    if [[ "$DO_EXODUS" == "yes" ]] ; then
        ensure_built_or_ready "exodus" $EXODUS_VERSION $EXODUS_BUILD_DIR $EXODUS_FILE
        if [[ $? != 0 ]] ; then
            ANY_ERRORS="yes"
            DO_EXODUS="no"
            error "Unable to build ExodusII.  ${EXODUS_FILE} not found."
        fi
    fi
}

function bv_exodus_dry_run
{
  if [[ "$DO_EXODUS" == "yes" ]] ; then
    echo "Dry run option not set for exodus."
  fi
}

# *************************************************************************** #
#                         Function 8.7, build_exodus                          #
#                                                                             #
#  Mark C. Miller, Wed Oct 27 19:26:16 PDT 2010                               #
#  Removed copies of libnetcdf.a and netcdf.h. The header file dependence     #
#  is handled via Exodus plugin's .xml file. The lib dependence is handled    #
#  through LIBDEP.                                                            #
# *************************************************************************** #

function build_exodus
{
    #
    # Prepare build dir
    #
    prepare_build_dir $EXODUS_BUILD_DIR $EXODUS_FILE
    untarred_exodus=$?
    if [[ $untarred_exodus == -1 ]] ; then
       warn "Unable to prepare Exodus Build Directory. Giving Up"
       return 1
    fi

    cd $EXODUS_BUILD_DIR || error "Can't cd to exodus build dir."
    #
    # Build Exodus
    #
    info "Building Exodus . . . (~2 minutes)"

    cd cbind/src
    export NETCDFLIB="$VISITDIR/netcdf/$NETCDF_VERSION/$VISITARCH/lib"
    export NETCDFINC="$VISITDIR/netcdf/$NETCDF_VERSION/$VISITARCH/include"
    ${C_COMPILER} $CFLAGS $C_OPT_FLAGS -I../include -I"$NETCDFINC" -c *.c
    if [[ $? != 0 ]] ; then
        warn "Exodus build failed.  Giving up"
        return 1
    fi
    #
    # Install into the VisIt third party location.
    #
    info "Installing Exodus . . ."

    mkdir "$VISITDIR/exodus"
    mkdir "$VISITDIR/exodus/$EXODUS_VERSION"
    mkdir "$VISITDIR/exodus/$EXODUS_VERSION/$VISITARCH"
    mkdir "$VISITDIR/exodus/$EXODUS_VERSION/$VISITARCH/inc"
    mkdir "$VISITDIR/exodus/$EXODUS_VERSION/$VISITARCH/lib"

    if [[ "$DO_STATIC_BUILD" == "no" && "$OPSYS" == "Darwin" ]]; then
        #
        # Make dynamic executable
        #
        info "Creating dynamic libraries for Exodus . . ."

        INSTALLNAMEPATH="$VISITDIR/exodus/${EXODUS_VERSION}/$VISITARCH/lib"

        ${C_COMPILER} -dynamiclib -o libexoIIv2c.${SO_EXT} *.o \
           -Wl,-headerpad_max_install_names \
           -Wl,-install_name,$INSTALLNAMEPATH/libexoIIv2c.${SO_EXT} \
           -Wl,-compatibility_version,4.0,-current_version,4.46 \
           -L$NETCDFLIB -lnetcdf
        if [[ $? != 0 ]] ; then
           warn "Creation of dynamic library failed.  Giving up"
           return 1
        fi
        cp libexoIIv2c.${SO_EXT} \
           "$VISITDIR/exodus/$EXODUS_VERSION/$VISITARCH/lib"
        cp ../include/*.h "$VISITDIR/exodus/$EXODUS_VERSION/$VISITARCH/inc"
    else
        ar -cr libexoIIv2c.a *.o
        if [[ $? != 0 ]] ; then
           warn "Exodus install failed.  Giving up"
           return 1
        fi
        cp libexoIIv2c.a "$VISITDIR/exodus/$EXODUS_VERSION/$VISITARCH/lib"
        cp ../include/exodusII.h \
           "$VISITDIR/exodus/$EXODUS_VERSION/$VISITARCH/inc"
        cp ../include/exodusII_ext.h \
           "$VISITDIR/exodus/$EXODUS_VERSION/$VISITARCH/inc"
    fi

    if [[ "$DO_GROUP" == "yes" ]] ; then
       chmod -R ug+w,a+rX "$VISITDIR/exodus"
       chgrp -R ${GROUP} "$VISITDIR/exodus"
    fi
    cd "$START_DIR"
    info "Done with Exodus"
    return 0
}

function bv_exodus_build
{
cd "$START_DIR"
if [[ "$DO_EXODUS" == "yes" ]] ; then
    check_if_installed "exodus" $EXODUS_VERSION
    if [[ $? == 0 ]] ; then
        info "Skipping EXODUS build.  EXODUS is already installed."
    else
        info "Building EXODUS (~2 minutes)"
        build_exodus
        if [[ $? != 0 ]] ; then
            error "Unable to build or install EXODUS.  Bailing out."
        fi
        info "Done building EXODUS"
    fi
fi
}

