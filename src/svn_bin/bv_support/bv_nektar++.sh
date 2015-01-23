function bv_nektar++_initialize
{
export DO_NEKTAR_PLUS_PLUS="no"
export ON_NEKTAR_PLUS_PLUS="off"
export USE_SYSTEM_NEKTAR_PLUS_PLUS="no"
add_extra_commandline_args "nektar++" "alt-nektar++-dir" 1 "Use alternative directory for nektar++"
}

function bv_nektar++_enable
{
DO_NEKTAR_PLUS_PLUS="yes"
ON_NEKTAR_PLUS_PLUS="on"
}

function bv_nektar++_disable
{
DO_NEKTAR_PLUS_PLUS="no"
ON_NEKTAR_PLUS_PLUS="off"
}

function bv_nektar++_alt_nektar++_dir
{
    bv_nektar++_enable
    USE_SYSTEM_NEKTAR_PLUS_PLUS="yes"
    NEKTAR_PLUS_PLUS_INSTALL_DIR="$1"
}

function bv_nektar++_depends_on
{
    depends_on="cmake boost"

    if [[ "$USE_SYSTEM_NEKTAR_PLUS_PLUS" == "yes" ]]; then
        echo ""
    else
        if [[ "$DO_ZLIB" == "yes" ]] ; then
           depends_on="$depends_on zlib"    
        fi

        echo $depends_on
    fi
}

function bv_nektar++_initialize_vars
{
    if [[ "$USE_SYSTEM_NEKTAR_PLUS_PLUS" == "no" ]]; then
        NEKTAR_PLUS_PLUS_INSTALL_DIR="${VISITDIR}/nektar++/$NEKTAR_PLUS_PLUS_VERSION/${VISITARCH}"
    fi
}

function bv_nektar++_info
{
export NEKTAR_PLUS_PLUS_VERSION=${NEKTAR_PLUS_PLUS_VERSION:-"4.0.1"}
export NEKTAR_PLUS_PLUS_FILE=${NEKTAR_PLUS_PLUS_FILE:-"nektar++-${NEKTAR_PLUS_PLUS_VERSION}.tar.gz"}
export NEKTAR_PLUS_PLUS_COMPATIBILITY_VERSION=${NEKTAR_PLUS_PLUS_COMPATIBILITY_VERSION:-"1.8"}
export NEKTAR_PLUS_PLUS_BUILD_DIR=${NEKTAR_PLUS_PLUS_BUILD_DIR:-"nektar++-${NEKTAR_PLUS_PLUS_VERSION}"}
export NEKTAR_PLUS_PLUS_URL=${NEKTAR_PLUS_PLUS_URL:-"http://www.nektar.info/downloads/nektar++-${NEKTAR_PLUS_PLUS_VERSION}/src"}
export NEKTAR_PLUS_PLUS_MD5_CHECKSUM=""
export NEKTAR_PLUS_PLUS_SHA256_CHECKSUM=""
}

function bv_nektar++_print
{
  printf "%s%s\n" "NEKTAR_PLUS_PLUS_FILE=" "${NEKTAR_PLUS_PLUS_FILE}"
  printf "%s%s\n" "NEKTAR_PLUS_PLUS_VERSION=" "${NEKTAR_PLUS_PLUS_VERSION}"
  printf "%s%s\n" "NEKTAR_PLUS_PLUS_COMPATIBILITY_VERSION=" "${NEKTAR_PLUS_PLUS_COMPATIBILITY_VERSION}"
  printf "%s%s\n" "NEKTAR_PLUS_PLUS_BUILD_DIR=" "${NEKTAR_PLUS_PLUS_BUILD_DIR}"
}

function bv_nektar++_print_usage
{
printf "%-15s %s [%s]\n" "--nektar++" "Build NEKTAR_PLUS_PLUS" "${DO_NEKTAR_PLUS_PLUS}"
}

function bv_nektar++_graphical
{
local graphical_out="NEKTAR_PLUS_PLUS     $NEKTAR_PLUS_PLUS_VERSION($NEKTAR_PLUS_PLUS_FILE)      $ON_NEKTAR_PLUS_PLUS"
echo $graphical_out
}

