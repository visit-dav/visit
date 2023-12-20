#initialize all the variables
function bv_openpmd_api_initialize
{
    export DO_OPENPMD_API="no"
}

#enable the module for install
function bv_openpmd_api_enable
{ 
    DO_OPENPMD_API="yes"
}

#disable the module for install
function bv_openpmd_api_disable
{
    DO_OPENPMD_API="no"
}

#add any dependency with comma separation, both dependers and dependees
function bv_openpmd_api_depends_on
{
    depends_on="cmake"

    if [[ "$DO_HDF5" == "yes" ]] ; then
        depends_on="$depends_on hdf5"
    fi

    if [[ "$DO_ADIOS2" == "yes" ]] ; then
        depends_on="$depends_on adios2"
    fi

    if [[ "$DO_MPICH" == "yes" ]] ; then
        depends_on="$depends_on mpich"
    fi

    echo $depends_on
}

#add information about how to get library..
function bv_openpmd_api_info
{
    export OPENPMD_API_VERSION=${OPENPMD_API_VERSION:-"0.15.2"}
    export OPENPMD_API_FILE=${OPENPMD_API_FILE:-"openpmd_api-${OPENPMD_API_VERSION}.tar.gz"}
    export OPENPMD_API_URL=${OPENPMD_API_URL:-"https://github.com/openPMD/openPMD-api/archive/refs/tags/${OPENPMD_API_VERSION}"}
    export OPENPMD_API_COMPATIBILITY_VERSION=${OPENPMD_API_COMPATIBILITY_VERSION:-"${OPENPMD_API_VERSION}"}
    export OPENPMD_API_BUILD_DIR=${OPENPMD_API_BUILD_DIR:-"openPMD-api-build"}
    export OPENPMD_API_MD5_CHECKSUM="3314027b23db98f57684d334af8bc6d3"
    export OPENPMD_API_SHA256_CHECKSUM="fbe3b356fe6f4589c659027c8056844692c62382e3ec53b953bed1c87e58ba13"
}

#print variables used by this module
function bv_openpmd_api_print
{
    printf "%s%s\n" "OPENPMD_API_FILE=" "${OPENPMD_API_FILE}"
    printf "%s%s\n" "OPENPMD_API_VERSION=" "${OPENPMD_API_VERSION}"
    printf "%s%s\n" "OPENPMD_API_COMPATIBILITY_VERSION=" "${OPENPMD_API_COMPATIBILITY_VERSION}"
    printf "%s%s\n" "OPENPMD_API_BUILD_DIR=" "${OPENPMD_API_BUILD_DIR}"
}

#print how to install and uninstall module..
function bv_openpmd_api_print_usage
{
    printf "%-20s %s [%s]\n" "--openpmd_api"   "Build OPENPMD_API" "$DO_OPENPMD_API"
    printf "%-20s %s [%s]\n" "--no-openpmd_api"   "Do not Build OPENPMD_API" "$DO_OPENPMD_API"
}

#values to add to host profile, write to $HOSTCONF
function bv_openpmd_api_host_profile
{
    #Add code to write values to variable $HOSTCONF
    if [[ "$DO_OPENPMD_API" == "yes" ]] ; then
        echo >> $HOSTCONF
        echo "##" >> $HOSTCONF
        echo "## OPENPMD_API" >> $HOSTCONF
        echo "##" >> $HOSTCONF
        echo "VISIT_OPTION_DEFAULT(VISIT_OPENPMD_API_DIR \${VISITHOME}/openpmd_api/$OPENPMD_API_VERSION/\${VISITARCH})" \
             >> $HOSTCONF
    fi
}

#prepare the module and check whether it is built or is ready to be built.
function bv_openpmd_api_ensure
{
    if [[ "$DO_OPENPMD_API" == "yes" ]] ; then
        ensure_built_or_ready "OPENPMD_API" $OPENPMD_API_VERSION $OPENPMD_API_BUILD_DIR $OPENPMD_API_FILE
        if [[ $? != 0 ]] ; then
            ANY_ERRORS="yes"
            DO_OPENPMD_API="no"
            error "Unable to build OPENPMD_API.  ${OPENPMD_API_FILE} not found."
        fi
    fi
}

