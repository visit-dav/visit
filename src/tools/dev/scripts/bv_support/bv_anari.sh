# Module automatically read in from construct_build_visit

#initialize all the variables
function bv_anari_initialize
{
    export DO_ANARI="no"
    export DO_ANARI_NVTX="no"
    export USE_ALT_ANARI="no"
    add_extra_commandline_args "anari" "enable-vtk-nvtx" 0 "Enable NVTX instrumentation for VTK"
    add_extra_commandline_args "anari" "alt-anari-dir" 1 "Use alternate directory for ANARI"
}

#enable the module for install
function bv_anari_enable
{
    DO_ANARI="yes"
}

#disable the module for install
function bv_anari_disable
{
    DO_ANARI="no"
}

#enable NVTX instrumentation for VTK
function bv_anari_enable_vtk_nvtx
{
    bv_anari_enable
    DO_ANARI_NVTX="yes"
    info "Enabling NVTX instrumentation for VTK"
}

#use alternate ANARI dir
function bv_anari_alt_anari_dir
{
    bv_anari_enable
    USE_ALT_ANARI="yes"
    ALT_ANARI_DIR="$1"
    info "Using Alternate ANARI Directory: $ALT_ANARI_DIR"
}

#add any dependency with comma separation, both dependers and dependees
function bv_anari_depends_on
{
    depends_on="cmake"
    echo "${depends_on}"
}

#add information about how to get library..
function bv_anari_info
{
    export ANARI_VERSION=${ANARI_VERSION:-"0.9.1"}
    export ANARI_SHORT_VERSION=${ANARI_SHORT_VERSION:-"0.9"}
    export ANARI_FILE=${ANARI_FILE:-"ANARI-SDK-${ANARI_VERSION}.tar.gz"}
    export ANARI_COMPATIBILITY_VERSION=${ANARI_SHORT_VERSION}
    export ANARI_URL=${ANARI_URL:-"https://github.com/KhronosGroup/ANARI-SDK/archive/refs/tags/v0.9.1.tar.gz"}
    export ANARI_SRC_DIR=${ANARI_SRC_DIR:-"ANARI-SDK-${ANARI_VERSION}"}
    export ANARI_INSTALL_DIR=${ANARI_INSTALL_DIR:-"anari"}
    export ANARI_MD5_CHECKSUM="cf60fcbf647214f40c9d4664792aba2a"
}

#print variables used by this module
function bv_anari_print
{
    printf "%s%s\n" "ANARI_FILE=" "${ANARI_FILE}"
    printf "%s%s\n" "ANARI_VERSION=" "${ANARI_VERSION}"
    printf "%s%s\n" "ANARI_SRC_DIR=" "${ANARI_SRC_DIR}"
}

#print how to install and uninstall module..
function bv_anari_print_usage
{
    printf "%-20s %s [%s]\n" "--anari"   "Build ANARI" "$DO_ANARI"
    printf "%-20s %s [%s]\n" "--alt-anari-dir"   "Use ANARI from an alternative directory" "$USE_ALT_ANARI"
}

#values to add to host profile, write to $HOSTCONF
function bv_anari_host_profile
{
    #Add code to write values to variable $HOSTCONF
    if [[ "$DO_ANARI" == "yes" ]] ; then
        echo >> $HOSTCONF
        echo "##" >> $HOSTCONF
        echo "## ANARI" >> $HOSTCONF
        echo "##" >> $HOSTCONF

	echo "SETUP_APP_VERSION(ANARI $ANARI_VERSION)" >> $HOSTCONF
        echo "VISIT_OPTION_DEFAULT(VISIT_ANARI ON TYPE BOOL)" >> $HOSTCONF

        if [[ "$USE_ALT_ANARI" == "yes" ]] ; then
	    echo "VISIT_OPTION_DEFAULT(VISIT_ANARI_DIR $ALT_ANARI_DIR)" >> $HOSTCONF
	else
	    echo "VISIT_OPTION_DEFAULT(VISIT_ANARI_DIR \${VISITHOME}/anari/\${ANARI_VERSION}/\${VISITARCH})" >> $HOSTCONF
        fi
    fi
}

#prepare the module and check whether it is built or is ready to be built.
function bv_anari_ensure
{
    if [[ "$DO_ANARI" == "yes" && "$USE_ALT_ANARI" == "no" ]] ; then
        ensure_built_or_ready $ANARI_INSTALL_DIR $ANARI_VERSION $ANARI_SRC_DIR $ANARI_FILE $ANARI_URL

        if [[ $? != 0 ]] ; then
            ANY_ERRORS="yes"
            bv_anari_disable
            error "Unable to build ANARI. ${ANARI_FILE} not found."
        fi
    fi
}

