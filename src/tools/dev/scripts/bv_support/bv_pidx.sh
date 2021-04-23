function bv_pidx_initialize
{
    export DO_PIDX="no"
    export USE_SYSTEM_PIDX="no"
    add_extra_commandline_args "pidx" "alt-pidx-dir" 1 "Use alternative directory for pidx"
}

function bv_pidx_enable
{
    DO_PIDX="yes"
}

function bv_pidx_disable
{
    DO_PIDX="no"
}

function bv_pidx_alt_pidx_dir
{
    bv_pidx_enable
    USE_SYSTEM_PIDX="yes"
    PIDX_INSTALL_DIR="$1"
}

function bv_pidx_depends_on
{
    if [[ "$USE_SYSTEM_PIDX" == "yes" ]]; then
        echo ""
    else
        depends_on="cmake"

        if [[ "$DO_MPICH" == "yes" ]] ; then
            depends_on="$depends_on mpich"
        fi

        echo $depends_on
    fi
}

function bv_pidx_initialize_vars
{
    if [[ "$USE_SYSTEM_PIDX" == "no" ]]; then
        PIDX_INSTALL_DIR="${VISITDIR}/pidx/$PIDX_VERSION/${VISITARCH}"
    fi
}

function bv_pidx_info
{
    export PIDX_VERSION=${PIDX_VERSION:-"0.9.3"}
    export PIDX_FILE=${PIDX_FILE:-"PIDX-${PIDX_VERSION}.tar.gz"}
    export PIDX_COMPATIBILITY_VERSION=${PIDX_COMPATIBILITY_VERSION:-"1.8"}
    export PIDX_SRC_DIR=${PIDX_SRC_DIR:-"PIDX-${PIDX_VERSION}"}
    export PIDX_BUILD_DIR=${PIDX_BUILD_DIR:-"${PIDX_SRC_DIR}-build"}
    export PIDX_URL=${PIDX_URL:-"https://github.com/sci-visus/PIDX/releases/download/v${PIDX_VERSION}"}
    export PIDX_MD5_CHECKSUM="bddd00f980e8e8e2ee701b4d816aa6dd"
    export PIDX_SHA256_CHECKSUM="e6c91546821134f87b80ab1d3ed6aa0930c4507d84ad1f19ec51a7ae10152888"
}

function bv_pidx_print
{
    printf "%s%s\n" "PIDX_FILE=" "${PIDX_FILE}"
    printf "%s%s\n" "PIDX_VERSION=" "${PIDX_VERSION}"
    printf "%s%s\n" "PIDX_COMPATIBILITY_VERSION=" "${PIDX_COMPATIBILITY_VERSION}"
    printf "%s%s\n" "PIDX_SRC_DIR=" "${PIDX_SRC_DIR}"
    printf "%s%s\n" "PIDX_BUILD_DIR=" "${PIDX_BUILD_DIR}"
}

function bv_pidx_print_usage
{
    printf "%-20s %s [%s]\n" "--pidx" "Build pidx" "${DO_PIDX}"
    printf "%-20s %s [%s]\n" "--alt-pidx-dir" "Use pidx from an alternative directory"
}

function bv_pidx_host_profile
{
    if [[ "$DO_PIDX" == "yes" ]] ; then
        echo >> $HOSTCONF
        echo "##" >> $HOSTCONF
        echo "## PIDX" >> $HOSTCONF
        echo "##" >> $HOSTCONF

        echo "SETUP_APP_VERSION(PIDX $PIDX_VERSION)" >> $HOSTCONF

        if [[ "$USE_SYSTEM_PIDX" == "yes" ]]; then
            echo \
                "VISIT_OPTION_DEFAULT(VISIT_PIDX_DIR $PIDX_INSTALL_DIR)" \
                >> $HOSTCONF
        else
            echo \
                "VISIT_OPTION_DEFAULT(VISIT_PIDX_DIR \${VISITHOME}/pidx/\${PIDX_VERSION}/\${VISITARCH})" \
                >> $HOSTCONF
        fi
    fi
}

function bv_pidx_ensure
{
    if [[ "$DO_PIDX" == "yes" && "$USE_SYSTEM_PIDX" == "no" ]] ; then
        check_installed_or_have_src "pidx" $PIDX_VERSION $PIDX_BUILD_DIR $PIDX_FILE $PIDX_URL
        if [[ $? != 0 ]] ; then
            ANY_ERRORS="yes"
            DO_PIDX="no"
            error "Unable to build pidx. ${PIDX_FILE} not found."
        fi
    fi
}

