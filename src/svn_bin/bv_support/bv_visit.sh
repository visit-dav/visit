# Module automatically read in from construct_build_visit
# Insert header and comments


#initialize all the variables
function bv_visit_initialize
{
    export DO_VISIT="yes"
    export ON_VISIT="on"
}

#enable the module for install
function bv_visit_enable
{ 
    DO_VISIT="yes"
    ON_VISIT="on"
}

#disable the module for install
function bv_visit_disable
{
    DO_VISIT="no"
    ON_VISIT="off"
}

#add any dependency with comma separation
function bv_visit_depends_on
{
    echo ""
}

#add information about how to get library..
function bv_visit_info
{

    ############################################################################
    # Path of VisIt src directory in svn.
    # For trunk use "trunk/src"
    # For versioned build_visit releases change this to point to the proper
    # release tag.
    ############################################################################

    export VISIT_VERSION=${VISIT_VERSION:-"2.5.1"}
    
    ####
    # Trunk:
    ####
    #export SVN_SOURCE_PATH="trunk/src"

    ###
    # Release:
    ###
    export SVN_SOURCE_PATH="tags/${VISIT_VERSION}/src"
}

#print variables used by this module
function bv_visit_print
{
    printf "%s%s\n" "VISIT_FILE=" "${VISIT_FILE}"
    printf "%s%s\n" "VISIT_VERSION=" "${VISIT_VERSION}"
    #printf "%s%s\n" "VISIT_COMPATIBILITY_VERSION=" "${VISIT_COMPATIBILITY_VERSION}"
    #printf "%s%s\n" "VISIT_BUILD_DIR=" "${VISIT_BUILD_DIR}"
}

#print how to install and uninstall module..
function bv_visit_print_usage
{
    printf "%-15s %s [%s]\n" "--visit"   "Build VisIt" "$DO_VISIT"
}

#how to show information for graphical editor
function bv_visit_graphical
{
    local graphical_out="VISIT    $VISIT_VERSION($VISIT_FILE)    $ON_VISIT"
    echo $graphical_out
}

#values to add to host profile, write to $HOSTCONF
function bv_visit_host_profile
{
    #Add code to write values to variable $HOSTCONF
    if [[ "$DO_VISIT" == "yes" ]] ; then
        echo >> $HOSTCONF
        echo "##" >> $HOSTCONF
        echo "## VISIT" >> $HOSTCONF
    fi
}

