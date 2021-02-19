function bv_icet_initialize
{
    export DO_ICET="no"
}

function bv_icet_enable
{
    DO_ICET="yes"
}

function bv_icet_disable
{
    DO_ICET="no"
}

function bv_icet_depends_on
{
    depends_on="cmake"
    if [[ "$DO_MPICH" == "yes" ]] ; then
        depends_on="$depends_on mpich"
    fi

    echo $depends_on
}

function bv_icet_info
{
    export ICET_FILE=${ICET_FILE:-"icet-master-77c708f9090236b576669b74c53e9f105eedbd7e.tar.gz"}
    export ICET_VERSION=${ICET_VERSION:-"77c708f9090236b576669b74c53e9f105eedbd7e"}
    export ICET_COMPATIBILITY_VERSION=${ICET_COMPATIBILITY_VERSION:-"77c708f9090236b576669b74c53e9f105eedbd7e"}
    export ICET_BUILD_DIR=${ICET_BUILD_DIR:-"icet-master-77c708f9090236b576669b74c53e9f105eedbd7e"}
    export ICET_MD5_CHECKSUM="c2e185e7d624b1f1bf0efd41bc83c83c"
    export ICET_SHA256_CHECKSUM="38ed9599b4815b376444223435905b66763912cb66749d90d377ef41d430ba77"
}

function bv_icet_print
{
    printf "%s%s\n" "ICET_FILE=" "${ICET_FILE}"
    printf "%s%s\n" "ICET_VERSION=" "${ICET_VERSION}"
    printf "%s%s\n" "ICET_COMPATIBILITY_VERSION=" "${ICET_COMPATIBILITY_VERSION}"
    printf "%s%s\n" "ICET_BUILD_DIR=" "${ICET_BUILD_DIR}"
}

function bv_icet_print_usage
{
    printf "%-20s %s [%s]\n" "--icet" "Build Ice-T (parallel rendering lib)" "$DO_ICET"
    printf "%-20s %s [%s]\n" "--no-icet" "Prevent Ice-T from being built" "$PREVENT_ICET"
    printf "%-20s %s\n" "" "NOTE: Ice-T is automatically built with --enable-parallel."
}

function bv_icet_host_profile
{
    if [[ "$DO_ICET" == "yes" && "$PREVENT_ICET" != "yes" ]] ; then
        echo >> $HOSTCONF
        echo "##" >> $HOSTCONF
        echo "## Ice-T" >> $HOSTCONF
        echo "##" >> $HOSTCONF
        echo \
            "VISIT_OPTION_DEFAULT(VISIT_ICET_DIR \${VISITHOME}/icet/$ICET_VERSION/\${VISITARCH})" \
            >> $HOSTCONF
    fi
}

function bv_icet_ensure
{
    if [[ "$DO_ICET" == "yes" && "$PREVENT_ICET" != "yes" ]] ; then
        ensure_built_or_ready "icet" $ICET_VERSION $ICET_BUILD_DIR $ICET_FILE "http://icet.sandia.gov/_assets/files"
        if [[ $? != 0 ]] ; then
            ANY_ERRORS="yes"
            DO_ICET="no"
            error "Unable to build Ice-T.  ${ICET_FILE} not found."
        fi
    fi
}

function bv_icet_dry_run
{
    if [[ "$DO_ICET" == "yes" ]] ; then
        echo "Dry run option not set for icet."
    fi
}

# *************************************************************************** #
#                           Function 8.13, build_icet                         #
# *************************************************************************** #

function apply_icet_patch
{
    info "Patching IceT . . ."
    return 0
}

