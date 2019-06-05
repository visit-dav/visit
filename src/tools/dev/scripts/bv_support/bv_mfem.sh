function bv_mfem_initialize
{
    export DO_MFEM="no"
}

function bv_mfem_enable
{
    DO_MFEM="yes"
}

function bv_mfem_disable
{
    DO_MFEM="no"
}

function bv_mfem_depends_on
{
    local depends_on="zlib"

    if [[ "$DO_CONDUIT" == "yes" ]] ; then
        depends_on="$depends_on conduit"
    fi

    echo $depends_on
}

function bv_mfem_info
{
    export MFEM_VERSION=${MFEM_VERSION:-"3.4"}
    export MFEM_FILE=${MFEM_FILE:-"mfem-${MFEM_VERSION}.tgz"}
    export MFEM_BUILD_DIR=${MFEM_BUILD_DIR:-"mfem-${MFEM_VERSION}"}
    export MFEM_URL=${MFEM_URL:-"https://bit.ly/mfem-3-4"}
    export MFEM_MD5_CHECKSUM="59aff55ba3d7d7816cb3efbf84af7724"
    export MFEM_SHA256_CHECKSUM="4e73e4fe0482636de3c5dc983cd395839a83cb16f6f509bd88b053e8b3858e05"
}

function bv_mfem_print
{
    printf "%s%s\n" "MFEM_FILE=" "${MFEM_FILE}"
    printf "%s%s\n" "MFEM_VERSION=" "${MFEM_VERSION}"
    printf "%s%s\n" "MFEM_BUILD_DIR=" "${MFEM_BUILD_DIR}"
}

function bv_mfem_print_usage
{
    printf "%-20s %s [%s]\n" "--mfem" "Build mfem support" "$DO_MFEM"
}

function bv_mfem_host_profile
{
    if [[ "$DO_MFEM" == "yes" ]] ; then
        echo >> $HOSTCONF
        echo "##" >> $HOSTCONF
        echo "## MFEM " >> $HOSTCONF
        echo "##" >> $HOSTCONF
        echo \
            "VISIT_OPTION_DEFAULT(VISIT_MFEM_DIR \${VISITHOME}/mfem/$MFEM_VERSION/\${VISITARCH})" \
            >> $HOSTCONF

        ZLIB_LIBDEP="\${VISITHOME}/zlib/$ZLIB_VERSION/\${VISITARCH}/lib z"

        CONDUIT_LIBDEP=""
        if [[ "$DO_CONDUIT" == "yes" ]] ; then
            CONDUIT_LIBDEP="\${VISIT_CONDUIT_LIBDEP}"
            echo \
                "VISIT_OPTION_DEFAULT(VISIT_MFEM_INCDEP CONDUIT_INCLUDE_DIR TYPE STRING)" \
                    >> $HOSTCONF
        fi


        echo \
            "VISIT_OPTION_DEFAULT(VISIT_MFEM_LIBDEP $CONDUIT_LIBDEP $ZLIB_LIBDEP TYPE STRING)" \
                >> $HOSTCONF
    fi
}

function bv_mfem_ensure
{
    if [[ "$DO_MFEM" == "yes" ]] ; then
        ensure_built_or_ready "mfem" $MFEM_VERSION $MFEM_BUILD_DIR $MFEM_FILE $MFEM_URL
        if [[ $? != 0 ]] ; then
            ANY_ERRORS="yes"
            DO_MFEM="no"
            error "Unable to build mfem.  ${MFEM_FILE} not found."
        fi
    fi
}

function bv_mfem_dry_run
{
    if [[ "$DO_MFEM" == "yes" ]] ; then
        echo "Dry run option not set for mfem."
    fi
}

