function bv_xdmf_initialize
{
    export DO_XDMF="no"
}

function bv_xdmf_enable
{
    DO_XDMF="yes"

    #xdmf is dependent on HDF5
    DO_HDF5="yes"
}

function bv_xdmf_disable
{
    DO_XDMF="no"
}

function bv_xdmf_depends_on
{
    echo "cmake vtk hdf5 zlib"
}

function bv_xdmf_info
{
    export XDMF_FILE=${XDMF_FILE:-"Xdmf-2.1.1.tar.gz"}
    export XDMF_VERSION=${XDMF_VERSION:-"2.1.1"}
    export XDMF_COMPATIBILITY_VERSION=${XDMF_COMPATIBILITY_VERSION:-"2.1.1"}
    export XDMF_BUILD_DIR=${XDMF_BUILD_DIR:-"Xdmf"}
    export XDMF_MD5_CHECKSUM="09e2afd3a1b7b3e7d650b860212a95d1"
    export XDMF_SHA256_CHECKSUM="4f0c2011d1d6f86052b102b25b36276168a31e191b4206a8d0c9d716ebced7e1"
}

function bv_xdmf_print
{
    printf "%s%s\n" "XDMF_FILE=" "${XDMF_FILE}"
    printf "%s%s\n" "XDMF_VERSION=" "${XDMF_VERSION}"
    printf "%s%s\n" "XDMF_COMPATIBILITY_VERSION=" "${XDMF_COMPATIBILITY_VERSION}"
    printf "%s%s\n" "XDMF_BUILD_DIR=" "${XDMF_BUILD_DIR}"
}

function bv_xdmf_print_usage
{
    printf "%-20s %s [%s]\n" "--xdmf" "Build Xdmf" "$DO_XDMF"
}

function bv_xdmf_host_profile
{
    if [[ "$DO_XDMF" == "yes" ]] ; then
        echo >> $HOSTCONF
        echo "##" >> $HOSTCONF
        echo "## Xdmf" >> $HOSTCONF
        echo "##" >> $HOSTCONF
        echo \
            "VISIT_OPTION_DEFAULT(VISIT_XDMF_DIR \${VISITHOME}/Xdmf/$XDMF_VERSION/\${VISITARCH})" \
            >> $HOSTCONF
        if [[ "$DO_VTK9" == "yes" ]] ; then
            if test -e $VISITDIR/${VTK_INSTALL_DIR}/$VTK_VERSION/$VISITARCH/lib64 ; then
                echo \
                    "VISIT_OPTION_DEFAULT(VISIT_XDMF_LIBDEP HDF5_LIBRARY_DIR hdf5 ${VISIT_HDF5_LIBDEP} \${VISIT_VTK_DIR}/lib64 vtklibxml2-\${VTK_MAJOR_VERSION}.\${VTK_MINOR_VERSION} ${VISIT_VTK_LIBDEP} TYPE STRING)"\
                    >> $HOSTCONF
            else
                echo \
                    "VISIT_OPTION_DEFAULT(VISIT_XDMF_LIBDEP HDF5_LIBRARY_DIR hdf5 ${VISIT_HDF5_LIBDEP} \${VISIT_VTK_DIR}/lib vtklibxml2-\${VTK_MAJOR_VERSION}.\${VTK_MINOR_VERSION} ${VISIT_VTK_LIBDEP} TYPE STRING)"\
                    >> $HOSTCONF
            fi
        else
            echo \
                "VISIT_OPTION_DEFAULT(VISIT_XDMF_LIBDEP HDF5_LIBRARY_DIR hdf5 ${VISIT_HDF5_LIBDEP} VTK_LIBRARY_DIRS vtklibxml2-\${VTK_MAJOR_VERSION}.\${VTK_MINOR_VERSION} ${VISIT_VTK_LIBDEP} TYPE STRING)"\
                >> $HOSTCONF
        fi
    fi
}

function bv_xdmf_ensure
{
    if [[ "$DO_XDMF" == "yes" ]] ; then
        ensure_built_or_ready "Xdmf" $XDMF_VERSION $XDMF_BUILD_DIR $XDMF_FILE
        if [[ $? != 0 ]] ; then
            ANY_ERRORS="yes"
            DO_XDMF="no"
            error "Unable to build XDMF.  ${XDMF_FILE} not found."
        fi
    fi
}


# *************************************************************************** #
#                         Function 8.19, build_xdmf                           #
# *************************************************************************** #