function build_icet
{
    # We need to set MPI_COMPILER for CMake. We can get that from MPICH
    # or from PAR_COMPILER. Note that DO_MPICH causes PAR_COMPILER to be
    # set, so testing for that also catches using MPICH.
    if [[ "$PAR_COMPILER" == "" ]] ; then
        warn "You must use MPICH or set either the PAR_COMPILER environment variable to build Ice-T."
        warn "PAR_COMPILER should be of the form \"/path/to/mpi/bin/mpicc\""
        warn "Giving Up!"
        return 1
    fi

    #
    # CMake is the build system for IceT.  We already required CMake to be
    # built, so it should be there.
    #
    CMAKE_BIN="${CMAKE_COMMAND}"

    prepare_build_dir $ICET_BUILD_DIR $ICET_FILE
    untarred_icet=$?
    if [[ $untarred_icet == -1 ]] ; then
        warn "Unable to prepare Ice-T build directory. Giving Up!"
        return 1
    fi

    apply_icet_patch

    info "Executing CMake on Ice-T"
    cd $ICET_BUILD_DIR || error "Can't cd to IceT build dir."
    if [[ "$DO_STATIC_BUILD" == "no" ]]; then
        LIBEXT="${SO_EXT}"
    else
        LIBEXT="a"
    fi
    rm -f CMakeCache.txt

    if [[ "$OPSYS" == "Darwin" ]] ; then
        ${CMAKE_BIN} \
        -DCMAKE_C_COMPILER:STRING=${C_COMPILER} \
        -DCMAKE_CXX_COMPILER:STRING=${CXX_COMPILER} \
        -DCMAKE_BUILD_TYPE:STRING="${VISIT_BUILD_MODE}" \
        -DCMAKE_C_FLAGS:STRING="${CFLAGS} ${C_OPT_FLAGS}" \
        -DCMAKE_CXX_FLAGS:STRING="${CXXFLAGS} ${CXX_OPT_FLAGS}" \
        -DCMAKE_VERBOSE_MAKEFILE:BOOL=ON \
        -DCMAKE_INSTALL_PREFIX:PATH="$VISITDIR/icet/${ICET_VERSION}/${VISITARCH}"\
        -DCMAKE_C_FLAGS:STRING="-fPIC ${CFLAGS} ${C_OPT_FLAGS}"\
        -DMPI_COMPILER:PATH="${PAR_COMPILER}"\
        -DBUILD_TESTING:BOOL=OFF\
        .
    else
        ${CMAKE_BIN} \
        -DCMAKE_C_COMPILER:STRING=${C_COMPILER} \
        -DCMAKE_CXX_COMPILER:STRING=${CXX_COMPILER} \
        -DCMAKE_BUILD_TYPE:STRING="${VISIT_BUILD_MODE}" \
        -DCMAKE_C_FLAGS:STRING="${CFLAGS} ${C_OPT_FLAGS}" \
        -DCMAKE_CXX_FLAGS:STRING="${CXXFLAGS} ${CXX_OPT_FLAGS}" \
        -DCMAKE_VERBOSE_MAKEFILE:BOOL=ON \
        -DCMAKE_INSTALL_PREFIX:PATH="$VISITDIR/icet/${ICET_VERSION}/${VISITARCH}"\
        -DOPENGL_INCLUDE_DIR:PATH="$VISITDIR/mesa/${MESAGL_VERSION}/${VISITARCH}/include"\
        -DOPENGL_gl_LIBRARY:FILEPATH="$VISITDIR/mesa/${MESAGL_VERSION}/${VISITARCH}/lib/libOSMesa.${LIBEXT}"\
        -DCMAKE_C_FLAGS:STRING="-fPIC ${CFLAGS} ${C_OPT_FLAGS}"\
        -DMPI_COMPILER:PATH="${PAR_COMPILER}"\
        -DBUILD_TESTING:BOOL=OFF\
        .
    fi

    if [[ $? != 0 ]] ; then
        warn "Cannot get CMAKE to create the makefiles.  Giving up."
        return 1
    fi

    #
    # Now build Ice-T.
    #
    info "Building Ice-T . . . (~2 minutes)"
    $MAKE $MAKE_OPT_FLAGS
    if [[ $? != 0 ]] ; then
        warn "Ice-T did not build correctly.  Giving up."
        return 1
    fi

    info "Installing Ice-T . . ."

    $MAKE install
    if [[ $? != 0 ]] ; then
        warn "Ice-T: 'make install' failed.  Giving up"
        return 1
    fi

    if [[ "$DO_GROUP" == "yes" ]] ; then
        chmod -R ug+w,a+rX "$VISITDIR/icet"
        chgrp -R ${GROUP} "$VISITDIR/icet"
    fi

    cd "$START_DIR"
    echo "Done with Ice-T"
    return 0
}

function bv_icet_is_enabled
{
    if [[ $DO_ICET == "yes" ]]; then
        return 1
    fi
    return 0
}

function bv_icet_is_installed
{
    check_if_installed "icet" $ICET_VERSION
    if [[ $? == 0 ]] ; then
        return 1
    fi
    return 0
}

function bv_icet_build
{
    cd "$START_DIR"
    if [[ "$DO_ICET" == "yes" && "$PREVENT_ICET" != "yes" ]] ; then
        check_if_installed "icet" $ICET_VERSION
        if [[ $? == 0 ]] ; then
            info "Skipping Ice-T build.  Ice-T is already installed."
        else
            info "Building Ice-T (~2 minutes)"
            build_icet
            if [[ $? != 0 ]] ; then
                error "Unable to build or install Ice-T.  Bailing out."
            fi
            info "Done building Ice-T"
        fi
    fi
}
