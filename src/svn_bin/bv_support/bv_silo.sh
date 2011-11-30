function bv_silo_initialize
{
export DO_SILO="no"
export ON_SILO="off"
}

function bv_silo_enable
{
DO_SILO="yes"
ON_SILO="on"
}

function bv_silo_disable
{
DO_SILO="no"
ON_SILO="off"
}

function bv_silo_depends_on
{
return ""
}

function bv_silo_info
{
export SILO_VERSION=${SILO_VERSION:-"4.8"}
export SILO_FILE=${SILO_FILE:-"silo-${SILO_VERSION}.tar.gz"}
export SILO_COMPATIBILITY_VERSION=${SILO_COMPATIBILITY_VERSION:-"4.8"}
export SILO_BUILD_DIR=${SILO_BUILD_DIR:-"silo-${SILO_VERSION}"}
export SILO_URL=${SILO_URL:-https://wci.llnl.gov/codes/silo/silo-${SILO_VERSION}}
}

function bv_silo_print
{
  printf "%s%s\n" "SILO_FILE=" "${SILO_FILE}"
  printf "%s%s\n" "SILO_VERSION=" "${SILO_VERSION}"
  printf "%s%s\n" "SILO_COMPATIBILITY_VERSION=" "${SILO_COMPATIBILITY_VERSION}"
  printf "%s%s\n" "SILO_BUILD_DIR=" "${SILO_BUILD_DIR}"
}

function bv_silo_print_usage
{
printf "%-15s %s [%s]\n" "--silo" "Build Silo support" "$DO_SILO"
}

function bv_silo_graphical
{
local graphical_out="Silo     $SILO_VERSION($SILO_FILE)      $ON_SILO"
echo "$graphical_out"
}

function bv_silo_host_profile
{
    if [[ "$DO_SILO" == "yes" ]] ; then
        echo >> $HOSTCONF
        echo "##" >> $HOSTCONF
        echo "## Silo" >> $HOSTCONF
        echo "##" >> $HOSTCONF
        echo \
        "VISIT_OPTION_DEFAULT(VISIT_SILO_DIR \${VISITHOME}/silo/$SILO_VERSION/\${VISITARCH})" \
        >> $HOSTCONF
        if [[ "$DO_HDF5" == "yes" ]] ; then
            echo \
            "VISIT_OPTION_DEFAULT(VISIT_SILO_LIBDEP HDF5_LIBRARY_DIR hdf5 \${VISIT_HDF5_LIBDEP} TYPE STRING)" \
            >> $HOSTCONF
        else
            echo \
            "VISIT_OPTION_DEFAULT(VISIT_SILO_LIBDEP /usr/lib z TYPE STRING)" \
            >> $HOSTCONF
        fi
    fi
}

function bv_silo_ensure
{
    if [[ "$DO_SILO" == "yes" ]] ; then
        ensure_built_or_ready "silo" $SILO_VERSION $SILO_BUILD_DIR $SILO_FILE $SILO_URL
        if [[ $? != 0 ]] ; then
            ANY_ERRORS="yes"
            DO_SILO="no"
            error "Unable to build Silo.  ${SILO_FILE} not found."
        fi
    fi
}

function bv_silo_dry_run
{
  if [[ "$DO_SILO" == "yes" ]] ; then
    echo "Dry run option not set for silo."
  fi
}

# *************************************************************************** #
#                            Function 8, build_silo
#
# Modfications:
#   Mark C. Miller, Wed Feb 18 22:57:25 PST 2009
#   Added logic to build silex and copy bins on Mac. Removed disablement of
#   browser.
# *************************************************************************** #

function build_silo
{
    #
    # Prepare build dir
    #
    prepare_build_dir $SILO_BUILD_DIR $SILO_FILE
    untarred_silo=$?
    if [[ $untarred_silo == -1 ]] ; then
       warn "Unable to prepare Silo build directory. Giving Up!"
       return 1
    fi
    
    #
    # Call configure
    #
    info "Configuring Silo . . ."
    cd $SILO_BUILD_DIR || error "Can't cd to Silo build dir."
    info "Invoking command to configure Silo"
    SILO_LINK_OPT=""
    if [[ "$DO_HDF5" == "yes" ]] ; then
       export HDF5INCLUDE="$VISITDIR/hdf5/$HDF5_VERSION/$VISITARCH/include"
       export HDF5LIB="$VISITDIR/hdf5/$HDF5_VERSION/$VISITARCH/lib"
       WITHHDF5ARG="--with-hdf5=$HDF5INCLUDE,$HDF5LIB"
       SILO_LINK_OPT="-L$HDF5LIB -lhdf5"
    else
       WITHHDF5ARG="--without-hdf5"
    fi
    SILO_LINK_OPT="$SILO_LINK_OPT -lz"
    if [[ "$DO_SZIP" == "yes" ]] ; then
       export SZIPDIR="$VISITDIR/szip/$SZIP_VERSION/$VISITARCH"
       WITHSZIPARG="--with-szlib=$SZIPDIR"
       SILO_LINK_OPT="$SILO_LINK_OPT -L$SZIPDIR/lib -lsz"
    else
       WITHSZIPARG="--without-szlib"
    fi
    if [[ "$DO_QT" != "yes" || "$OPSYS" == "Darwin" ]] ; then
       WITHSILOQTARG="--disable-silex"
    else
       export SILOQTDIR="${VISITDIR}/qt/${QT_VERSION}/${VISITARCH}"
       WITHSILOQTARG="--with-qt=$SILOQTDIR"
    fi

    if [[ "$FC_COMPILER" == "no" ]] ; then
        FORTRANARGS="--disable-fortran"
    else
        FORTRANARGS="FC=\"$FC_COMPILER\" F77=\"$FC_COMPILER\" FCFLAGS=\"$FCFLAGS\" FFLAGS=\"$FCFLAGS\""
    fi

    info "./configure CXX=\"$CXX_COMPILER\" CC=\"$C_COMPILER\" \
        CFLAGS=\"$CFLAGS $C_OPT_FLAGS\" CXXFLAGS=\"$CXXFLAGS $CXX_OPT_FLAGS\" \
        $FORTRANARGS \
        --prefix=\"$VISITDIR/silo/$SILO_VERSION/$VISITARCH\" \
        \"$WITHHDF5ARG\" \"$WITHSZIPARG\" \"$WITHSILOQTARG\" \
        --without-readline"

    # In order to ensure $FORTRANARGS is expanded to build the arguments to
    # configure, we wrap the invokation in 'sh -c "..."' syntax
    sh -c "./configure CXX=\"$CXX_COMPILER\" CC=\"$C_COMPILER\" \
        CFLAGS=\"$CFLAGS $C_OPT_FLAGS\" CXXFLAGS=\"$CXXFLAGS $CXX_OPT_FLAGS\" \
        $FORTRANARGS \
        --prefix=\"$VISITDIR/silo/$SILO_VERSION/$VISITARCH\" \
        \"$WITHHDF5ARG\" \"$WITHSZIPARG\" \"$WITHSILOQTARG\" \
        --without-readline"

    if [[ $? != 0 ]] ; then
       warn "Silo configure failed.  Giving up"
       return 1
    fi

    #
    # Build Silo
    #
    info "Building Silo . . . (~2 minutes)"
    $MAKE $MAKE_OPT_FLAGS
    if [[ $? != 0 ]] ; then
       warn "Silo build failed.  Giving up"
       return 1
    fi
    #
    # Install into the VisIt third party location.
    #
    info "Installing Silo"

    if [[ "$DO_STATIC_BUILD" == "no" && "$OPSYS" == "Darwin" ]]; then
        mkdir "$VISITDIR/silo"
        mkdir "$VISITDIR/silo/${SILO_VERSION}"
        mkdir "$VISITDIR/silo/${SILO_VERSION}/$VISITARCH"
        mkdir "$VISITDIR/silo/${SILO_VERSION}/$VISITARCH/include"
        mkdir "$VISITDIR/silo/${SILO_VERSION}/$VISITARCH/lib"
        mkdir "$VISITDIR/silo/${SILO_VERSION}/$VISITARCH/bin"
        cp src/silo/silo.h   \
"$VISITDIR/silo/${SILO_VERSION}/$VISITARCH/include"
        cp src/silo/silo.inc \
"$VISITDIR/silo/${SILO_VERSION}/$VISITARCH/include"
        cp src/silo/pmpio.h \
"$VISITDIR/silo/${SILO_VERSION}/$VISITARCH/include"
        cp tools/silex/silex \
"$VISITDIR/silo/${SILO_VERSION}/$VISITARCH/bin"
        cp tools/browser/browser \
"$VISITDIR/silo/${SILO_VERSION}/$VISITARCH/bin"
        cp tools/browser/silodiff \
"$VISITDIR/silo/${SILO_VERSION}/$VISITARCH/bin"
        cp tools/browser/silofile \
"$VISITDIR/silo/${SILO_VERSION}/$VISITARCH/bin"
        cp tools/silock/silock \
"$VISITDIR/silo/${SILO_VERSION}/$VISITARCH/bin"
        #
        # Make dynamic executable
        #
        INSTALLNAMEPATH="$VISITDIR/silo/${SILO_VERSION}/$VISITARCH/lib"

        # Remove the tmp directory if it is already present
        if [[ -d tmp ]] ; then
            rm -rf tmp
        fi
        mkdir tmp
        cd tmp
        SILO_H5=""
        if [[ "$DO_HDF5" == "yes" ]] ; then
            SILO_H5="h5"
        fi
        ar x ../src/.libs/libsilo${SILO_H5}.a
        $CXX_COMPILER -dynamiclib -o libsilo${SILO_H5}.${SO_EXT} *.o \
           -Wl,-headerpad_max_install_names \
           -Wl,-install_name,$INSTALLNAMEPATH/libsilo${SILO_H5}.${SO_EXT} \
           -Wl,-compatibility_version,${SILO_COMPATIBILITY_VERSION} \
           -Wl,-current_version,${SILO_VERSION} $SILO_LINK_OPT
        if [[ $? != 0 ]] ; then
           warn "Silo dynamic library build failed.  Giving up"
           return 1
        fi
        cp libsilo${SILO_H5}.${SO_EXT}  \
"$VISITDIR/silo/${SILO_VERSION}/$VISITARCH/lib"
    else
        $MAKE install
        if [[ $? != 0 ]] ; then
           warn "Silo install failed.  Giving up"
           return 1
        fi
    fi

    if [[ "$DO_GROUP" == "yes" ]] ; then
       chmod -R ug+w,a+rX "$VISITDIR/silo"
       chgrp -R ${GROUP} "$VISITDIR/silo"
    fi
    cd "$START_DIR"
    info "Done with Silo"
    return 0
}

function bv_silo_build
{
cd "$START_DIR"
if [[ "$DO_SILO" == "yes" ]] ; then
    check_if_installed "silo" $SILO_VERSION
    if [[ $? == 0 ]] ; then
        info "Skipping Silo build.  Silo is already installed."
    else
        info "Building Silo (~2 minutes)"
        build_silo
        if [[ $? != 0 ]] ; then
            error "Unable to build or install Silo.  Bailing out."
        fi
        info "Done building Silo"
    fi
fi
}

