function bv_nektarpp_initialize
{
    export DO_NEKTAR_PLUS_PLUS="no"
    export USE_SYSTEM_NEKTAR_PLUS_PLUS="no"
    add_extra_commandline_args "nektarpp" "alt-nektarpp-dir" 1 "Use alternative directory for nektar++"
}

function bv_nektarpp_enable
{
    DO_NEKTAR_PLUS_PLUS="yes"
}

function bv_nektarpp_disable
{
    DO_NEKTAR_PLUS_PLUS="no"
}

function bv_nektarpp_alt_nektarpp_dir
{
    bv_nektarpp_enable
    USE_SYSTEM_NEKTAR_PLUS_PLUS="yes"
    NEKTAR_PLUS_PLUS_INSTALL_DIR="$1"
}

function bv_nektarpp_depends_on
{
    if [[ "$USE_SYSTEM_NEKTAR_PLUS_PLUS" == "yes" ]]; then
        echo ""
    else
        echo "cmake boost zlib"
    fi
}

function bv_nektarpp_initialize_vars
{
    if [[ "$USE_SYSTEM_NEKTAR_PLUS_PLUS" == "no" ]]; then
        NEKTAR_PLUS_PLUS_INSTALL_DIR="${VISITDIR}/nektar++/$NEKTAR_PLUS_PLUS_VERSION/${VISITARCH}"
    fi
}

function bv_nektarpp_info
{
    export NEKTAR_PLUS_PLUS_VERSION=${NEKTAR_PLUS_PLUS_VERSION:-"5.0.0"}
    export NEKTAR_PLUS_PLUS_FILE=${NEKTAR_PLUS_PLUS_FILE:-"nektar-${NEKTAR_PLUS_PLUS_VERSION}.tar.gz"}
    export NEKTAR_PLUS_PLUS_COMPATIBILITY_VERSION=${NEKTAR_PLUS_PLUS_COMPATIBILITY_VERSION:-"5.0"}
    export NEKTAR_PLUS_PLUS_URL=${NEKTAR_PLUS_PLUS_URL:-"https://www.nektar.info/wp-content/uploads/2019/12/"}
    export NEKTAR_PLUS_PLUS_BUILD_DIR=${NEKTAR_PLUS_PLUS_BUILD_DIR:-"nektar++-${NEKTAR_PLUS_PLUS_VERSION}"}
    export NEKTAR_PLUS_PLUS_SHA256_CHECKSUM="6e759541ecba1e814856b89ae1e788c2d266f757f5149b13d7dd1d71ed1215b2"
}

function bv_nektarpp_print
{
    printf "%s%s\n" "NEKTAR_PLUS_PLUS_FILE=" "${NEKTAR_PLUS_PLUS_FILE}"
    printf "%s%s\n" "NEKTAR_PLUS_PLUS_VERSION=" "${NEKTAR_PLUS_PLUS_VERSION}"
    printf "%s%s\n" "NEKTAR_PLUS_PLUS_COMPATIBILITY_VERSION=" "${NEKTAR_PLUS_PLUS_COMPATIBILITY_VERSION}"
    printf "%s%s\n" "NEKTAR_PLUS_PLUS_BUILD_DIR=" "${NEKTAR_PLUS_PLUS_BUILD_DIR}"
}

function bv_nektarpp_print_usage
{
    printf "%-20s %s [%s]\n" "--nektarpp" "Build Nektar++" "${DO_NEKTAR_PLUS_PLUS}"
    printf "%-20s %s [%s]\n" "--alt-nektarpp-dir" "Use Nektar++ from an alternative directory"
}

function bv_nektarpp_host_profile
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

            ZLIB_LIBDEP="\${VISITHOME}/zlib/\${ZLIB_VERSION}/\${VISITARCH}/lib z"

            echo \
                "VISIT_OPTION_DEFAULT(VISIT_NEKTAR++_LIBDEP $ZLIB_LIBDEP TYPE STRING)" \
                >> $HOSTCONF
        fi
    fi
}