# *************************************************************************** #
#                            Function 8, build_mfem
# *************************************************************************** #
function build_mfem
{
    #
    # Prepare build dir
    #
    prepare_build_dir $MFEM_BUILD_DIR $MFEM_FILE
    untarred_mfem=$?
    if [[ $untarred_mfem == -1 ]] ; then
        warn "Unable to prepare mfem build directory. Giving Up!"
        return 1
    fi

    cd $MFEM_BUILD_DIR || error "Can't cd to mfem build dir."

    ZLIBARG=-L${VISITDIR}/zlib/${ZLIB_VERSION}/${VISITARCH}/lib

    MFEM_USE_CONDUIT=NO

    if [[ "$DO_CONDUIT" == "yes" ]] ; then
        MFEM_USE_CONDUIT=YES
        CONDUIT_OPT_VALS="-I${VISITDIR}/conduit/${CONDUIT_VERSION}/${VISITARCH}/include/conduit"
        CONDUIT_LIB_VALS="-L${VISITDIR}/conduit/${CONDUIT_VERSION}/${VISITARCH}/lib/ -lconduit_relay -lconduit_blueprint -lconduit"
        # we may also need HDF5, conduit's config.mk includes this info, but mfem isn't using it yet
        if [[ "$DO_HDF5" == "yes" ]] ; then
            CONDUIT_OPT_VALS="${CONDUIT_OPT_VALS} -I${VISITDIR}/hdf5/${HDF5_VERSION}/${VISITARCH}/include/"
            CONDUIT_LIB_VALS="${CONDUIT_OPT_VALS} -L${VISITDIR}/hdf5/${HDF5_VERSION}/${VISITARCH}/ -lhdf5"
        fi
    fi

    #
    # Call configure
    #
    info "Configuring mfem . . ."
    info $MAKE config CXX="$CXX_COMPILER" CXXFLAGS="$CXXFLAGS $CXX_OPT_FLAGS" MFEM_USE_GZSTREAM=YES LDFLAGS="$ZLIBARG -lz" \
              MFEM_USE_CONDUIT=${MFEM_USE_CONDUIT} CONDUIT_OPT="${CONDUIT_OPT_VALS}" CONDUIT_LIB="${CONDUIT_LIB_VALS}"

    $MAKE config CXX="$CXX_COMPILER" CXXFLAGS="$CXXFLAGS $CXX_OPT_FLAGS" MFEM_USE_GZSTREAM=YES LDFLAGS="$ZLIBARG -lz" \
              MFEM_USE_CONDUIT=${MFEM_USE_CONDUIT} CONDUIT_OPT="${CONDUIT_OPT_VALS}" CONDUIT_LIB="${CONDUIT_LIB_VALS}"

    #
    # Build mfem
    #

    info "Building mfem . . . (~2 minutes)"
    $MAKE $MAKE_OPT_FLAGS
    if [[ $? != 0 ]] ; then
        warn "mfem build failed.  Giving up"
        return 1
    fi

    #
    # Install into the VisIt third party location.
    #
    info "Installing mfem"
    $MAKE install PREFIX="$VISITDIR/mfem/$MFEM_VERSION/$VISITARCH/"

    if [[ "$DO_GROUP" == "yes" ]] ; then
        chmod -R ug+w,a+rX "$VISITDIR/mfem"
        chgrp -R ${GROUP} "$VISITDIR/mfem"
    fi
    cd "$START_DIR"
    info "Done with mfem"
    return 0
}


function bv_mfem_is_enabled
{
    if [[ $DO_MFEM == "yes" ]]; then
        return 1    
    fi
    return 0
}

function bv_mfem_is_installed
{
    check_if_installed "mfem" $MFEM_VERSION
    if [[ $? == 0 ]] ; then
        return 1
    fi
    return 0
}

function bv_mfem_build
{
    cd "$START_DIR"
    if [[ "$DO_MFEM" == "yes" ]] ; then
        check_if_installed "mfem" $MFEM_VERSION
        if [[ $? == 0 ]] ; then
            info "Skipping mfem build.  mfem is already installed."
        else
            info "Building mfem (~2 minutes)"
            build_mfem
            if [[ $? != 0 ]] ; then
                error "Unable to build or install mfem.  Bailing out."
            fi
            info "Done building mfem"
        fi
    fi
}