function bv_pidx_dry_run
{
    if [[ "$DO_PIDX" == "yes" ]] ; then
        echo "Dry run option not set for pidx."
    fi
}

function apply_pidx_patch
{
    cd ${PIDX_SRC_DIR} || error "Can't cd to PIDX source dir."

    #    if [[ "${PIDX_VERSION}" == 4.0.0 ]] ; then
    #        apply_pidx_XXX_patch
    #        if [[ $? != 0 ]]; then
    #           return 1
    #        fi
    #    fi

    cd "$START_DIR"

    return 0
}

# *************************************************************************** #
#              Function 8.1, build_pidx                                   #
# *************************************************************************** #
function build_pidx
{
    #
    # Uncompress the source file
    #
    uncompress_src_file $PIDX_SRC_DIR $PIDX_FILE
    untarred_pidx=$?
    if [[ $untarred_pidx == -1 ]] ; then
        warn "Unable to uncompress pidx source file. Giving Up"
        return 1
    fi

    #
    # Apply patches
    #
    info "Patching pidx . . ."
    apply_pidx_patch
    if [[ $? != 0 ]] ; then
        if [[ $untarred_pidx == 1 ]] ; then
            warn "Giving up on pidx build because the patch failed."
            return 1
        else
            warn "Patch failed, but continuing. I believe that this script\n" \
                 "tried to apply a patch to an existing directory that had\n" \
                 "already been patched ... that is, the patch is\n" \
                 "failing harmlessly on a second application."
        fi
    fi

    # Make a build directory for an out-of-source build.
    cd "$START_DIR"
    if [[ ! -d $PIDX_BUILD_DIR ]] ; then
        echo "Making build directory $PIDX_BUILD_DIR"
        mkdir $PIDX_BUILD_DIR
    else
        #
        # Remove the CMakeCache.txt files ... existing files sometimes
        # prevent fields from getting overwritten properly.
        #
        rm -Rf ${PIDX_BUILD_DIR}/CMakeCache.txt ${PIDX_BUILD_DIR}/*/CMakeCache.txt
    fi
    cd ${PIDX_BUILD_DIR}

    #
    # Configure pidx
    #
    info "Configuring pidx . . ."

    ntopts=""
    pidx_build_mode="${VISIT_BUILD_MODE}"
    pidx_inst_path="${PIDX_INSTALL_DIR}"

    ntopts="${ntopts} -DCMAKE_BUILD_TYPE:STRING=${pidx_build_mode}"
    ntopts="${ntopts} -DCMAKE_INSTALL_PREFIX:PATH=${pidx_inst_path}"

    # Currently does not work but should be used.
#    ntopts="${ntopts} -DBUILD_SHARED_LIBS:BOOL=ON"

    # Because above the build type is specificed the compiler flags are set
    # So do not set any of these four flags. Otherse a semi-colon gets
    # inserted into the the makefile comands.
    ntopts="${ntopts} -DCMAKE_C_COMPILER:STRING=${C_COMPILER}"
    ntopts="${ntopts} -DCMAKE_CXX_COMPILER:STRING=${CXX_COMPILER}"
    ntopts="${ntopts} -DCMAKE_C_FLAGS:STRING=\"\""
    ntopts="${ntopts} -DCMAKE_CXX_FLAGS:STRING=\"\""

#    ntopts="${ntopts} -DCMAKE_C_FLAGS:STRING=\"${C_OPT_FLAGS}\""
#    ntopts="${ntopts} -DCMAKE_CXX_FLAGS:STRING=\"${CXX_OPT_FLAGS}\""

#    ntopts="${ntopts} -DCMAKE_EXE_LINKER_FLAGS:STRING=${lf}"
#    ntopts="${ntopts} -DCMAKE_MODULE_LINKER_FLAGS:STRING=${lf}"
#    ntopts="${ntopts} -DCMAKE_SHARED_LINKER_FLAGS:STRING=${lf}"

    # pidx specific options.


#    if test "${OPSYS}" = "Darwin" ; then
#        ntopts="${ntopts} -DCMAKE_INSTALL_NAME_DIR:PATH=${pidx_inst_path}/lib"
#    fi

    if [[ "${DO_MPICH}" == "yes" ]]; then
        info "mpich requested. Configuring PIDX with mpich support."
        ntopts="${ntopts} -DMPI_C_COMPILER:PATH=${VISITDIR}/mpich/${MPICH_VERSION}/${VISITARCH}/bin/mpicc"
        ntopts="${ntopts} -DMPI_CXX_COMPILER:PATH=${VISITDIR}/mpich/${MPICH_VERSION}/${VISITARCH}/bin/mpicxx"

#        if [[ "$OPSYS" == "Darwin" ]]; then
#            export DYLD_LIBRARY_PATH="$VISITDIR/mpich/$MPICH_VERSION/$VISITARCH/lib":$DYLD_LIBRARY_PATH
#        else
#            export LD_LIBRARY_PATH="$VISITDIR/mpich/$MPICH_VERSION/$VISITARCH/lib":$LD_LIBRARY_PATH
#        fi
    elif [[ "$parallel" == "yes" ]]; then
        if [[ "$PAR_COMPILER" != "" ]]; then
            ntopts="${ntopts} -DMPI_C_COMPILER:STRING=${PAR_COMPILER}"
        fi
        if [[ "$PAR_COMPILER_CXX" != "" ]]; then
            ntopts="${ntopts} -DMPI_CXX_COMPILER:STRING=${PAR_COMPILER_CXX}"
        fi
        if [[ "$PAR_INCLUDE" != "" ]] ; then
            ntopts="${ntopts} -DMPI_C_INCLUDE_PATH:STRING=${PAR_INCLUDE_PATH}"
            ntopts="${ntopts} -DMPI_CXX_INCLUDE_PATH:STRING=${PAR_INCLUDE_PATH}"
        fi
        if [[ "$PAR_LIBS" != "" ]] ; then
            ntopts="${ntopts} -DMPI_C_LINK_FLAGS:STRING=${PAR_LINKER_FLAGS}"
            ntopts="${ntopts} -DMPI_C_LIBRARIES:STRING=${PAR_LIBRARY_LINKER_FLAGS}"
            ntopts="${ntopts} -DMPI_CXX_LINK_FLAGS:STRING=${PAR_LINKER_FLAGS}"
            ntopts="${ntopts} -DMPI_CXX_LIBRARIES:STRING=${PAR_LIBRARY_LINKER_FLAGS}"
        fi
    fi

    #
    # Several platforms have had problems with the VTK cmake configure command
    # issued simply via "issue_command". This was first discovered on
    # BGQ and then showed up in random cases for both OSX and Linux machines.
    # Brad resolved this on BGQ  with a simple work around - we write a simple
    # script that we invoke with bash which calls cmake with all of the properly
    # arguments. We are now using this strategy for all platforms.
    #
    CMAKE_BIN="${CMAKE_INSTALL}/cmake"

    if test -e bv_run_cmake.sh ; then
        rm -f bv_run_cmake.sh
    fi

    echo "\"${CMAKE_BIN}\"" ${ntopts} ../${PIDX_SRC_DIR} > bv_run_cmake.sh
    cat bv_run_cmake.sh
    issue_command bash bv_run_cmake.sh

    if [[ $? != 0 ]] ; then
        warn "PIDX configure failed. Giving up"
        return 1
    fi

    #
    # Build PIDX
    #
    info "Making pidx . . ."
    $MAKE $MAKE_OPT_FLAGS
    if [[ $? != 0 ]] ; then
        warn "pidx build failed. Giving up"
        return 1
    fi

    #
    # Install into the VisIt third party location.
    #
    info "Installing pidx . . ."
    $MAKE install
    if [[ $? != 0 ]] ; then
        warn "pidx install failed. Giving up"
        return 1
    fi

#    mv ${pidx_inst_path}/lib64/* ${pidx_inst_path}/lib

    if [[ "$DO_GROUP" == "yes" ]] ; then
        chmod -R ug+w,a+rX "$VISITDIR/pidx"
        chgrp -R ${GROUP} "$VISITDIR/pidx"
    fi
    cd "$START_DIR"
    info "Done with pidx"
    return 0
}

function bv_pidx_is_enabled
{
    if [[ $DO_PIDX == "yes" ]]; then
        return 1
    fi
    return 0
}

function bv_pidx_is_installed
{
    if [[ "$USE_SYSTEM_PIDX" == "yes" ]]; then
        return 1
    fi

    check_if_installed "pidx" $PIDX_VERSION
    if [[ $? == 0 ]] ; then
        return 1
    fi
    return 0
}

function bv_pidx_build
{
    cd "$START_DIR"

    if [[ "$DO_PIDX" == "yes" && "$USE_SYSTEM_PIDX" == "no" ]] ; then
        check_if_installed "pidx" $PIDX_VERSION
        if [[ $? == 0 ]] ; then
            info "Skipping PIDX build. PIDX is already installed."
        else
            info "Building PIDX (~2 minutes)"
            build_pidx
            if [[ $? != 0 ]] ; then
                error "Unable to build or install PIDX. Bailing out."
            fi
            info "Done building pidx"
        fi
    fi
}