function build_anari
{
    if [[ -d $ANARI_SRC_DIR ]] ; then
        if [[ ! -f $ANARI_FILE ]] ; then
            warn "The directory ${ANARI_SRC_DIR} exists, deleting before downloading and uncompressing"
            rm -Rf $ANARI_SRC_DIR
            bv_anari_ensure

            if [[ "$DO_ANARI" == "no" ]] ; then
                return 1
            fi
        fi
    fi

    # Extract sources
    prepare_build_dir $ANARI_SRC_DIR $ANARI_FILE
    untarred_anari=$?
    # -1 on failure, 0 for success without untar
    #  1 for success with untar, 2 for failure with checksum

    if [[ $untarred_anari == -1 ]] ; then
        warn "Unable to prepare ANARI source directory. Giving Up!"
        return 1
    fi

    # Make build directory for an out-of-source build.
    ANARI_BUILD_DIR="${ANARI_SRC_DIR}-build"

    if [[ ! -d $ANARI_BUILD_DIR ]] ; then
        echo "Making build directory $ANARI_BUILD_DIR"
        mkdir $ANARI_BUILD_DIR
    fi

    # CMake config options
    anari_build_mode="${VISIT_BUILD_MODE}"
    anari_inst_path="${VISITDIR}/${ANARI_INSTALL_DIR}/${ANARI_VERSION}/${VISITARCH}"

    vopts="-DCMAKE_BUILD_TYPE:STRING=${anari_build_mode}"
    vopts="${vopts} -DCMAKE_INSTALL_PREFIX:PATH=${anari_inst_path}"
    vopts="${vopts} -DCMAKE_C_COMPILER:STRING=${C_COMPILER}"
    vopts="${vopts} -DCMAKE_CXX_COMPILER:STRING=${CXX_COMPILER}"
    vopts="${vopts} -DCMAKE_C_FLAGS:STRING=\"${C_OPT_FLAGS}\""
    vopts="${vopts} -DCMAKE_CXX_FLAGS:STRING=\"${CXX_OPT_FLAGS}\""
    vopts="${vopts} -DCMAKE_EXE_LINKER_FLAGS:STRING=${lf}"
    vopts="${vopts} -DCMAKE_MODULE_LINKER_FLAGS:STRING=${lf}"
    vopts="${vopts} -DCMAKE_SHARED_LINKER_FLAGS:STRING=${lf}"

    if test "${OPSYS}" = "Darwin" ; then
	    vopts="${vopts} -DCMAKE_INSTALL_NAME_DIR:PATH=${anari_inst_path}/lib"
    fi

    # ANARI config options
    if test "x${DO_STATIC_BUILD}" = "xyes" ; then
	    vopts="${vopts} -DBUILD_SHARED_LIBS:BOOL=OFF"
    else
	    vopts="${vopts} -DBUILD_SHARED_LIBS:BOOL=ON"
    fi

    vopts="${vopts} -DBUILD_HELIDE_DEVICE:BOOL=ON"
    vopts="${vopts} -DBUILD_REMOTE_DEVICE:BOOL=OFF"
    vopts="${vopts} -DINSTALL_CODE_GEN_SCRIPTS:BOOL=ON"
    vopts="${vopts} -DBUILD_CTS:BOOL=OFF"
    vopts="${vopts} -DBUILD_EXAMPLES:BOOL=ON"
    vopts="${vopts} -DBUILD_TESTING:BOOL=OFF"
    vopts="${vopts} -DBUILD_VIEWER:BOOL=OFF"

    #
    # Configure and Build the ANARI SDK
    #
    cd ${ANARI_BUILD_DIR}
    rm -rf *

    #CMAKE_BIN="${CMAKE_INSTALL}/cmake"
    CMAKE_BIN=cmake

    #
    # Several platforms have had problems with the VTK cmake configure command
    # issued simply via "issue_command".  This was first discovered on
    # BGQ and then showed up in random cases for both OSX and Linux machines.
    # Brad resolved this on BGQ  with a simple work around - we write a simple
    # script that we invoke with bash which calls cmake with all of the properly
    # arguments. We are now using this strategy for all platforms.
    #
    echo "\"${CMAKE_BIN}\"" ${vopts} ../${ANARI_SRC_DIR} > bv_run_cmake.sh
    cat bv_run_cmake.sh
    issue_command bash bv_run_cmake.sh || error "ANARI configuration failed."

    #
    # Now build ANARI
    #
    info "Building ANARI . . ."
    env DYLD_LIBRARY_PATH=`pwd`/bin $MAKE $MAKE_OPT_FLAGS || \
        error "ANARI did not build correctly.  Giving up."

    info "Installing ANARI . . . "
    $MAKE install || error "ANARI did not install correctly."

    chmod -R ug+w,a+rX ${VISITDIR}/${ANARI_INSTALL_DIR}

    if [[ "$DO_GROUP" == "yes" ]] ; then
        chgrp -R ${GROUP} "$VISITDIR/${ANARI_INSTALL_DIR}"
    fi

    cd "$START_DIR"
    return 0
}

function bv_anari_is_enabled
{
    if [[ $DO_ANARI == "yes" ]]; then
        return 1
    fi
    return 0
}

function bv_anari_is_installed
{
    if [[ "USE_ALT_ANARI" == "yes" ]]; then
	    return 1
    fi

    check_if_installed "anari" $ANARI_VERSION
    if [[ $? == 0 ]] ; then
        return 1
    fi
    return 0
}

#the build command..
function bv_anari_build
{
    cd "$START_DIR"

    if [[ "$DO_ANARI" == "yes" && "$USE_ALT_ANARI" == "no" ]] ; then
        check_if_installed $ANARI_INSTALL_DIR $ANARI_VERSION

        if [[ $? == 0 ]] ; then
            info "Skipping ANARI build.  ANARI is already installed."
        else
            #Build the Module
            build_anari

            if [[ $? != 0 ]] ; then
                error "Unable to build or install ANARI.  Bailing out."
            fi

            info "Done building ANARI"
       	fi
    fi
}