function build_openpmd_api
{
    #
    # openpmd_api uses CMake  -- make sure we have it built.
    #
    CMAKE_INSTALL=${CMAKE_INSTALL:-"$VISITDIR/cmake/${CMAKE_VERSION}/$VISITARCH/bin"}
    if [[ -e ${CMAKE_INSTALL}/cmake ]] ; then
        info "openPMD-api: CMake found"
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
    prepare_build_dir $OPENPMD_API_BUILD_DIR $OPENPMD_API_FILE
    untarred_openpmd_api=$?
    if [[ $untarred_openpmd_api == -1 ]] ; then
        warn "Unable to prepare openPMD-api build directory. Giving Up!"
        return 1
    fi

    cd $OPENPMD_API_BUILD_DIR || error "Can't cd to openPMD-api build dir."

    openpmd_api_install_path="${VISITDIR}/openpmd_api/${OPENPMD_API_VERSION}/${VISITARCH}"

    cfg_opts=""
    cfg_opts="${cfg_opts} -DCMAKE_INSTALL_PREFIX:PATH=${openpmd_api_install_path}"
    if [[ "$DO_MPICH" == "yes" ]] ; then
        cfg_opts="${cfg_opts} -DopenPMD_USE_MPI=ON"
    else
        cfg_opts="${cfg_opts} -DopenPMD_USE_MPI=OFF"
    fi
    if [[ "$DO_HDF5" == "yes" ]] ; then
        cfg_opts="${cfg_opts} -DopenPMD_USE_HDF5=ON"
    else
        cfg_opts="${cfg_opts} -DopenPMD_USE_HDF5=OFF"
    fi
    if [[ "$DO_ADIOS2" == "yes" ]] ; then
        cfg_opts="${cfg_opts} -DopenPMD_USE_ADIOS2=ON"
    else
        cfg_opts="${cfg_opts} -DopenPMD_USE_ADIOS2=OFF"
    fi
    cfg_opts="${cfg_opts} -DopenPMD_INSTALL=ON"

    CMAKE_BIN="${CMAKE_INSTALL}/cmake"
    if test -e bv_run_cmake.sh ; then
        rm -f bv_run_cmake.sh
    fi

    echo "\"${CMAKE_BIN}\"" ${cfg_opts} ../openPMD-api > bv_run_cmake.sh
    cat bv_run_cmake.sh
    issue_command bash bv_run_cmake.sh

    if [[ $? != 0 ]] ; then
        warn "openPMD-api configure failed.  Giving up"
        return 1
    fi

    # 
    # Build openPMD-api
    # 
    info "Building openPMD-api . . . (~5 minutes)"
    if test -e bv_run_cmake.sh ; then
        rm -f bv_run_cmake.sh
    fi

    echo "\"${CMAKE_BIN}\"" --build . > bv_run_cmake.sh
    cat bv_run_cmake.sh
    issue_command bash bv_run_cmake.sh

    #
    # Install into the VisIt third party location.
    #
    info "Installing openPMD-api"
    if test -e bv_run_cmake.sh ; then
        rm -f bv_run_cmake.sh
    fi

    echo "\"${CMAKE_BIN}\"" --build . --target install > bv_run_cmake.sh
    cat bv_run_cmake.sh
    issue_command bash bv_run_cmake.sh

    if [[ "$DO_GROUP" == "yes" ]] ; then
        chmod -R ug+w,a+rX "$VISITDIR/openpmd_api"
        chgrp -R ${GROUP} "$VISITDIR/openpmd_api"
    fi

    cd "$START_DIR"
    info "Done with openPMD-api"
    return 0
}

function bv_openpmd_api_is_enabled
{
    if [[ $DO_OPENPMD_API == "yes" ]]; then
        return 1
    fi
    return 0
}

function bv_openpmd_api_is_installed
{
    check_if_installed "openpmd_api" $OPENPMD_API_VERSION
    if [[ $? == 0 ]] ; then
        return 1
    fi
    return 0
}

#the build command..
function bv_openpmd_api_build
{
    cd "$START_DIR"
    if [[ "$DO_OPENPMD_API" == "yes" ]] ; then
        check_if_installed "openpmd_api" $OPENPMD_API_VERSION
        if [[ $? == 0 ]] ; then
            info "Skipping OPENPMD_API build.  OPENPMD_API is already installed."
        else
            info "Building OPENPMD_API (~5 minutes)"

            #Build the Module 
            build_openpmd_api
            if [[ $? != 0 ]] ; then
                error "Unable to build or install OPENPMD_API.  Bailing out."
            fi
            info "Done building OPENPMD_API"
       fi
    fi
}
