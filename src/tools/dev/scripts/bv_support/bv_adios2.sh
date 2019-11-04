function bv_adios2_initialize
{
    export FORCE_ADIOS2="no"
    export DO_ADIOS2="no"
    export USE_SYSTEM_ADIOS2="no"
    add_extra_commandline_args "adios2" "alt-adios2-dir" 1 "Use alternative directory for adios"

}

function bv_adios2_enable
{
    if [[ "$1" == "force" ]]; then
        FORCE_ADIOS2="yes"
    fi

    DO_ADIOS2="yes"
}

function bv_adios2_disable
{
    DO_ADIOS2="no"
}

function bv_adios2_alt_adios2_dir
{
    echo "Using alternate Adios2 directory"

    # Check to make sure the directory or a particular include file exists.
    #    [ ! -e "$1" ] && error "Adios not found in $1"

    bv_adios2_enable
    USE_SYSTEM_ADIOS2="yes"
    ADIOS2_INSTALL_DIR="$1"
}

function bv_adios2_depends_on
{
    if [[ "$USE_SYSTEM_ADIOS2" == "yes" ]]; then
        echo ""
    else
        depends_on=""

        if [[ "$DO_MPICH" == "yes" ]] ; then
            depends_on="$depends_on mpich"
        fi

        if [[ "$DO_HDF5" == "yes" ]] ; then
            depends_on="$depends_on hdf5"
        fi

        echo $depends_on
    fi
}

function bv_adios2_initialize_vars
{
    if [[ "$USE_SYSTEM_ADIOS2" == "no" ]]; then
        ADIOS2_INSTALL_DIR="${VISITDIR}/adios2/$ADIOS2_VERSION/$VISITARCH"
    fi
}

function bv_adios2_info
{
    export ADIOS2_VERSION=${ADIOS2_VERSION:-"2.5.0"}
    export ADIOS2_FILE=${ADIOS2_FILE:-"adios2-${ADIOS2_VERSION}.tar.gz"}
    export ADIOS2_COMPATIBILITY_VERSION=${ADIOS2_COMPATIBILITY_VERSION:-"${ADIOS2_VERSION}"}
    export ADIOS2_URL=${ADIOS2_URL:-"https://github.com/ornladios/ADIOS2/archive/v2.5.0"}
    export ADIOS2_BUILD_DIR=${ADIOS2_BUILD_DIR:-"ADIOS2-"${ADIOS2_VERSION}}
    export ADIOS2_MD5_CHECKSUM="a50a6bcd02a0a296484a213dca7f9a11"
    export ADIOS2_MD5_CHECKSUM=""
    export ADIOS2_SHA256_CHECKSUM=""
}

function bv_adios2_print
{
    printf "%s%s\n" "ADIOS2_FILE=" "${ADIOS2_FILE}"
    printf "%s%s\n" "ADIOS2_VERSION=" "${ADIOS2_VERSION}"
    printf "%s%s\n" "ADIOS2_COMPATIBILITY_VERSION=" "${ADIOS2_COMPATIBILITY_VERSION}"
    printf "%s%s\n" "ADIOS2_BUILD_DIR=" "${ADIOS2_BUILD_DIR}"
}

function bv_adios2_print_usage
{
    printf "%-20s %s [%s]\n" "--adios2" "Build ADIOS2" "$DO_ADIOS2"
    printf "%-20s %s [%s]\n" "--alt-adios2-dir" "Use ADIOS2 from an alternative directory"
}

function bv_adios2_host_profile
{
    if [[ "$DO_ADIOS2" == "yes" ]] ; then
        echo >> $HOSTCONF
        echo "##" >> $HOSTCONF
        echo "## ADIOS2" >> $HOSTCONF
        echo "##" >> $HOSTCONF

        if [[ "$USE_SYSTEM_ADIOS2" == "yes" ]]; then
            echo "SETUP_APP_VERSION(ADIOS2 $ADIOS2_VERSION)" >> $HOSTCONF
            echo "VISIT_OPTION_DEFAULT(VISIT_ADIOS2_DIR $ADIOS2_INSTALL_DIR)" >> $HOSTCONF
        else
            echo "SETUP_APP_VERSION(ADIOS2 $ADIOS2_VERSION)" >> $HOSTCONF
            echo \
                "VISIT_OPTION_DEFAULT(VISIT_ADIOS2_DIR \${VISITHOME}/adios2-ser/\${ADIOS2_VERSION}/\${VISITARCH})" \
                >> $HOSTCONF

            if [[ "$parallel" == "yes" ]] ; then
                echo "## (configured w/ mpi compiler wrapper)" >> $HOSTCONF
                echo "VISIT_OPTION_DEFAULT(VISIT_ADIOS2_PAR_DIR \${VISITHOME}/adios2-par/\${ADIOS2_VERSION}/\${VISITARCH})" \
                >> $HOSTCONF
            fi
        fi
    fi
}