function apply_xdmf_patch1
{
   patch -p0 << \EOF
diff -c a/libsrc/FXdmfValuesBinary.cxx Xdmf/libsrc/XdmfValuesBinary.cxx
*** a/libsrc/XdmfValuesBinary.cxx
--- Xdmf/libsrc/XdmfValuesBinary.cxx
***************
*** 282,288 ****
      }
      FullFileName << DataSetName << ends;
      char * path = FullFileName.rdbuf()->str();
!     XdmfDebug("Opening Binary Data for Reading : " << FullFileName);


      //char * path = new char [ strlen(this->DOM->GetWorkingDirectory())+strlen(DataSetName) + 1 ];
--- 282,288 ----
      }
      FullFileName << DataSetName << ends;
      char * path = FullFileName.rdbuf()->str();
!     XdmfDebug("Opening Binary Data for Reading : " << path);


      //char * path = new char [ strlen(this->DOM->GetWorkingDirectory())+strlen(DataSetName) + 1 ];
EOF
    if [[ $? != 0 ]] ; then
        warn "Xdmf patch1 failed."
        return 1
    fi

    return 0;

}

function apply_xdmf_osx_patch
{
    info "Patching Xdmf 2.1.1 for Xcode 9 and up . . ."
    patch -p0 << \EOF
diff -c Xdmf/libsrc/XdmfDsmComm.cxx.orig Xdmf/libsrc/XdmfDsmComm.cxx
*** Xdmf/libsrc/XdmfDsmComm.cxx.orig    Thu Aug 23 22:05:42 2018
--- Xdmf/libsrc/XdmfDsmComm.cxx         Thu Aug 23 21:27:43 2018
***************
*** 50,56 ****
          XdmfErrorMessage("Cannot Receive Message of Length = " << Msg->Length);
          return(XDMF_FAIL);
      }
!     if(Msg->Data <= 0 ){
          XdmfErrorMessage("Cannot Receive Message into Data Buffer = " << Msg->Length);
          return(XDMF_FAIL);
      }
--- 50,56 ----
          XdmfErrorMessage("Cannot Receive Message of Length = " << Msg->Length);
          return(XDMF_FAIL);
      }
!     if(Msg->Data == (void *) NULL ){
          XdmfErrorMessage("Cannot Receive Message into Data Buffer = " << Msg->Length);
          return(XDMF_FAIL);
      }
***************
*** 64,70 ****
          XdmfErrorMessage("Cannot Send Message of Length = " << Msg->Length);
          return(XDMF_FAIL);
      }
!     if(Msg->Data <= 0 ){
          XdmfErrorMessage("Cannot Send Message from Data Buffer = " << Msg->Length);
          return(XDMF_FAIL);
      }
--- 64,70 ----
          XdmfErrorMessage("Cannot Send Message of Length = " << Msg->Length);
          return(XDMF_FAIL);
      }
!     if(Msg->Data == (void *) NULL ){
          XdmfErrorMessage("Cannot Send Message from Data Buffer = " << Msg->Length);
          return(XDMF_FAIL);
      }
EOF
    if [[ $? != 0 ]] ; then
        warn "Xdmf 2.1.1 Xcode 9 patch failed."
        return 1
    fi

    return 0;
}

function apply_xdmf_gcc_11_2_patch
{
    info "Patching Xdmf 2.1.1 for gcc 11.2 . . ."
    patch -p0 << \EOF
diff -c Xdmf/libsrc/XdmfDsmComm.cxx.orig Xdmf/libsrc/XdmfDsmComm.cxx
*** Xdmf/libsrc/XdmfDsmComm.cxx.orig    Fri May 20 12:34:02 2022
--- Xdmf/libsrc/XdmfDsmComm.cxx         Fri May 20 12:34:50 2022
***************
*** 50,56 ****
          XdmfErrorMessage("Cannot Receive Message of Length = " << Msg->Length);
          return(XDMF_FAIL);
      }
!     if(Msg->Data <= 0 ){
          XdmfErrorMessage("Cannot Receive Message into Data Buffer = " << Msg->Length);
          return(XDMF_FAIL);
      }
--- 50,56 ----
          XdmfErrorMessage("Cannot Receive Message of Length = " << Msg->Length);
          return(XDMF_FAIL);
      }
!     if(Msg->Data == (void*)0 ){
          XdmfErrorMessage("Cannot Receive Message into Data Buffer = " << Msg->Length);
          return(XDMF_FAIL);
      }
