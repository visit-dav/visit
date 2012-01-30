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
    return ""
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

    export VISIT_VERSION=${VISIT_VERSION:-"2.4.1"}
    
    #export VISIT_COMPATIBILITY_VERSION=${VISIT_COMPATIBILITY_VERSION:-"2.4.1"}
    #export VISIT_BUILD_DIR=${VISIT_BUILD_DIR:-"visit"}

    ####
    # Trunk:
    ####
    export SVN_SOURCE_PATH="trunk/src"

    ###
    # Release:
    ###
    #export SVN_SOURCE_PATH="tags/${VISIT_VERSION}/src"
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
    # Set up environment variables for the configure step.
    #
    PARFLAGS=""
    if [[ "$parallel" == "yes" ]] ; then
       PARFLAGS="--enable-parallel"
       CXXFLAGS="$CXXFLAGS $PAR_INCLUDE"
    fi

    #
    # Set up the config-site file, which gives configure the information it
    # needs about the third party libraries.
    #
    if [[ "$DO_SVN" == "yes" && "$USE_VISIT_FILE" == "no" ]] ; then
        cd src
    else
        VISIT_DIR=${VISIT_FILE%.tar*}/src
        cd "${VISIT_DIR}"
    fi
    cp $START_DIR/$(hostname).cmake config-site

    #
    # Call configure
    # 
    info "Configuring VisIt . . ."
    EXTRA_FEATURES=""
    if [[ "${DO_MODULE}" == "yes" ]] ; then
       EXTRA_FEATURES="${EXTRA_FEATURES} --enable-visitmodule"
    fi
    if [[ "${DO_JAVA}" == "yes" ]] ; then
       EXTRA_FEATURES="${EXTRA_FEATURES} --enable-java"
    fi
    if [[ "${DO_SLIVR}" == "yes" ]] ; then
       EXTRA_FEATURES="${EXTRA_FEATURES} --enable-slivr"
    fi
    # A dbio-only build disables pretty much everything else.
    if [[ "${DO_DBIO_ONLY}" == "yes" ]] ; then
       EXTRA_FEATURES="--enable-dbio-only"
    else
       EXTRA_FEATURES="${EXTRA_FEATURES} --enable-install-thirdparty"
    fi
    ./configure ${PARFLAGS} ${EXTRA_FEATURES}
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