#prepare the module and check whether it is built or is ready to be built.
function bv_visit_ensure_built_or_ready
{
    # Check-out the latest svn sources, before building VisIt
    if [[ "$DO_SVN" == "yes" && "$USE_VISIT_FILE" == "no" ]] ; then
        if [[ -d src ]] ; then
           info "Found existing VisIt SVN src directory, using that . . ."
        else
           # Print a dialog screen
           info "SVN check-out of VisIt ($SVN_ROOT_PATH/$SVN_SOURCE_PATH) . . ."
           if [[ "$DO_REVISION" == "yes" && "$SVNREVISION" != "" ]] ; then
               svn co --quiet --non-interactive --revision "$SVNREVISION" \
                  $SVN_ROOT_PATH/$SVN_SOURCE_PATH
           else
               svn co --quiet --non-interactive $SVN_ROOT_PATH/$SVN_SOURCE_PATH
           fi
           if [[ $? != 0 ]] ; then
               warn "Unable to build VisIt. SVN download failed."
               return 1
           fi
        fi

    # Build using (the assumed) existing VisIt svn "src" directory
    elif [[ -d src ]] ; then
           info "Found VisIt SVN src directory found, using it."
           #resetting any values that have mixup the build between Trunk and RC
           VISIT_FILE="" #erase any accidental setting of these values
           USE_VISIT_FILE="no"
           ON_USE_VISIT_FILE="off"
           DO_SVN="yes" #if src directory exists it may have come from svn..

    # Build using a VisIt source tarball
    else
        if [[ -e ${VISIT_FILE%.gz} || -e ${VISIT_FILE} ]] ; then
            info \
"Got VisIt source code. Lets look for 3rd party libraries."
        else
            download_file $VISIT_FILE
            if [[ $? != 0 ]] ; then
               warn \
"Unable to build VisIt.  Can't find source code: ${VISIT_FILE}."
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


#print what the module will do for building
function bv_visit_print_build_command
{
   #print the build command..
   echo "visit has no build commands set"
}

# *************************************************************************** #
#                          Function 9.1, build_visit                          #
# *************************************************************************** #

function build_visit
{
    if [[ "$DO_SVN" != "yes" || "$USE_VISIT_FILE" == "yes" ]] ; then
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
    # Set up the config-site file, which gives configure the information it
    # needs about the third party libraries.
    #
    local VISIT_DIR="${VISIT_FILE%.tar*}/src"
    if [[ "$DO_SVN" == "yes" && "$USE_VISIT_FILE" == "no" ]] ; then
        VISIT_DIR="src" 
    else
        #visit2.5.0 needs a patch for ModelFit operator
        if [[ "${VISIT_FILE%.tar*}" == "visit2.5.0" ]]; then
            bv_patch_2_5_0
        fi
    fi
    
    if [[ "$DO_MANGLED_LIBRARIES" == "yes" ]]; then
        mangle_libraries "$VISIT_DIR" "mangled_$VISIT_DIR"
    
        if [[ $? == 0 ]]; then
            #TODO: fix and remove this
            #modify cmake to find library
            cd "mangled_$VISIT_DIR"
            mangle_file "$CMAKE_ROOT"/Modules/FindVTK.cmake CMake/FindMTK.cmake
patch -f -p0 <<\EOF
*** CMake/FindVisItMTK_tmp.cmake    2012-02-29 18:56:18.770322939 -0800
--- CMake/FindVisItMTK.cmake    2012-02-29 19:11:10.950323153 -0800
***************
*** 72,78 ****
  SET(MTK_DIR ${VISIT_MTK_DIR}/lib)
  
  MESSAGE(STATUS "Checking for MTK in ${MTK_DIR}")
! INCLUDE(${CMAKE_ROOT}/Modules/FindMTK.cmake)
  
  # Set the VisIt mangled mesa off of the MTK mangled mesa variable.
  IF("${MTK_USE_MANGLED_MESA}" STREQUAL "ON")
--- 72,78 ----
  SET(MTK_DIR ${VISIT_MTK_DIR}/lib)
  
  MESSAGE(STATUS "Checking for MTK in ${MTK_DIR}")
! INCLUDE(${VISIT_SOURCE_DIR}/CMake/FindMTK.cmake)
  
  # Set the VisIt mangled mesa off of the MTK mangled mesa variable.
  IF("${MTK_USE_MANGLED_MESA}" STREQUAL "ON")
EOF
            cd ..
            [[ $VISIT_DIR != "src" ]] && cd ..
            cp -R $VISIT_DIR/bin/shaders "mangled_${VISIT_DIR}/bin/shaders"
            VISIT_DIR="mangled_$VISIT_DIR"
        else
            error "Mangling VisIt failed"
            exit 0
        fi
    fi
    cd $VISIT_DIR
    cp $START_DIR/$(hostname).cmake config-site

    #
    # Call cmake
    # 
    info "Configuring VisIt . . ."
    FEATURES="-DVISIT_INSTALL_THIRD_PARTY:BOOL=ON"
    if [[ "$parallel" == "yes" ]] ; then
        FEATURES="${FEATURES} -DVISIT_PARALLEL:BOOL=ON"
    fi
    FEATURES="${FEATURES} -DCMAKE_BUILD_TYPE:STRING=${VISIT_BUILD_MODE}"
    FEATURES="${FEATURES} -DVISIT_C_COMPILER:FILEPATH=${C_COMPILER}"
    FEATURES="${FEATURES} -DVISIT_CXX_COMPILER:FILEPATH=${CXX_COMPILER}"
    
    FEATURES="${FEATURES} -DVISIT_C_FLAGS:STRING=\"${CFLAGS} ${C_OPT_FLAGS}\""
    if [[ "$parallel" == "yes" ]] ; then
        CXXFLAGS="$CXXFLAGS $PAR_INCLUDE"
    fi
    FEATURES="${FEATURES} -DVISIT_CXX_FLAGS:STRING=\"${CXXFLAGS} ${CXX_OPT_FLAGS}\""

    if [[ "${DO_MODULE}" == "yes" ]] ; then
       FEATURES="${FEATURES} -DVISIT_PYTHON_MODULE:BOOL=ON"
    fi
    if [[ "${DO_JAVA}" == "yes" ]] ; then
       FEATURES="${FEATURES} -DVISIT_JAVA:BOOL=ON"
    fi
    if [[ "${DO_SLIVR}" == "yes" ]] ; then
       FEATURES="${FEATURES} -DVISIT_SLIVR:BOOL=ON"
    fi
    if [[ "${VISIT_INSTALL_PREFIX}" != "" ]] ; then
       FEATURES="${FEATURES} -DCMAKE_INSTALL_PREFIX:PATH=${VISIT_INSTALL_PREFIX}"
    fi
    # Select a specialized build mode.
    if [[ "${DO_DBIO_ONLY}" == "yes" ]] ; then
       FEATURES="${FEATURES} -DVISIT_DBIO_ONLY:BOOL=ON"
    elif [[ "${DO_ENGINE_ONLY}" = "yes" ]] ; then
       FEATURES="${FEATURES} -DVISIT_ENGINE_ONLY:BOOL=ON"
    elif [[ "${DO_SERVER_COMPONENTS_ONLY}" = "yes" ]] ; then
       FEATURES="${FEATURES} -DVISIT_SERVER_COMPONENTS_ONLY:BOOL=ON"
    fi

    CMAKE_INSTALL=${CMAKE_INSTALL:-"$VISITDIR/cmake/${CMAKE_VERSION}/$VISITARCH/bin"}
    CMAKE_BIN="${CMAKE_INSTALL}/cmake"
    rm -f CMakeCache.txt
    issue_command "${CMAKE_BIN}" ${FEATURES} . 
    if [[ $? != 0 ]] ; then
       echo "VisIt configure failed.  Giving up"
       return 1
    fi

    if [[ "$OPSYS" == "Darwin" ]]; then
        # Check for version < 8.0.0 (MacOS 10.4, Tiger) for gcc < 4.x
        VER=$(uname -r)
        if (( ${VER%%.*} > 8 )) ; then
           cat databases/Shapefile/Makefile | \
              sed '/LDFLAGS/s/$/ -Wl,-dylib_file,\/System\/Library\/Frameworks\/OpenGL.framework\/Versions\/A\/Libraries\/libGLU.dylib:\/System\/Library\/Frameworks\/OpenGL.framework\/Versions\/A\/Libraries\/libGLU.dylib/' > Make.tmp
           mv -f databases/Shapefile/Makefile databases/Shapefile/Makefile.orig
           mv -f Make.tmp databases/Shapefile/Makefile
           if [[ "$DO_CCMIO" == "yes" ]] ; then
              cat databases/CCM/Makefile | \
                 sed '/LDFLAGS/s/$/ -Wl,-dylib_file,\/System\/Library\/Frameworks\/OpenGL.framework\/Versions\/A\/Libraries\/libGLU.dylib:\/System\/Library\/Frameworks\/OpenGL.framework\/Versions\/A\/Libraries\/libGLU.dylib/' > Make.tmp
              mv -f databases/CCM/Makefile databases/CCM/Makefile.orig
              mv -f Make.tmp databases/CCM/Makefile
           fi
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
           cat databases/SimV1/Makefile | \
              sed '/LDFLAGS/s/$/ -Wl,-undefined,dynamic_lookup/g' > Make.tmp
           mv -f databases/SimV1/Makefile databases/SimV1/Makefile.orig
           mv -f Make.tmp databases/SimV1/Makefile
           cat databases/SimV1Writer/Makefile | \
              sed '/LDFLAGS/s/$/ -Wl,-undefined,dynamic_lookup/g' > Make.tmp
           mv -f databases/SimV1Writer/Makefile \
             databases/SimV1Writer/Makefile.orig
           mv -f Make.tmp databases/SimV1Writer/Makefile
           cat avt/Expressions/Makefile | \
              sed '/LDFLAGS/s/$/ -Wl,-undefined,dynamic_lookup/g' > Make.tmp
           mv -f avt/Expressions/Makefile \
             avt/Expressions/Makefile.orig
           mv -f Make.tmp avt/Expressions/Makefile
        else
           cat databases/SimV1/Makefile | \
              sed '/LDFLAGS/s/$/ -Wl,-flat_namespace,-undefined,suppress/g' > \
              Make.tmp
           mv -f databases/SimV1/Makefile databases/SimV1/Makefile.orig
           mv -f Make.tmp databases/SimV1/Makefile
           cat databases/SimV1Writer/Makefile | \
              sed '/LDFLAGS/s/$/ -Wl,-flat_namespace,-undefined,suppress/g' > \
              Make.tmp
           mv -f databases/SimV1Writer/Makefile \
             databases/SimV1Writer/Makefile.orig
           mv -f Make.tmp databases/SimV1Writer/Makefile
           cat avt/Expressions/Makefile | \
              sed '/LDFLAGS/s/$/ -Wl,-flat_namespace,-undefined,suppress/g' > \
              Make.tmp
           mv -f avt/Expressions/Makefile \
             avt/Expressions/Makefile.orig
           mv -f Make.tmp avt/Expressions/Makefile
        fi
    elif [[ "$OPSYS" == "SunOS" ]]; then
        # Some Solaris systems hang when compiling Fluent when optimizations
        # are on.  Turn optimizations off.
        info "Patching VisIt . . ."
        cat databases/Fluent/Makefile | sed '/CXXFLAGS/s/$/ -O0/g' > Make.tmp
        mv -f databases/Fluent/Makefile databases/Fluent/Makefile.orig
        mv -f Make.tmp databases/Fluent/Makefile
    fi

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

function bv_patch_2_5_0
{

  if [[ -e visit2.5.0 ]]; then
    info "apply patch to ModelFit operator"
patch -f -p0 visit2.5.0/src/operators/ModelFit/CMakeLists.txt <<\EOF
24d23
< QT_WRAP_CPP(GModelFitOperator LIBG_SOURCES ${LIBG_MOC_SOURCES})
94a94
>     QT_WRAP_CPP(GModelFitOperator LIBG_SOURCES ${LIBG_MOC_SOURCES})
EOF
  fi

}

#the build command..
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

        if [[ "$DO_SVN" == "yes" && "$USE_VISIT_FILE" == "no" ]] ; then
            VISIT_SRC_DIR="src"
        else
            VISIT_SRC_DIR="${VISIT_FILE%.tar*}/src"
        fi

    FINISHED_MSG="Finished building VisIt.\n\n\
    You many now try to run VisIt by cd'ing into the \
    $VISIT_SRC_DIR/bin directory and invoking \"visit\".\n\n\
    To create a binary distribution tarball from this build, cd to \
    ${START_DIR}/${VISIT_SRC_DIR}\n\
    then enter: \"make package\"\n\n\
    This will produce a tarball called visitVERSION.ARCH.tar.gz, where \
    VERSION is the version number, and ARCH is the OS architecure.\n\n\
    To install the above tarball in a directory called \"INSTALL_DIR_PATH\",\
    enter: svn_bin/visit-install VERSION ARCH INSTALL_DIR_PATH\n\n\
    If you run into problems, contact visit-users@ornl.gov."

        #
        # Output the message indicating that we are finished. I couldn't get
        # the imbedded newlines in the FINISHED_MSG to display properly so I
        # am treating the graphical and console interfaces differently.
        #
        if test "${GRAPHICAL}" = "yes" ; then
            if [[ "$REDIRECT_ACTIVE" == "yes" ]] ; then
                info_box_large "$FINISHED_MSG" 1>&3
            else
                info_box_large "$FINISHED_MSG"
            fi
            log "Finished building VisIt."
            log
            log "You many now try to run VisIt by cd'ing into the"
            log "$VISIT_SRC_DIR/bin directory and invoking \"visit\""
            log
            log "To create a binary distribution tarball from this build, cd to"
            log "${START_DIR}/${VISIT_SRC_DIR}"
            log "then enter: \"make package\""
            log
            log "This will produce a tarball called visitVERSION.ARCH.tar.gz, where"
            log "VERSION is the version number, and ARCH is the OS architecure."
            log
            log "To install the above tarball in a directory called \"INSTALL_DIR_PATH\""
            log "enter: svn_bin/visit-install VERSION ARCH INSTALL_DIR_PATH"
            log
            log "If you run into problems, contact visit-users@ornl.gov."
        else
            info "Finished building VisIt."
            info
            info "You many now try to run VisIt by cd'ing into the"
            info "$VISIT_SRC_DIR/bin directory and invoking \"visit\""
            info
            info "To create a binary distribution tarball from this build, cd to"
            info "${START_DIR}/${VISIT_SRC_DIR}"
            info "then enter: \"make package\""
            info
            info "This will produce a tarball called visitVERSION.ARCH.tar.gz, where"
            info "VERSION is the version number, and ARCH is the OS architecure."
            info
            info "To install the above tarball in a directory called \"INSTALL_DIR_PATH\""
            info "enter: svn_bin/visit-install VERSION ARCH INSTALL_DIR_PATH"
            info
            info "If you run into problems, contact visit-users@ornl.gov."
        fi
    else
        if [[ $ANY_ERRORS == "no" ]] ; then
            info "Finished!"
        else
            info "Finished with Errors"
        fi
    fi

}
