function bv_visit_initialize
{
    export DO_VISIT="yes"
}

function bv_visit_enable
{ 
    DO_VISIT="yes"
}

function bv_visit_disable
{
    DO_VISIT="no"
}

function bv_visit_depends_on
{
    echo ""
}

function bv_visit_info
{
    if [[ "$USE_VISIT_FILE" == "yes" ]] ; then
        export VISIT_MD5_CHECKSUM=""
        export VISIT_SHA256_CHECKSUM=""
    else
        export VISIT_MD5_CHECKSUM="edccd6d6c289356ac1462b1606b10ef9"
        export VISIT_SHA256_CHECKSUM="40c33f08de7a048fb436b8a72156b9e5303434e8e52d5d8590c7dc3ce8ac607d"
    fi
}

function bv_visit_print
{
    printf "%s%s\n" "VISIT_FILE=" "${VISIT_FILE}"
    printf "%s%s\n" "VISIT_VERSION=" "${VISIT_VERSION}"
}

function bv_visit_print_usage
{
    printf "%-20s %s [%s]\n" "--visit"   "Build VisIt" "$DO_VISIT"
}

function bv_visit_host_profile
{
    if [[ "$DO_VISIT" == "yes" ]] ; then
        echo >> $HOSTCONF
        echo "##" >> $HOSTCONF
        echo "## VISIT" >> $HOSTCONF
    fi
}

function bv_visit_ensure_built_or_ready
{
    # Check-out the latest git sources, before building VisIt
    if [[ "$DO_GIT" == "yes" && "$USE_VISIT_FILE" == "no" ]] ; then
        if [[ -d visit ]] ; then
            info "Found existing GIT visit directory, using that . . ."
        else
            # Print a dialog screen
            info "GIT clone of visit ($GIT_ROOT_PATH) . . ."
            if [[ "$DO_REVISION" == "yes" && "$GITREVISION" != "" ]] ; then
                # Get the specified revision.
                git clone $GIT_ROOT_PATH
                cd visit
                git checkout $GITREVISION
                cd ..
            elif [[ "$TRUNK_BUILD" == "yes" ]] ; then
                # Get the trunk version
                git clone $GIT_ROOT_PATH
            elif [[ "$RC_BUILD" == "yes" ]] ; then
                # Get the RC version
                git clone $GIT_ROOT_PATH
                cd visit
                git checkout ${VISIT_VERSION:0:3}RC
                cd ..
            elif [[ "$TAGGED_BUILD" == "yes" ]] ; then
                # Get the tagged version
                git clone $GIT_ROOT_PATH
                cd visit
                git checkout v${VISIT_VERSION}
                cd ..
            fi
            if [[ $? != 0 ]] ; then
                warn "Unable to build VisIt. GIT clone failed."
                return 1
            fi
        fi

    # Build using (the assumed) existing GIT "visit" directory
    elif [[ -d visit ]] ; then
        info "Found existing GIT visit directory, using that . . ."
        #resetting any values that have mixup the build between Trunk and RC
        VISIT_FILE="" #erase any accidental setting of these values
        USE_VISIT_FILE="no"
        DO_GIT="yes" #if visit directory exists it may have come from git.

    # Build using a VisIt source tarball
    else
        if [[ -e ${VISIT_FILE%.gz} || -e ${VISIT_FILE} ]] ; then
            info \
                "Got VisIt source code. Let's look for 3rd party libraries."
        else
            download_file $VISIT_FILE
            if [[ $? != 0 ]] ; then
                warn \
                    "Unable to build VisIt. Can't find source code: ${VISIT_FILE}."
                return 1
            fi
        fi
    fi
}

function bv_visit_dry_run
{
    if [[ "$DO_VISIT" == "yes" ]] ; then
        echo "Dry run option not set for VisIt"
    fi
}


function bv_visit_print_build_command
{
    echo "visit has no build commands set"
}