function bv_nektar++_host_profile
{
    if [[ "$DO_NEKTAR_PLUS_PLUS" == "yes" ]] ; then
        echo >> $HOSTCONF
        echo "##" >> $HOSTCONF
        echo "## Nektar++" >> $HOSTCONF
        echo "##" >> $HOSTCONF

        echo "SETUP_APP_VERSION(NEKTAR++ $NEKTAR_PLUS_PLUS_VERSION)" >> $HOSTCONF 

        if [[ "$USE_SYSTEM_NEKTAR_PLUS_PLUS" == "yes" ]]; then
            echo \
            "VISIT_OPTION_DEFAULT(VISIT_NEKTAR++_DIR $NEKTAR_PLUS_PLUS_INSTALL_DIR)" \
            >> $HOSTCONF 
        else
            echo \
            "VISIT_OPTION_DEFAULT(VISIT_NEKTAR++_DIR \${VISITHOME}/nektar++/\${NEKTAR++_VERSION}/\${VISITARCH})" \
            >> $HOSTCONF 

            if [[ "$DO_ZLIB" == "yes" ]] ; then
               ZLIB_LIBDEP="\${VISITHOME}/zlib/$ZLIB_VERSION/\${VISITARCH}/lib z"
            else
               ZLIB_LIBDEP="/usr/lib z"
               #moving global patch to have limited effect
               if [[ -d /usr/lib/x86_64-linux-gnu ]]; then
                ZLIB_LIBDEP="/usr/lib/x86_64-linux-gnu z"
               fi
            fi

            echo \
            "VISIT_OPTION_DEFAULT(VISIT_NEKTAR++_LIBDEP $ZLIB_LIBDEP TYPE STRING)" \
            >> $HOSTCONF
        fi
    fi
}

function bv_nektar++_ensure
{
    if [[ "$DO_NEKTAR_PLUS_PLUS" == "yes" && "$USE_SYSTEM_NEKTAR_PLUS_PLUS" == "no" ]] ; then
        ensure_built_or_ready "nektar++" $NEKTAR_PLUS_PLUS_VERSION $NEKTAR_PLUS_PLUS_BUILD_DIR $NEKTAR_PLUS_PLUS_FILE $NEKTAR_PLUS_PLUS_URL 
        if [[ $? != 0 ]] ; then
            ANY_ERRORS="yes"
            DO_NEKTAR_PLUS_PLUS="no"
            error "Unable to build Netar++.  ${NEKTAR_PLUS_PLUS_FILE} not found."
        fi
    fi
}

function bv_nektar++_dry_run
{
  if [[ "$DO_NEKTAR_PLUS_PLUS" == "yes" ]] ; then
    echo "Dry run option not set for nektar_PLUS_PLUS."
  fi
}

function apply_nektar++_patch
{
#    if [[ "${NEKTAR_PLUS_PLUS_VERSION}" == 4.0.0 ]] ; then
#        apply_nektar++_zlib_patch
#        if [[ $? != 0 ]]; then
#           return 1
#        fi
#    fi

    return 0
}