function bv_nektarpp_ensure
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

function apply_nektarpp_4_4_0_patch
{
    info "Patching Nektar++ 4.4.0"
    patch -p0 << \EOF
diff -rcN nektar++-4.4.0/library/LibUtilities/Communication/CommDataType_orig.h nektar++-4.4.0/library/LibUtilities/Communication/CommDataType.h
*** nektar++-4.4.0/library/LibUtilities/Communication/CommDataType_orig.h	2017-03-06 11:04:22.000000000 -0700
--- nektar++-4.4.0/library/LibUtilities/Communication/CommDataType.h	2017-09-05 14:22:16.000000000 -0600
***************
*** 56,73 ****
  {
  namespace LibUtilities
  {
! enum CommDataType
! {
!     MPI_INT,
!     MPI_UNSIGNED,
!     MPI_LONG,
!     MPI_UNSIGNED_LONG,
!     MPI_LONG_LONG,
!     MPI_UNSIGNED_LONG_LONG,
!     MPI_FLOAT,
!     MPI_DOUBLE,
!     MPI_LONG_DOUBLE
! };
  }
  }
  #endif
--- 56,99 ----
  {
  namespace LibUtilities
  {
! typedef int CommDataType;
!
! #ifndef MPI_INT
!     #define MPI_INT            ((CommDataType)0x4c000405)
! #endif
!
! #ifndef MPI_UNSIGNED
!     #define MPI_UNSIGNED       ((CommDataType)0x4c000406)
! #endif
!
! #ifndef MPI_LONG
!     #define MPI_LONG           ((CommDataType)0x4c000807)
! #endif
!
! #ifndef MPI_UNSIGNED_LONG
!     #define MPI_UNSIGNED_LONG  ((CommDataType)0x4c000808)
! #endif
!
! #ifndef MPI_LONG_LONG
!     #define MPI_LONG_LONG      ((CommDataType)0x4c000809)
! #endif
!
! #ifndef MPI_UNSIGNED_LONG_LONG
!     #define MPI_UNSIGNED_LONG_LONG ((CommDataType)0x4c000819)
! #endif
!
! #ifndef MPI_FLOAT
!     #define MPI_FLOAT          ((CommDataType)0x4c00040a)
! #endif
!
! #ifndef MPI_DOUBLE
!     #define MPI_DOUBLE         ((CommDataType)0x4c00080b)
! #endif
!
! #ifndef MPI_LONG_DOUBLE
!     #define MPI_LONG_DOUBLE    ((CommDataType)0x4c00100c)
! #endif
!
  }
  }
  #endif
EOF
}