function bv_visit_modify_makefiles
{
    # NOTE: We are inside the VisIt src directory when this function is called.

    if [[ "$OPSYS" == "Darwin" ]]; then
        # Check for version < 8.0.0 (MacOS 10.4, Tiger) for gcc < 4.x
        VER=$(uname -r)
        if (( ${VER%%.*} > 8 )) ; then
            cat databases/Shapefile/Makefile | \
                sed '/LDFLAGS/s/$/ -Wl,-dylib_file,\/System\/Library\/Frameworks\/OpenGL.framework\/Versions\/A\/Libraries\/libGLU.dylib:\/System\/Library\/Frameworks\/OpenGL.framework\/Versions\/A\/Libraries\/libGLU.dylib/' > Make.tmp
            mv -f databases/Shapefile/Makefile databases/Shapefile/Makefile.orig
            mv -f Make.tmp databases/Shapefile/Makefile
        fi 
        if (( ${VER%%.*} < 8 )) ; then
            info "Patching VisIt . . ."
            cat databases/Fluent/Makefile | sed '/CXXFLAGS/s/$/ -O0/g' > Make.tmp
            mv -f databases/Fluent/Makefile databases/Fluent/Makefile.orig
            mv -f Make.tmp databases/Fluent/Makefile
            cat avt/Pipeline/Data/avtCommonDataFunctions.C | \
                sed '/isfinite/s/isfinite/__isfinited/g' > C.tmp
            mv -f avt/Pipeline/Data/avtCommonDataFunctions.C \
               avt/Pipeline/Data/avtCommonDataFunctions.C.orig
            mv -f C.tmp avt/Pipeline/Data/avtCommonDataFunctions.C
            cat avt/Expressions/Abstract/avtExpressionFilter.C | \
                sed '/isfinite/s/isfinite/__isfinited/g' > C.tmp
            mv -f avt/Expressions/Abstract/avtExpressionFilter.C \
               avt/Expressions/Abstract/avtExpressionFilter.C.orig
            mv -f C.tmp avt/Expressions/Abstract/avtExpressionFilter.C
        fi
        if (( ${VER%%.*} < 7 )) ; then
            cat third_party_builtin/mesa_stub/Makefile | \
                sed 's/glx.c glxext.c//' > Make.tmp
            mv -f third_party_builtin/mesa_stub/Makefile \
               third_party_builtin/mesa_stub/Makefile.orig
            mv -f Make.tmp third_party_builtin/mesa_stub/Makefile
        fi
        if (( ${VER%%.*} > 6 )) ; then
            cat avt/Expressions/Makefile | \
                sed '/LDFLAGS/s/$/ -Wl,-undefined,dynamic_lookup/g' > Make.tmp
            mv -f avt/Expressions/Makefile \
               avt/Expressions/Makefile.orig
            mv -f Make.tmp avt/Expressions/Makefile
        else
            cat avt/Expressions/Makefile | \
                sed '/LDFLAGS/s/$/ -Wl,-flat_namespace,-undefined,suppress/g' > \
                    Make.tmp
            mv -f avt/Expressions/Makefile \
               avt/Expressions/Makefile.orig
            mv -f Make.tmp avt/Expressions/Makefile
        fi
    fi

    if [[ "$BUILD_VISIT_BGQ" == "yes" ]] ; then
        # Filter the engine link line so it will not include X11 libraries. CMake is adding
        # them even though we don't want them. Also get rid of extra static/dynamic 
        # link keywords that prevent the linker from making a good static executable.
        for target in engine_ser_exe.dir engine_par_exe.dir
        do
            edir="engine/main/CMakeFiles/$target"
            if test -e "$edir/link.txt" ; then
                sed "s/-lX11//g" $edir/link.txt > $edir/link1.txt
                sed "s/-lXext//g" $edir/link1.txt > $edir/link2.txt
                sed "s/-Wl,-Bstatic//g" $edir/link2.txt > $edir/link3.txt
                sed "s/-Wl,-Bdynamic//g" $edir/link3.txt > $edir/link4.txt
                rm -f $edir/link1.txt $edir/link2.txt $edir/link3.txt
                mv $edir/link4.txt $edir/link.txt
            else
                echo "***** DID NOT SEE: $edir/link.txt   pwd=`pwd`"
            fi
            if test -e "$edir/relink.txt" ; then
                sed "s/-lX11//g" $edir/relink.txt > $edir/relink1.txt
                sed "s/-lXext//g" $edir/relink1.txt > $edir/relink2.txt
                sed "s/-Wl,-Bstatic//g" $edir/relink2.txt > $edir/relink3.txt
                sed "s/-Wl,-Bdynamic//g" $edir/relink3.txt > $edir/relink4.txt
                rm -f $edir/relink1.txt $edir/relink2.txt $edir/relink3.txt
                mv $edir/relink4.txt $edir/relink.txt
            else
                echo "***** DID NOT SEE: $edir/relink.txt   pwd=`pwd`"
            fi
        done
        # Filter the visitconvert link line so it will not include X11 libraries. CMake 
        # is adding them even though we don't want them. Also get rid of extra static/dynamic 
        # link keywords that prevent the linker from making a good static executable.
        for target in visitconvert_ser.dir visitconvert_par.dir
        do
            edir="tools/convert/CMakeFiles/$target"
            if test -e "$edir/link.txt" ; then
                sed "s/-lX11//g" $edir/link.txt > $edir/link1.txt
                sed "s/-lXext//g" $edir/link1.txt > $edir/link2.txt
                sed "s/-Wl,-Bstatic//g" $edir/link2.txt > $edir/link3.txt
                sed "s/-Wl,-Bdynamic//g" $edir/link3.txt > $edir/link4.txt
                rm -f $edir/link1.txt $edir/link2.txt $edir/link3.txt
                mv $edir/link4.txt $edir/link.txt
            else
                echo "***** DID NOT SEE: $edir/link.txt   pwd=`pwd`"
            fi
            if test -e "$edir/relink.txt" ; then
                sed "s/-lX11//g" $edir/relink.txt > $edir/relink1.txt
                sed "s/-lXext//g" $edir/relink1.txt > $edir/relink2.txt
                sed "s/-Wl,-Bstatic//g" $edir/relink2.txt > $edir/relink3.txt
                sed "s/-Wl,-Bdynamic//g" $edir/relink3.txt > $edir/relink4.txt
                rm -f $edir/relink1.txt $edir/relink2.txt $edir/relink3.txt
                mv $edir/relink4.txt $edir/relink.txt
            else
                echo "***** DID NOT SEE: $edir/relink.txt   pwd=`pwd`"
            fi
        done
    fi

    return 0
}