***************
*** 64,70 ****
          XdmfErrorMessage("Cannot Send Message of Length = " << Msg->Length);
          return(XDMF_FAIL);
      }
!     if(Msg->Data <= 0 ){
          XdmfErrorMessage("Cannot Send Message from Data Buffer = " << Msg->Length);
          return(XDMF_FAIL);
      }
--- 64,70 ----
          XdmfErrorMessage("Cannot Send Message of Length = " << Msg->Length);
          return(XDMF_FAIL);
      }
!     if(Msg->Data == (void*)0 ){
          XdmfErrorMessage("Cannot Send Message from Data Buffer = " << Msg->Length);
          return(XDMF_FAIL);
      }
EOF
    if [[ $? != 0 ]] ; then
        warn "Xdmf 2.1.1 gcc 11.2 failed."
        return 1
    fi

    return 0;
}

function apply_xdmf_patch
{
    if [[ ${XDMF_VERSION} == 2.1.1 ]] ; then
        apply_xdmf_patch1
        if [[ $? != 0 ]] ; then
            return 1
        fi

        if [[ "$OPSYS" == "Darwin" ]] ; then
                XCODE_VERSION="$(/usr/bin/xcodebuild -version)"
                # this will catch Xcode 10 +, we don't have to worry about
                # XCode 1, it shouldn't be in the wild and even if it was
                # zero hope that current bv stack will build using
                if [[ "$XCODE_VERSION" == "Xcode 9"* ||
                      "$XCODE_VERSION" == "Xcode 1"* ]] ; then
                    apply_xdmf_osx_patch
                    if [[ $? != 0 ]] ; then
                        return 1
                    fi
                fi
        fi

        if [[ "$OPSYS" == "Linux" ]]; then
            if [[ "$C_COMPILER" == "gcc" ]]; then
                apply_xdmf_gcc_11_2_patch
                if [[ $? != 0 ]] ; then
                    return 1
                fi
            fi
        fi
    fi

    return 0;
}