function apply_nektarpp_4_4_1_patch
{
    info "Patching Nektar++ 4.4.1"
    patch -p0 << \EOF
diff -rcN nektar++-4.4.1/library/LibUtilities/Communication/CommDataType_orig.h nektar++-4.4.1/library/LibUtilities/Communication/CommDataType.h
*** nektar++-4.4.1/library/LibUtilities/Communication/CommDataType_orig.h	2017-03-06 11:04:22.000000000 -0700
--- nektar++-4.4.1/library/LibUtilities/Communication/CommDataType.h	2017-09-05 14:22:16.000000000 -0600
***************
*** 56,73 ****
  {
  namespace LibUtilities
  {
! enum CommDataType
! {
!     MPI_INT,
!     MPI_UNSIGNED,
!     MPI_LONG,
!     MPI_UNSIGNED_LONG,
!     MPI_LONG_LONG,
!     MPI_UNSIGNED_LONG_LONG,
!     MPI_FLOAT,
!     MPI_DOUBLE,
!     MPI_LONG_DOUBLE
! };
  }
  }
  #endif
--- 56,99 ----
  {
  namespace LibUtilities
  {
! typedef int CommDataType;
!
! #ifndef MPI_INT
!     #define MPI_INT            ((CommDataType)0x4c000405)
! #endif
!
! #ifndef MPI_UNSIGNED
!     #define MPI_UNSIGNED       ((CommDataType)0x4c000406)
! #endif
!
! #ifndef MPI_LONG
!     #define MPI_LONG           ((CommDataType)0x4c000807)
! #endif
!
! #ifndef MPI_UNSIGNED_LONG
!     #define MPI_UNSIGNED_LONG  ((CommDataType)0x4c000808)
! #endif
!
! #ifndef MPI_LONG_LONG
!     #define MPI_LONG_LONG      ((CommDataType)0x4c000809)
! #endif
!
! #ifndef MPI_UNSIGNED_LONG_LONG
!     #define MPI_UNSIGNED_LONG_LONG ((CommDataType)0x4c000819)
! #endif
!
! #ifndef MPI_FLOAT
!     #define MPI_FLOAT          ((CommDataType)0x4c00040a)
! #endif
!
! #ifndef MPI_DOUBLE
!     #define MPI_DOUBLE         ((CommDataType)0x4c00080b)
! #endif
!
! #ifndef MPI_LONG_DOUBLE
!     #define MPI_LONG_DOUBLE    ((CommDataType)0x4c00100c)
! #endif
!
  }
  }
  #endif
EOF
}

function apply_nektarpp_5_0_0_patch
{
    info "Patching Nektar++ 5.0.0"
    patch -p0 << \EOF
diff -rcN nektar++-5.0.0/library/LibUtilities/Communication/CommDataType_orig.h nektar++-5.0.0/library/LibUtilities/Communication/CommDataType.h
*** nektar++-5.0.0/library/LibUtilities/Communication/CommDataType_orig.h	2020-02-10 17:12:37.037503006 -0700
--- nektar++-5.0.0/library/LibUtilities/Communication/CommDataType.h	2020-02-10 17:14:00.179539468 -0700
***************
*** 67,85 ****
  {
  namespace LibUtilities
  {
! enum CommDataType
! {
!     MPI_CHAR,
!     MPI_INT,
!     MPI_UNSIGNED,
!     MPI_LONG,
!     MPI_UNSIGNED_LONG,
!     MPI_LONG_LONG,
!     MPI_UNSIGNED_LONG_LONG,
!     MPI_FLOAT,
!     MPI_DOUBLE,
!     MPI_LONG_DOUBLE
! };
  }
  }
  #endif
--- 67,113 ----
  {
  namespace LibUtilities
  {
! typedef int CommDataType;
!
! #ifndef MPI_CHAR
!     #define MPI_CHAR            ((CommDataType)0x4c000101)
! #endif
!
! #ifndef MPI_INT
!     #define MPI_INT            ((CommDataType)0x4c000405)
! #endif
!
! #ifndef MPI_UNSIGNED
!     #define MPI_UNSIGNED       ((CommDataType)0x4c000406)
! #endif
!
! #ifndef MPI_LONG
!     #define MPI_LONG           ((CommDataType)0x4c000807)
! #endif
!
! #ifndef MPI_UNSIGNED_LONG
!     #define MPI_UNSIGNED_LONG  ((CommDataType)0x4c000808)
! #endif
!
! #ifndef MPI_LONG_LONG
!     #define MPI_LONG_LONG      ((CommDataType)0x4c000809)
! #endif
!
! #ifndef MPI_UNSIGNED_LONG_LONG
!     #define MPI_UNSIGNED_LONG_LONG ((CommDataType)0x4c000819)
! #endif
!
! #ifndef MPI_FLOAT
!     #define MPI_FLOAT          ((CommDataType)0x4c00040a)
! #endif
!
! #ifndef MPI_DOUBLE
!     #define MPI_DOUBLE         ((CommDataType)0x4c00080b)
! #endif
!
! #ifndef MPI_LONG_DOUBLE
!     #define MPI_LONG_DOUBLE    ((CommDataType)0x4c00100c)
! #endif
  }
  }
  #endif
EOF
}