# *************************************************************************** #
#                          Function 9.1, build_visit                          #
# *************************************************************************** #

function build_visit
{
    if [[ "$DO_GIT" != "yes" || "$USE_VISIT_FILE" == "yes" ]] ; then
        #
        # Unzip the file, provided a gzipped file exists.
        #
        if [[ -f ${VISIT_FILE} ]] ; then
            info "Unzipping/untarring ${VISIT_FILE} . . ."
            uncompress_untar ${VISIT_FILE}
            if [[ $? != 0 ]] ; then
                warn \
                    "Unable to untar ${VISIT_FILE}.  Corrupted file or out of space on device?"
                return 1
            fi
        elif [[ -f ${VISIT_FILE%.*} ]] ; then
            info "Unzipping ${VISIT_FILE%.*} . . ."
            $TAR xf ${VISIT_FILE%.*}
            if [[ $? != 0 ]] ; then
                warn  \
                    "Unable to untar ${VISIT_FILE%.*}.  Corrupted file or out of space on device?"
                return 1
            fi
        fi
    fi

    #
    # Set up the VisIt build dir which is a sibling to the VisIt src dir
    #
    if [[ "$DO_GIT" == "yes" && "$USE_VISIT_FILE" == "no" ]] ; then
        VISIT_BUILD_DIR="visit/build"
    else
        VISIT_BUILD_DIR="${VISIT_FILE%.tar*}/build"
    fi

    if [[ ! -e $VISIT_BUILD_DIR ]] ; then
        mkdir $VISIT_BUILD_DIR || error "Can't make VisIt build dir."
    else
        rm -rf $VISIT_BUILD_DIR/* || error "Can't clean VisIt build dir."
    fi

    info "Building VisIt in ${VISIT_BUILD_DIR} . . ."
    
    cd $VISIT_BUILD_DIR

    #
    # Create the GIT_VERSION file.
    #
    if [[ "$DO_GIT" == "yes" && "$USE_VISIT_FILE" == "no" ]] ; then
        git log -1 | grep "^commit" | cut -d' ' -f2 | head -c 7 > ../src/GIT_VERSION
    fi

    #
    # Set up the config-site file, which gives configure the information it
    # needs about the third party libraries.
    #

    # No real need to do this as it is defined on the cmake line BUT
    # Users may rebuild visit with updated git
    cp ${START_DIR}/${HOSTCONF} config-site

    #
    # Call cmake
    # 
    info "Configuring VisIt . . ."
    FEATURES="-DVISIT_CONFIG_SITE:FILEPATH=${START_DIR}/${HOSTCONF}"
    FEATURES="${FEATURES} -DVISIT_INSTALL_THIRD_PARTY:BOOL=ON"
    if [[ "$parallel" == "yes" ]] ; then
        FEATURES="${FEATURES} -DVISIT_PARALLEL:BOOL=ON"
    fi
    FEATURES="${FEATURES} -DCMAKE_BUILD_TYPE:STRING=${VISIT_BUILD_MODE}"
    FEATURES="${FEATURES} -DVISIT_C_COMPILER:FILEPATH=${C_COMPILER}"
    FEATURES="${FEATURES} -DVISIT_CXX_COMPILER:FILEPATH=${CXX_COMPILER}"

    if test -n "${CFLAGS}" || test -n "${C_OPT_FLAGS}" ; then
        FEATURES="${FEATURES} -DVISIT_C_FLAGS:STRING=\"${CFLAGS} ${C_OPT_FLAGS}\""
    fi
    if [[ "$parallel" == "yes" ]] ; then
        CXXFLAGS="$CXXFLAGS $PAR_INCLUDE"
    fi
    if test -n "${CXXFLAGS}" || test -n "${CXX_OPT_FLAGS}" ; then
        FEATURES="${FEATURES} -DVISIT_CXX_FLAGS:STRING=\"${CXXFLAGS} ${CXX_OPT_FLAGS}\""
    fi
    if [[ "${DO_JAVA}" == "yes" ]] ; then
        FEATURES="${FEATURES} -DVISIT_JAVA:BOOL=ON"
    fi
    if [[ "${VISIT_INSTALL_PREFIX}" != "" ]] ; then
        FEATURES="${FEATURES} -DCMAKE_INSTALL_PREFIX:PATH=${VISIT_INSTALL_PREFIX}"
        FEATURES="${FEATURES} -DCPACK_INSTALL_PREFIX:PATH=${VISIT_INSTALL_PREFIX}"
        FEATURES="${FEATURES} -DCPACK_PACKAGING_INSTALL_PREFIX:PATH=${VISIT_INSTALL_PREFIX}"
    fi
    # Select a specialized build mode.
    if [[ "${DO_DBIO_ONLY}" == "yes" ]] ; then
        FEATURES="${FEATURES} -DVISIT_DBIO_ONLY:BOOL=ON"
    elif [[ "${DO_ENGINE_ONLY}" = "yes" ]] ; then
        FEATURES="${FEATURES} -DVISIT_ENGINE_ONLY:BOOL=ON"
    elif [[ "${DO_SERVER_COMPONENTS_ONLY}" = "yes" ]] ; then
        FEATURES="${FEATURES} -DVISIT_SERVER_COMPONENTS_ONLY:BOOL=ON"
    fi

    # Let the user turn on XDB.
    if [[ "${DO_XDB}" == "yes" ]] ; then
        FEATURES="${FEATURES} -DVISIT_ENABLE_XDB:BOOL=ON"
    fi

    # Let the user pick a subset of plugins.
    if [[ "${VISIT_SELECTED_DATABASE_PLUGINS}" != "" ]] ; then
        FEATURES="${FEATURES} -DVISIT_SELECTED_DATABASE_PLUGINS:STRING=${VISIT_SELECTED_DATABASE_PLUGINS}"
    fi
    CMAKE_INSTALL=${CMAKE_INSTALL:-"$VISITDIR/cmake/${CMAKE_VERSION}/$VISITARCH/bin"}
    CMAKE_BIN="${CMAKE_INSTALL}/cmake"
    rm -f CMakeCache.txt

    if [[ "${CREATE_RPM}" == "yes" ]]; then
        sed -i "s/SET(CPACK_GENERATOR \"TGZ\")/#SET(CPACK_GENERATOR \"TGZ\")/" CMakeLists.txt
        FEATURES="${FEATURES} -DCPACK_BINARY_RPM:BOOL=ON -DCPACK_GENERATOR:STRING=\"RPM;TGZ\""
        FEATURES="${FEATURES} -DCPACK_RPM_SPEC_MORE_DEFINE:STRING=\"%global_python_bytecompile_errors_terminate_build 0\""
    fi

    issue_command "${CMAKE_BIN}" ${FEATURES} ../src

    if [[ $? != 0 ]] ; then
        echo "VisIt configure failed.  Giving up"
        return 1
    fi

    #
    # Some platforms like to modify the generated Makefiles.
    #
    bv_visit_modify_makefiles

    #
    # Build VisIt
    #
    info "Building VisIt . . . (~50 minutes)"
    $MAKE $MAKE_OPT_FLAGS
    if [[ $? != 0 ]] ; then
        warn "VisIt build failed.  Giving up"
        return 1
    fi
    warn "All indications are that VisIt successfully built."

    #
    # Install VisIt
    #
    if [[ "${VISIT_INSTALL_PREFIX}" != "" ]] ; then
        $MAKE $MAKE_OPT_FLAGS install
        if [[ $? != 0 ]] ; then
            warn "VisIt installation failed.  Giving up"
            return 1
        fi
        warn "All indications are that VisIt successfully installed."
    fi

    #
    # Major hack here. Mark M. should really pull this total hack out of
    # this script. It is here to make the visitconvert tool be called
    # imeshio to satisfy needs of ITAPS SciDAC project.
    #
    if [[ "${DO_DBIO_ONLY}" == "yes" && "$0" == "build_imeshio" ]] ; then
        if [[ -e exe/visitconvert_ser_lite ]]; then
            cp exe/visitconvert_ser_lite exe/imeshioconvert
            cp bin/visitconvert bin/imeshioconvert
        fi
    fi
}

function bv_visit_is_enabled
{
    if [[ $DO_VISIT == "yes" ]]; then
        return 1    
    fi
    return 0
}

function bv_visit_is_installed
{
    #always return false?
    return 0
}

function bv_visit_build
{
    #
    # Build the actual VisIt code
    #

    if [[ "$DO_VISIT" == "yes" ]] ; then
        cd "$START_DIR"
        info "Building VisIt (~50 minutes)"
        build_visit
        if [[ $? != 0 ]] ; then
            error "Unable to build or install VisIt.  Bailing out."
        fi

        #
        # Output the message indicating that we are finished.
        #
        info "Finished building VisIt."
        info
        info "You many now try to run VisIt by cd'ing into the"
        info "$VISIT_BUILD_DIR/bin directory and invoking \"visit\""
        info
        info "To create a binary distribution tarball from this build, cd to"
        info "${START_DIR}/${VISIT_BUILD_DIR}"
        info "then enter: \"make package\""
        info
        info "This will produce a tarball called visitVERSION.ARCH.tar.gz, where"
        info "VERSION is the version number, and ARCH is the OS architecure."
        info
        info "To install the above tarball in a directory called \"INSTALL_DIR_PATH\""
        info "enter: tools/dev/scripts/visit-install VERSION ARCH INSTALL_DIR_PATH"
        info
        info "If you run into problems, contact visit-users@ornl.gov."
    else
        if [[ $ANY_ERRORS == "no" ]] ; then
            info "Finished!"
        else
            info "Finished with Errors"
        fi
    fi
}