function build_xdmf
{
    CMAKE_BIN="${CMAKE_COMMAND}"

    #
    # Prepare build dir
    #
    prepare_build_dir $XDMF_BUILD_DIR $XDMF_FILE
    untarred_xdmf=$?
    if [[ $untarred_xdmf == -1 ]] ; then
        warn "Unable to prepare Xdmf Build Directory. Giving up"
        return 1
    fi

    #
    # Apply patches
    #
    info "Patching Xdmf . . ."
    apply_xdmf_patch
    if [[ $? != 0 ]] ; then
        if [[ $untarred_xdmf == 1 ]] ; then
            warn "Giving up on Xdmf build because the patch failed."
            return 1
        else
            warn "Patch failed, but continuing.  I believe that this script\n" \
                 "tried to apply a patch to an existing directory that had\n" \
                 "already been patched ... that is, the patch is\n" \
                 "failing harmlessly on a second application."
        fi
    fi

    cd $XDMF_BUILD_DIR || error "Can't cd to Xdmf build dir."
    rm -f CMakeCache.txt #remove any CMakeCache that may have existed

    #
    # Configure Xdmf
    #
    info "Executing CMake on Xdmf"
    if [[ "$DO_STATIC_BUILD" == "yes" ]]; then
        XDMF_SHARED_LIBS="OFF"
        LIBEXT="a"
    else
        XDMF_SHARED_LIBS="ON"
        LIBEXT="${SO_EXT}"
    fi

    if [[ "$DO_VTK9" == "yes" ]] ; then
        xmlinc=$VISITDIR/${VTK_INSTALL_DIR}/$VTK_VERSION/$VISITARCH/include/vtk-${VTK_SHORT_VERSION}/vtklibxml2/include
        if test -e $VISITDIR/${VTK_INSTALL_DIR}/$VTK_VERSION/$VISITARCH/lib64 ; then
            xmllib=$VISITDIR/${VTK_INSTALL_DIR}/$VTK_VERSION/$VISITARCH/lib64/libvtklibxml2-${VTK_SHORT_VERSION}.${SO_EXT}
        else
            xmllib=$VISITDIR/${VTK_INSTALL_DIR}/$VTK_VERSION/$VISITARCH/lib/libvtklibxml2-${VTK_SHORT_VERSION}.${SO_EXT}
        fi
    else
        xmlinc=$VISITDIR/${VTK_INSTALL_DIR}/$VTK_VERSION/$VISITARCH/include/vtk-${VTK_SHORT_VERSION}/vtklibxml2
        xmllib=$VISITDIR/${VTK_INSTALL_DIR}/$VTK_VERSION/$VISITARCH/lib/libvtklibxml2-${VTK_SHORT_VERSION}.${SO_EXT}
    fi

    ${CMAKE_BIN} -DCMAKE_INSTALL_PREFIX:PATH="$VISITDIR/Xdmf/${XDMF_VERSION}/${VISITARCH}"\
                 -DCMAKE_BUILD_TYPE:STRING="${VISIT_BUILD_MODE}" \
                 -DCMAKE_BUILD_WITH_INSTALL_RPATH:BOOL=ON \
                 -DBUILD_SHARED_LIBS:BOOL=${XDMF_SHARED_LIBS}\
                 -DCMAKE_CXX_FLAGS:STRING="${CXXFLAGS} ${CXX_OPT_FLAGS}"\
                 -DCMAKE_CXX_COMPILER:STRING=${CXX_COMPILER}\
                 -DCMAKE_C_FLAGS:STRING="${CFLAGS} ${C_OPT_FLAGS}"\
                 -DCMAKE_C_COMPILER:STRING=${C_COMPILER}\
                 -DBUILD_TESTING:BOOL=OFF \
                 -DXDMF_BUILD_MPI:BOOL=OFF \
                 -DXDMF_BUILD_VTK:BOOL=OFF \
                 -DXDMF_BUILD_UTILS:BOOL=OFF \
                 -DXDMF_SYSTEM_HDF5:BOOL=ON \
                 -DHDF5_INCLUDE_PATH:PATH="$VISITDIR/hdf5/$HDF5_VERSION/$VISITARCH/include" \
                 -DHDF5_LIBRARY:FILEPATH="$VISITDIR/hdf5/$HDF5_VERSION/$VISITARCH/lib/libhdf5.${SO_EXT}" \
                 -DXDMF_SYSTEM_ZLIB:BOOL=ON \
                 -DZLIB_INCLUDE_DIR:PATH=${ZLIB_INCLUDE_DIR} \
                 -DZLIB_LIBRARY:FILEPATH=${ZLIB_LIBRARY} \
                 -DXDMF_SYSTEM_LIBXML2:BOOL=ON \
                 -DLIBXML2_INCLUDE_PATH:PATH="${xmlinc}" \
                 -DLIBXML2_LIBRARY:FILEPATH="${xmllib}" \
                 .

    if [[ $? != 0 ]] ; then
        warn "Xdmf configure failed.  Giving up"
        return 1
    fi

    #
    # Build Xdmf
    #
    info "Building Xdmf . . . (~3 minutes)"

    $MAKE
    if [[ $? != 0 ]] ; then
        warn "Xdmf build failed.  Giving up"
        return 1
    fi

    # Install Xdmf
    info "Installing Xdmf"
    $MAKE install
    if [[ $? != 0 ]] ; then
        warn "Xdmf install failed.  Giving up"
        return 1
    fi

    if [[ "$DO_STATIC_BUILD" != "yes" && "$OPSYS" == "Darwin" ]]; then
        LIBDIR="$VISITDIR/Xdmf/${XDMF_VERSION}/${VISITARCH}/lib"
        install_name_tool -id $LIBDIR/libXdmf.dylib $LIBDIR/libXdmf.dylib
    fi


    if [[ "$DO_GROUP" == "yes" ]] ; then
        chmod -R ug+w,a+rX "$VISITDIR/Xdmf"
        chgrp -R ${GROUP} "$VISITDIR/Xdmf"
    fi

    cd "$START_DIR"
    info "Done with Xdmf"
    return 0
}

function bv_xdmf_is_enabled
{
    if [[ $DO_XDMF == "yes" ]]; then
        return 1
    fi
    return 0
}

function bv_xdmf_is_installed
{
    check_if_installed "Xdmf" $XDMF_VERSION
    if [[ $? == 0 ]] ; then
        return 1
    fi
    return 0
}

function bv_xdmf_build
{
    cd "$START_DIR"
    if [[ "$DO_XDMF" == "yes" ]] ; then
        check_if_installed "Xdmf" $XDMF_VERSION
        if [[ $? == 0 ]] ; then
            info "Skipping Xdmf build.  Xdmf is already installed."
        else
            info "Building Xdmf (~2 minutes)"
            build_xdmf
            if [[ $? != 0 ]] ; then
                error "Unable to build or install Xdmf.  Bailing out."
            fi
            info "Done building Xdmf"
        fi
    fi
}