function apply_nektarpp_4_4_0_OSX_patch
{
    info "Patching Nektar++ 4.4.0 for OS X"
    patch -p0 << \EOF
diff -rcN nektar++-4.4.0/CMakeLists_orig.txt  nektar++-4.4.0/CMakeLists.txt
*** nektar++-4.4.0/CMakeLists_orig.txt	2017-03-06 11:04:22.000000000 -0700
--- nektar++-4.4.0/CMakeLists.txt	2017-09-05 14:47:37.000000000 -0600
***************
*** 326,333 ****

  # Build active components
  IF (NEKTAR_BUILD_LIBRARY)
!     SET(NEKTAR++_LIBRARIES SolverUtils LibUtilities StdRegions SpatialDomains LocalRegions
!         MultiRegions Collections GlobalMapping FieldUtils NekMeshUtils)
      INCLUDE_DIRECTORIES(library)
      ADD_SUBDIRECTORY(library)
      INSTALL(EXPORT Nektar++Libraries DESTINATION ${LIB_DIR}/cmake COMPONENT dev)
--- 326,333 ----

  # Build active components
  IF (NEKTAR_BUILD_LIBRARY)
!     SET(NEKTAR++_LIBRARIES LibUtilities StdRegions SpatialDomains LocalRegions
!         MultiRegions Collections GlobalMapping FieldUtils)
      INCLUDE_DIRECTORIES(library)
      ADD_SUBDIRECTORY(library)
      INSTALL(EXPORT Nektar++Libraries DESTINATION ${LIB_DIR}/cmake COMPONENT dev)
EOF
}

function apply_nektarpp_4_4_1_OSX_patch
{
    info "Patching Nektar++ 4.4.1 for OS X"
    patch -p0 << \EOF
diff -rcN nektar++-4.4.1/CMakeLists.orig.txt  nektar++-4.4.1/CMakeLists.txt
*** nektar++-4.4.1/CMakeLists.orig.txt	2018-11-06 08:03:29.000000000 -0700
--- nektar++-4.4.1/CMakeLists.txt	2018-11-06 08:04:33.000000000 -0700
***************
*** 326,333 ****

  # Build active components
  IF (NEKTAR_BUILD_LIBRARY)
!     SET(NEKTAR++_LIBRARIES SolverUtils LibUtilities StdRegions SpatialDomains LocalRegions
!         MultiRegions Collections GlobalMapping FieldUtils NekMeshUtils)
      INCLUDE_DIRECTORIES(library)
      ADD_SUBDIRECTORY(library)
      INSTALL(EXPORT Nektar++Libraries DESTINATION ${LIB_DIR}/cmake COMPONENT dev)
--- 326,333 ----

  # Build active components
  IF (NEKTAR_BUILD_LIBRARY)
!     SET(NEKTAR++_LIBRARIES LibUtilities StdRegions SpatialDomains LocalRegions
!         MultiRegions Collections GlobalMapping FieldUtils)
      INCLUDE_DIRECTORIES(library)
      ADD_SUBDIRECTORY(library)
      INSTALL(EXPORT Nektar++Libraries DESTINATION ${LIB_DIR}/cmake COMPONENT dev)
EOF
}