function bv_adios2_ensure
{
    if [[ "$DO_ADIOS2" == "yes" && "$USE_SYSTEM_ADIOS2" == "no" ]] ; then
        ensure_built_or_ready "adios" $ADIOS2_VERSION $ADIOS2_BUILD_DIR $ADIOS2_FILE
        if [[ $? != 0 ]] ; then
            ANY_ERRORS="yes"
            DO_ADIOS2="no"
            error "Unable to build ADIOS2.  ${ADIOS2_FILE} not found."
        fi
    fi
}

function bv_adios2_dry_run
{
    if [[ "$DO_ADIOS2" == "yes" ]] ; then
        echo "Dry run option not set for adios2."
    fi
}

function build_adios2
{
    #
    # ADIOS2 uses CMake  -- make sure we have it built.
    #
    CMAKE_INSTALL=${CMAKE_INSTALL:-"$VISITDIR/cmake/${CMAKE_VERSION}/$VISITARCH/bin"}
    if [[ -e ${CMAKE_INSTALL}/cmake ]] ; then
        info "ADIOS2: CMake found"
    else
        build_cmake
        if [[ $? != 0 ]] ; then
            warn "Unable to build cmake.  Giving up"
            return 1
        fi
    fi

    #
    # Prepare build dir
    #
    prepare_build_dir $ADIOS2_BUILD_DIR $ADIOS2_FILE
    untarred_ADIOS2=$?
    if [[ $untarred_ADIOS2 == -1 ]] ; then
        warn "Unable to prepare ADIOS2 Build Directory. Giving Up"
        return 1
    fi
    #### begin parallel

    par_build_types="ser"
    if [[ "$parallel" == "yes" ]]; then
        par_build_types="$par_build_types par"
    fi

    ADIOS2_SRC_DIR=$ADIOS2_BUILD_DIR

    for bt in $par_build_types; do

        # Configure.
        cd $ADIOS2_SRC_DIR || error "Can't cd to $ADIOS2_SRC_DIR"
        info "Configuring ADIOS2-$bt (~1 minute)"

        if [[ "$bt" == "par" ]]; then

            SED_CMD="sed -i "

            # sed for OSX is different then most Linux distros in that you have
            # to use a few extra characters to get it to do the same command (see
            # https://ed.gs/2016/01/26/os-x-sed-invalid-command-code/).
            if [[ "$OPSYS" == "Darwin" ]]; then
                SED_CMD="sed -i \"\" "
            fi

            # Change all references from adios2 to adios2_mpi.
            find . -name "CMakeLists.txt" -exec sed -i "s/adios2/adios2_mpi/g" {} \;
            # This changes too many things, now we need to change specific things back.

            ${SED_CMD} "s/adios2_mpi/adios2/g" source/CMakeLists.txt
            find . -name "CMakeLists.txt" -exec ${SED_CMD} "s/adios2_mpi.h/adios2.h/g" {} \;
            find . -name "CMakeLists.txt" -exec ${SED_CMD} "s/adios2_mpi\//adios2\//g" {} \;
            find . -name "CMakeLists.txt" -exec ${SED_CMD} "s/adios2_mpi_/adios2_/g" {} \;
            find . -name "CMakeLists.txt" -exec ${SED_CMD} "s/adios2_mpi-/adios2-/g" {} \;
            find . -name "CMakeLists.txt" -exec ${SED_CMD} "s/adios2_mpi::/adios2::/g" {} \;
            find . -name "CMakeLists.txt" -exec ${SED_CMD} "s/adios2_mpisys/adios2sys/g" {} \;
            find . -name "CMakeLists.txt" -exec ${SED_CMD} "s/\/adios2_mpi/\/adios2/g" {} \;
            find . -name "CMakeLists.txt" -exec ${SED_CMD} "s/adios2_mpiExports/adios2Exports/g" {} \;
            ${SED_CMD} "s/adios2.helper/adios2\/helper/g" source/adios2/toolkit/sst/CMakeLists.txt
            ${SED_CMD} "s/find_package(adios2_mpi/find_package(adios2/g" cmake/install/post/adios2-config-dummy/CMakeLists.txt
        fi

        # Make a build directory for an out-of-source build.. Change the
        # VISIT_BUILD_DIR variable to represent the out-of-source build directory.
        ADIOS2_BUILD_DIR="${ADIOS2_SRC_DIR}-$bt-build"

        if [[ ! -d $ADIOS2_BUILD_DIR ]] ; then
            echo "Making build directory $ADIOS2_BUILD_DIR"
            mkdir $ADIOS2_BUILD_DIR
        fi

        cd $ADIOS2_BUILD_DIR || error "Can't cd to $ADIOS2_BUILD_DIR"

        #
        # Remove the CMakeCache.txt files ... existing files sometimes prevent
        # fields from getting overwritten properly.
        #
        rm -Rf $ADIOS2_BUILD_DIR/CMakeCache.txt $ADIOS2_BUILD_DIR/*/CMakeCache.txt

        adios2_build_mode="${VISIT_BUILD_MODE}"
        adios2_install_path="${VISITDIR}/adios2-$bt/${ADIOS2_VERSION}/${VISITARCH}"

        cfg_opts=""
        cfg_opts="${cfg_opts} -DADIOS2_BUILD_EXAMPLES:BOOL=OFF"
        cfg_opts="${cfg_opts} -DADIOS2_BUILD_TESTING:BOOL=OFF"
        cfg_opts="${cfg_opts} -DADIOS2_USE_ZeroMQ:BOOL=OFF"
        cfg_opts="${cfg_opts} -DADIOS2_USE_Fortran:BOOL=OFF"

        if test "x${DO_STATIC_BUILD}" = "xyes" ; then
            cfg_opts="${cfg_opts} -DBUILD_SHARED_LIBS:BOOL=OFF"
        else
            cfg_opts="${cfg_opts} -DBUILD_SHARED_LIBS:BOOL=ON"
        fi
        cfg_opts="${cfg_opts} -DCMAKE_BUILD_TYPE:STRING=${adios2_build_mode}"
        cfg_opts="${cfg_opts} -DCMAKE_INSTALL_PREFIX:PATH=${adios2_install_path}"
        cfg_opts="${cfg_opts} -DCMAKE_C_FLAGS:STRING=\"${C_OPT_FLAGS}\""
        cfg_opts="${cfg_opts} -DCMAKE_CXX_FLAGS:STRING=\"${CXX_OPT_FLAGS}\""
        cfg_opts="${cfg_opts} -DADIOS2_USE_SST:BOOL=ON"

        if [[ "$bt" == "ser" ]]; then
            cfg_opts="${cfg_opts} -DADIOS2_USE_MPI:BOOL=OFF"
            cfg_opts="${cfg_opts} -DCMAKE_C_COMPILER:STRING=${C_COMPILER}"
            cfg_opts="${cfg_opts} -DCMAKE_CXX_COMPILER:STRING=${CXX_COMPILER}"
        elif [[ "$bt" == "par" ]]; then
            cfg_opts="${cfg_opts} -DADIOS2_USE_MPI:BOOL=ON"
            cfg_opts="${cfg_opts} -DCMAKE_C_COMPILER:STRING=${PAR_COMPILER}"
            cfg_opts="${cfg_opts} -DCMAKE_CXX_COMPILER:STRING=${PAR_COMPILER_CXX}"
        fi

        # call configure.
        CMAKE_BIN="${CMAKE_INSTALL}/cmake"
        if test -e bv_run_cmake.sh ; then
            rm -f bv_run_cmake.sh
        fi

        echo "\"${CMAKE_BIN}\"" ${cfg_opts} ../ > bv_run_cmake.sh
        cat bv_run_cmake.sh
        issue_command bash bv_run_cmake.sh

        if [[ $? != 0 ]] ; then
            warn "ADIOS2 configure failed.  Giving up"
            return 1
        fi

        #
        # Build ADIOS2
        #
        info "Building ADIOS2-$bt . . . (~5 minutes)"
        $MAKE $MAKE_OPT_FLAGS
        if [[ $? != 0 ]] ; then
            warn "ADIOS2 build failed.  Giving up"
            return 1
        fi

        #
        # Install into the VisIt third party location.
        #
        info "Installing ADIOS2-$bt"
        $MAKE install
        if [[ $? != 0 ]] ; then
            warn "ADIOS2 install failed.  Giving up"
            return 1
        fi

        if [[ "$DO_GROUP" == "yes" ]] ; then
            chmod -R ug+w,a+rX "$VISITDIR/adios2"
            chgrp -R ${GROUP} "$VISITDIR/adios2"
        fi

        cd "$START_DIR"
    done

    cd "$START_DIR"
    info "Done with ADIOS2"
    return 0
}

function bv_adios2_is_enabled
{
    if [[ $DO_ADIOS2 == "yes" ]]; then
        return 1
    fi
    return 0
}

function bv_adios2_is_installed
{
    if [[ "$USE_SYSTEM_ADIOS2" == "yes" ]]; then
        return 1
    fi

    check_if_installed "adios2" $ADIOS2_VERSION
    if [[ $? == 0 ]] ; then
        return 1
    fi
    return 0
}

function bv_adios2_build
{
    cd "$START_DIR"

    if [[ "$DO_ADIOS2" == "yes" && "$USE_SYSTEM_ADIOS2" == "no" ]] ; then
        ser_installed="no"
        par_installed="no"
        check_if_installed "adios2-ser" $ADIOS2_VERSION
        if [[ $? == 0 ]] ; then ser_installed="yes"; fi
        if [[ "$parallel" == "yes" ]]; then
            check_if_installed "adios2-par" $ADIOS2_VERSION
            if [[ $? == 0 ]] ; then par_installed="yes"; fi
        fi

        if [ "$ser_installed" == "yes" ] && ([ "$parallel" == "no" ] || [ "$par_installed" == "yes" ]) ; then
            info "ADIOS2 already installed, skipping"
        else
            build_adios2
            if [[ $? != 0 ]] ; then
                error "Unable to build or install ADIOS2.  Bailing out."
            fi
            info "Done building ADIOS2"
        fi
    fi
}