# *************************************************************************** #
#              Function 8.1, build_nektar++                                   #
# *************************************************************************** #
function build_nektar++
{
    #
    # CMake is the build system for VTK.  Call another script that will build
    # that program.
    #
    CMAKE_INSTALL=${CMAKE_INSTALL:-"$VISITDIR/cmake/${CMAKE_VERSION}/$VISITARCH/bin"}
    if [[ -e ${CMAKE_INSTALL}/cmake ]] ; then
        info "Nektar++: CMake found"
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
    prepare_build_dir $NEKTAR_PLUS_PLUS_BUILD_DIR $NEKTAR_PLUS_PLUS_FILE
    untarred_nektar_plus_plus=$?
    if [[ $untarred_nektar_plus_plus == -1 ]] ; then
       warn "Unable to prepare Nektar++ Build Directory. Giving Up"
       return 1
    fi

    #
    cd $NEKTAR_PLUS_PLUS_BUILD_DIR || error "Can't cd to Nektar++ build dir." $NEKTAR_PLUS_PLUS_BUILD_DIR 
    apply_nektar++_patch
    if [[ $? != 0 ]]; then
        warn "Patch failed, but continuing."
    fi

    ntopts=""
    nektar_plus_plus_build_mode="${VISIT_BUILD_MODE}"
    nektar_plus_plus_inst_path="${NEKTAR_PLUS_PLUS_INSTALL_DIR}"

    ntopts="${ntopts} -DCMAKE_BUILD_TYPE:STRING=${nektar_plus_plus_build_mode}"
    ntopts="${ntopts} -DCMAKE_INSTALL_PREFIX:PATH=${nektar_plus_plus_inst_path}"

    ntopts="${ntopts} -DCMAKE_C_COMPILER:STRING=${C_COMPILER}"
    ntopts="${ntopts} -DCMAKE_CXX_COMPILER:STRING=${CXX_COMPILER}"
    ntopts="${ntopts} -DCMAKE_C_FLAGS:STRING=\"${C_OPT_FLAGS}\""
    ntopts="${ntopts} -DCMAKE_CXX_FLAGS:STRING=\"${CXX_OPT_FLAGS}\""
    ntopts="${ntopts} -DCMAKE_EXE_LINKER_FLAGS:STRING=${lf}"
    ntopts="${ntopts} -DCMAKE_MODULE_LINKER_FLAGS:STRING=${lf}"
    ntopts="${ntopts} -DCMAKE_SHARED_LINKER_FLAGS:STRING=${lf}"

    # Nektar++ specific options for a faster build.
#    ntopts="${ntopts} -DTHIRDPARTY_BUILD_BOOST:BOOL=ON"
    ntopts="${ntopts} -DNEKTAR_BUILD_DEMOS:BOOL=OFF"
    ntopts="${ntopts} -DNEKTAR_BUILD_SOLVERS:BOOL=OFF"
    ntopts="${ntopts} -DNEKTAR_BUILD_UTILITIES:BOOL=OFF"
    ntopts="${ntopts} -DNEKTAR_BUILD_TESTS:BOOL=OFF"
    ntopts="${ntopts} -DNEKTAR_BUILD_UNIT_TESTS:BOOL=OFF"

#    if test "${OPSYS}" = "Darwin" ; then
#        ntopts="${ntopts} -DCMAKE_INSTALL_NAME_DIR:PATH=${nektar_plus_plus_inst_path}/lib"
#    fi

    if test "x${DO_BOOST}" = "xyes"; then
        info "boost requested.  Configuring NEKTAR++ with boost support."
        ntopts="${ntopts} -DBOOST_ROOT:PATH=${VISITDIR}/boost/${BOOST_VERSION}/${VISITARCH}"

        if [[ "$OPSYS" == "Darwin" ]]; then
            export DYLD_LIBRARY_PATH="$VISITDIR/boost/$BOOST_VERSION/$VISITARCH/lib":$DYLD_LIBRARY_PATH
        else
            export LD_LIBRARY_PATH="$VISITDIR/boost/$BOOST_VERSION/$VISITARCH/lib":$LD_LIBRARY_PATH
        fi
    fi


    if test "x${DO_ZLIB}" = "xyes"; then
        info "zlib requested.  Configuring NEKTAR++ with zlib support."
        ntopts="${ntopts} -DZLIB_ROOT:PATH=${VISITDIR}/zlib/${ZLIB_VERSION}/${VISITARCH}"

        if [[ "$OPSYS" == "Darwin" ]]; then
            export DYLD_LIBRARY_PATH="$VISITDIR/zlib/$ZLIB_VERSION/$VISITARCH/lib":$DYLD_LIBRARY_PATH
        else
            export LD_LIBRARY_PATH="$VISITDIR/zlib/$ZLIB_VERSION/$VISITARCH/lib":$LD_LIBRARY_PATH
        fi
    fi

#    if test "x${DO_MPICH}" = "xyes"; then
#        info "mpich requested.  Configuring NEKTAR++ with mpich support."
#        ntopts="${ntopts} -DMPI_ROOT:PATH=${VISITDIR}/mpich/${MPICH_VERSION}/${VISITARCH}"

#        if [[ "$OPSYS" == "Darwin" ]]; then
#            export DYLD_LIBRARY_PATH="$VISITDIR/mpich/$MPICH_VERSION/$VISITARCH/lib":$DYLD_LIBRARY_PATH
#        else
#            export LD_LIBRARY_PATH="$VISITDIR/mpich/$MPICH_VERSION/$VISITARCH/lib":$LD_LIBRARY_PATH
#        fi
#    fi

#    if test "x${DO_VTK}" = "xyes"; then
#        info "vtk requested.  Configuring NEKTAR++ with vtk support."
#        ntopts="${ntopts} -DNEKTAR_USE_VTK=ON -DVTK_DIR:PATH=${VISITDIR}/${VTK_INSTALL_DIR}/${VTK_VERSION}/${VISITARCH}/lib/cmake/vtk-${VTK_SHORT_VERSION}"

#        if [[ "$OPSYS" == "Darwin" ]]; then
#            export DYLD_LIBRARY_PATH="$VISITDIR/$VTK_INSTALL_DIR/$VTK_VERSION/$VISITARCH/lib":$DYLD_LIBRARY_PATH
#        else
#            export LD_LIBRARY_PATH="$VISITDIR/$VTK_INSTALL_DIR/$VTK_VERSION/$VISITARCH/lib":$LD_LIBRARY_PATH
#        fi
#    fi

    info "Configuring Nektar++ . . ."

    cd "$START_DIR"

    # Make a build directory for an out-of-source build.. Change the
    # VISIT_BUILD_DIR variable to represent the out-of-source build directory.
    NEKTAR_PLUS_PLUS_SRC_DIR=$NEKTAR_PLUS_PLUS_BUILD_DIR
    NEKTAR_PLUS_PLUS_BUILD_DIR="${NEKTAR_PLUS_PLUS_SRC_DIR}-build"
    if [[ ! -d $NEKTAR_PLUS_PLUS_BUILD_DIR ]] ; then
        echo "Making build directory $NEKTAR_PLUS_PLUS_BUILD_DIR"
        mkdir $NEKTAR_PLUS_PLUS_BUILD_DIR
    fi

    CMAKE_BIN="${CMAKE_INSTALL}/cmake"

    cd ${NEKTAR_PLUS_PLUS_BUILD_DIR}

    if test -e bv_run_cmake.sh ; then
        rm -f bv_run_cmake.sh
    fi

    #
    # Remove the CMakeCache.txt files ... existing files sometimes prevent
    # fields from getting overwritten properly.
    #
    rm -Rf ${NEKTAR_PLUS_PLUS_BUILD_DIR}/CMakeCache.txt ${NEKTAR_PLUS_PLUS_BUILD_DIR}/*/CMakeCache.txt

    echo "\"${CMAKE_BIN}\"" ${ntopts} ../${NEKTAR_PLUS_PLUS_SRC_DIR} > bv_run_cmake.sh
    cat bv_run_cmake.sh
    issue_command bash bv_run_cmake.sh || error "Nektar++ configuration failed."

    #
    # Build NEKTAR_PLUS_PLUS
    #
    info "Making Nektar++ . . ."
    $MAKE $MAKE_OPT_FLAGS
    if [[ $? != 0 ]] ; then
       warn "Nektar++ build failed.  Giving up"
       return 1
    fi

    #
    # Install into the VisIt third party location.
    #
    info "Installing Nektar++ . . ."
    $MAKE install
    if [[ $? != 0 ]] ; then
       warn "Nektar++ install failed.  Giving up"
       return 1
    fi

#    mv ${nektar_plus_plus_inst_path}/lib64/* ${nektar_plus_plus_inst_path}/lib

    if [[ "$DO_GROUP" == "yes" ]] ; then
       chmod -R ug+w,a+rX "$VISITDIR/nektar++"
       chgrp -R ${GROUP} "$VISITDIR/nektar++"
    fi
    cd "$START_DIR"
    info "Done with Nektar++"
    return 0
}

function bv_nektar++_is_enabled
{
    if [[ $DO_NEKTAR_PLUS_PLUS == "yes" ]]; then
        return 1    
    fi
    return 0
}

function bv_nektar++_is_installed
{

    if [[ "$USE_SYSTEM_NEKTAR_PLUS_PLUS" == "yes" ]]; then
        return 1
    fi

    check_if_installed "nektar++" $NEKTAR_PLUS_PLUS_VERSION
    if [[ $? == 0 ]] ; then
        return 1
    fi
    return 0
}

function bv_nektar++_build
{
cd "$START_DIR"

if [[ "$DO_NEKTAR_PLUS_PLUS" == "yes" && "$USE_SYSTEM_NEKTAR_PLUS_PLUS" == "no" ]] ; then
    check_if_installed "nektar++" $NEKTAR_PLUS_PLUS_VERSION
    if [[ $? == 0 ]] ; then
        info "Skipping Nektar++ build.  Nektar++ is already installed."
    else
        info "Building Nektar++ (~10 minutes)"
        build_nektar++
        if [[ $? != 0 ]] ; then
            error "Unable to build or install Nektar++.  Bailing out."
        fi
        info "Done building Nektar++"
    fi
fi
}