function apply_nektarpp_patch
{
    if [[ "${NEKTAR_PLUS_PLUS_VERSION}" == 4.4.0 ]] ; then
        apply_nektarpp_4_4_0_patch
        if [[ $? != 0 ]]; then
           return 1
        fi

#        if [[ "$OPSYS" == "Darwin" ]]; then
            apply_nektarpp_4_4_OSX_patch
            if [[ $? != 0 ]]; then
		return 1
            fi
#	fi

    elif [[ "${NEKTAR_PLUS_PLUS_VERSION}" == 4.4.1 ]] ; then
        apply_nektarpp_4_4_1_patch
        if [[ $? != 0 ]]; then
           return 1
        fi

#        if [[ "$OPSYS" == "Darwin" ]]; then
            apply_nektarpp_4_4_1_OSX_patch
            if [[ $? != 0 ]]; then
		return 1
            fi
#	fi
    elif [[ "${NEKTAR_PLUS_PLUS_VERSION}" == 5.0.0 ]] ; then
        apply_nektarpp_5_0_0_patch
        if [[ $? != 0 ]]; then
           return 1
        fi
    fi

    return 0
}

# *************************************************************************** #
#              Function 8.1, build_nektarpp                                   #
# *************************************************************************** #
function build_nektarpp
{
    #
    # CMake is the build system for VTK.  Call another script that will build
    # that program.
    #
    CMAKE_INSTALL=${CMAKE_INSTALL:-"$VISITDIR/cmake/${CMAKE_VERSION}/$VISITARCH/bin"}
    if [[ -e ${CMAKE_INSTALL}/cmake ]] ; then
        info "Nektar++: CMake found"
    else
        warn "Unable to find cmake, cannot build Nektar++. Giving up."
        return 1
    fi

    #
    # Prepare build dir
    #
    prepare_build_dir $NEKTAR_PLUS_PLUS_BUILD_DIR $NEKTAR_PLUS_PLUS_FILE
    untarred_nektar_plus_plus=$?
    # 0, already exists, 1 untarred src, 2 error

    if [[ $untarred_nektar_plus_plus == -1 ]] ; then
        warn "Unable to prepare Nektar++ Build Directory. Giving Up"
        return 1
    fi

    #
    # Apply patches
    #
    info "Patching Nektar++ . . ."
    apply_nektarpp_patch
    if [[ $? != 0 ]] ; then
        if [[ $untarred_nektar_plus_plus == 1 ]] ; then
            warn "Giving up on Nektar++ build because the patch failed."
            return 1
        else
            warn "Patch failed, but continuing.  I believe that this script\n" \
                 "tried to apply a patch to an existing directory that had\n" \
                 "already been patched ... that is, the patch is\n" \
                 "failing harmlessly on a second application."
        fi
    fi

    #
    cd $NEKTAR_PLUS_PLUS_BUILD_DIR || error "Can't cd to Nektar++ build dir." $NEKTAR_PLUS_PLUS_BUILD_DIR

    #
    # Configure Nektar++
    #
    info "Configuring Nektar++ . . ."

    ntopts=""
    nektar_plus_plus_build_mode="${VISIT_BUILD_MODE}"
    nektar_plus_plus_inst_path="${NEKTAR_PLUS_PLUS_INSTALL_DIR}"

    ntopts="${ntopts} -DCMAKE_BUILD_TYPE:STRING=${nektar_plus_plus_build_mode}"
    ntopts="${ntopts} -DCMAKE_INSTALL_PREFIX:PATH=${nektar_plus_plus_inst_path}"

    ntopts="${ntopts} -DCMAKE_C_COMPILER:STRING=${C_COMPILER}"
    ntopts="${ntopts} -DCMAKE_CXX_COMPILER:STRING=${CXX_COMPILER}"
    ntopts="${ntopts} -DCMAKE_C_FLAGS:STRING=\"${C_OPT_FLAGS}\""
    ntopts="${ntopts} -DCMAKE_CXX_FLAGS:STRING=\"${CXX_OPT_FLAGS}\""
#    ntopts="${ntopts} -DCMAKE_EXE_LINKER_FLAGS:STRING=${lf}"
#    ntopts="${ntopts} -DCMAKE_MODULE_LINKER_FLAGS:STRING=${lf}"
#    ntopts="${ntopts} -DCMAKE_SHARED_LINKER_FLAGS:STRING=${lf}"

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

    if [[ "$DO_BOOST" == "yes" ]] ; then
        info "boost requested.  Configuring NEKTAR++ with boost support."
        ntopts="${ntopts} -DBOOST_ROOT:PATH=${VISITDIR}/boost/${BOOST_VERSION}/${VISITARCH}"

        if [[ "$OPSYS" == "Darwin" ]]; then
            export DYLD_LIBRARY_PATH="$VISITDIR/boost/$BOOST_VERSION/$VISITARCH/lib":$DYLD_LIBRARY_PATH
        else
            export LD_LIBRARY_PATH="$VISITDIR/boost/$BOOST_VERSION/$VISITARCH/lib":$LD_LIBRARY_PATH
        fi
    fi

    info "Configuring NEKTAR++ with zlib support."
    ntopts="${ntopts} -DZLIB_ROOT:PATH=${VISITDIR}/zlib/${ZLIB_VERSION}/${VISITARCH}"

    if [[ "$OPSYS" == "Darwin" ]]; then
        export DYLD_LIBRARY_PATH="$VISITDIR/zlib/$ZLIB_VERSION/$VISITARCH/lib":$DYLD_LIBRARY_PATH
    else
        export LD_LIBRARY_PATH="$VISITDIR/zlib/$ZLIB_VERSION/$VISITARCH/lib":$LD_LIBRARY_PATH
    fi

#    if test "${DO_MPICH}" = "yes"; then
#        info "mpich requested.  Configuring NEKTAR++ with mpich support."
#        ntopts="${ntopts} -DMPI_ROOT:PATH=${VISITDIR}/mpich/${MPICH_VERSION}/${VISITARCH}"

#        if [[ "$OPSYS" == "Darwin" ]]; then
#            export DYLD_LIBRARY_PATH="$VISITDIR/mpich/$MPICH_VERSION/$VISITARCH/lib":$DYLD_LIBRARY_PATH
#        else
#            export LD_LIBRARY_PATH="$VISITDIR/mpich/$MPICH_VERSION/$VISITARCH/lib":$LD_LIBRARY_PATH
#        fi
#    fi

#        if test "${DO_VTK}" = "yes"; then
#            info "vtk requested.  Configuring NEKTAR++ with vtk support."
#            ntopts="${ntopts} -DNEKTAR_USE_VTK=ON -DVTK_DIR:PATH=${VISITDIR}/${VTK_INSTALL_DIR}/${VTK_VERSION}/${VISITARCH}/lib/cmake/vtk-${VTK_SHORT_VERSION}"

#            if [[ "$OPSYS" == "Darwin" ]]; then
#                export DYLD_LIBRARY_PATH="$VISITDIR/$VTK_INSTALL_DIR/$VTK_VERSION/$VISITARCH/lib":$DYLD_LIBRARY_PATH
#            else
#                export LD_LIBRARY_PATH="$VISITDIR/$VTK_INSTALL_DIR/$VTK_VERSION/$VISITARCH/lib":$LD_LIBRARY_PATH
#            fi
#        fi

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

function bv_nektarpp_is_enabled
{
    if [[ $DO_NEKTAR_PLUS_PLUS == "yes" ]]; then
        return 1
    fi
    return 0
}

function bv_nektarpp_is_installed
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

function bv_nektarpp_build
{
    cd "$START_DIR"

    if [[ "$DO_NEKTAR_PLUS_PLUS" == "yes" && "$USE_SYSTEM_NEKTAR_PLUS_PLUS" == "no" ]] ; then
        check_if_installed "nektar++" $NEKTAR_PLUS_PLUS_VERSION
        if [[ $? == 0 ]] ; then
            info "Skipping Nektar++ build.  Nektar++ is already installed."
        else
            info "Building Nektar++ (~10 minutes)"
            build_nektarpp
            if [[ $? != 0 ]] ; then
                error "Unable to build or install Nektar++.  Bailing out."
            fi
            info "Done building Nektar++"
        fi
    fi
}
