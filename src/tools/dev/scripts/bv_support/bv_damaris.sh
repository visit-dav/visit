function bv_damaris_initialize
{
    export DO_DAMARIS="no"
}

function bv_damaris_enable
{ 
    DO_DAMARIS="yes"
}

function bv_damaris_disable
{
    DO_DAMARIS="no"
}

function bv_damaris_depends_on
{
    if [[ "$DO_MPICH" == "yes" ]] ; then
        echo "cmake xsd xercesc boost mpich"
    else
        echo "cmake xsd xercesc boost"        
    fi

}

function bv_damaris_info
{
    export DAMARIS_VERSION=${DAMARIS_VERSION:-"1.0.1"}
    export DAMARIS_FILE=${DAMARIS_FILE:-"damaris-${DAMARIS_VERSION}.tgz"}
    export DAMARIS_COMPATIBILITY_VERSION=${DAMARIS_COMPATIBILITY_VERSION:-"1.0"}
    export DAMARIS_URL=${DAMARIS_URL:-"https://gforge.inria.fr/frs/download.php/file/35204"}
    export DAMARIS_BUILD_DIR=${DAMARIS_BUILD_DIR:-"damaris-${DAMARIS_VERSION}"}
    export DAMARIS_MD5_CHECKSUM=""
    export DAMARIS_SHA256_CHECKSUM=""
}

function bv_damaris_print
{
    printf "%s%s\n" "DAMARIS_FILE=" "${DAMARIS_FILE}"
    printf "%s%s\n" "DAMARIS_VERSION=" "${DAMARIS_VERSION}"
    printf "%s%s\n" "DAMARIS_COMPATIBILITY_VERSION=" "${DAMARIS_COMPATIBILITY_VERSION}"
    printf "%s%s\n" "DAMARIS_BUILD_DIR=" "${DAMARIS_BUILD_DIR}"
}

function bv_damaris_print_usage
{
    printf "%-20s %s [%s]\n" "--damaris"   "Build DAMARIS" "$DO_DAMARIS"
}

function bv_damaris_host_profile
{
    if [[ "$DO_DAMARIS" == "yes" ]] ; then
        echo "##" >> $HOSTCONF
        echo "## DAMARIS" >> $HOSTCONF
        echo "##" >> $HOSTCONF
        echo "VISIT_OPTION_DEFAULT(VISIT_DAMARIS_DIR \${VISITHOME}/damaris/$DAMARIS_VERSION/\${VISITARCH})" \
             >> $HOSTCONF
    fi
}

function bv_damaris_ensure
{
    if [[ "$DO_DAMARIS" == "yes" ]] ; then
        ensure_built_or_ready "damaris" $DAMARIS_VERSION $DAMARIS_BUILD_DIR $DAMARIS_FILE $DAMARIS_URL
        if [[ $? != 0 ]] ; then
            ANY_ERRORS="yes"
            DO_DAMARIS="no"
            error "Unable to build DAMARIS.  ${DAMARIS_FILE} not found."
        fi
    fi
}

function apply_damaris_patch
{
    return 0
}

function build_damaris
{
    # Checking that we have a parallel compiler
    if [ -z ${PAR_COMPILER+x} ]; then 
        warn "Damaris requires PAR_COMPILER to be set."
        warn "For instance, PAR_COMPILER=mpicxx"
        warn "Giving up."
        return 1
    fi 


    
    export PAR_CXX_COMPILER="${PAR_COMPILER}"
    

    #
    # The build dir of VisIt is required to get the headers of libsimV2
    # so we create it now. This is pretty much a copy-past of some
    # content from bv_visit.sh.
    #
    if [[ "$DO_GIT" != "yes" || "$USE_VISIT_FILE" == "yes" ]] ; then
        #
        # Unzip the file, provided a gzipped file exists.
        #
        if [[ -f ${VISIT_FILE} ]] ; then
            info "Unzipping/untarring ${VISIT_FILE} because Damaris needs it . . ."
            uncompress_untar ${VISIT_FILE}
            if [[ $? != 0 ]] ; then
                warn "Unable to untar ${VISIT_FILE}.  Corrupted file or out of space on device?"
                return 1
            fi
        elif [[ -f ${VISIT_FILE%.*} ]] ; then
            info "Unzipping ${VISIT_FILE%.*} because Damaris needs it . . ."
            $TAR xf ${VISIT_FILE%.*}
            if [[ $? != 0 ]] ; then
                warn "Unable to untar ${VISIT_FILE%.*}.  Corrupted file or out of space on device?"
                return 1
            fi
        fi
    fi

    local LIBSIMV2_INCLUDE="../${VISIT_FILE%.tar*}/src/sim/V2/lib"
    if [[ "$DO_GIT" == "yes" && "$USE_VISIT_FILE" == "no" ]] ; then
        LIBSIMV2_INCLUDE="../visit/src/sim/V2/lib"
    fi
    
    #
    # Prepare build dir for Damaris
    #
    prepare_build_dir $DAMARIS_BUILD_DIR $DAMARIS_FILE
    untarred_damaris=$?
    if [[ $untarred_damaris == -1 ]] ; then
        warn "Unable to prepare Damaris build directory. Giving Up!"
    fi
    cd $DAMARIS_BUILD_DIR

    #
    # Applying Patches
    #
    apply_damaris_patch
    if [[ $? != 0 ]] ; then
        warn "Unable to prepare Damaris build directory. Giving Up"
        return 1
    fi

    #
    # Calling Cmake
    #
    
    sh -c "$CMAKE_COMMAND -G \"Unix Makefiles\" \
    -DPAR_CXX_COMPILER:PATH=$PAR_CXX_COMPILER \
    -DCMAKE_INSTALL_PREFIX:PATH=$VISITDIR/damaris/$DAMARIS_VERSION/$VISITARCH \
    -DCMAKE_BUILD_TYPE:STRING=$VISIT_BUILD_MODE \
    -DVISIT_INCLUDE:PATH=$LIBSIMV2_INCLUDE \
    -DXERCESC_ROOT:PATH=$VISITDIR/xerces-c/$XERCESC_VERSION/$VISITARCH \
    -DXSD_ROOT:PATH=$VISITDIR/xsd/$XSD_VERSION/$VISITARCH \
    -DXSD_INCLUDE_DIR:PATH=$VISITDIR/xsd/$XSD_VERSION/$VISITARCH/include \
    -DBOOST_ROOT=$VISITDIR/boost/$BOOST_VERSION/$VISITARCH \
    -DBOOST_INCLUDEDIR=$VISITDIR/boost/$BOOST_VERSION/$VISITARCH/include \
    -DENABLE_EXAMPLES:BOOL=false \
    -DENABLE_TESTS:BOOL=false"

    if [[ $? != 0 ]] ; then
        warn "Cmake failed to create build files for Damaris.  Giving up."
        return 1
    fi

    #
    # Building Damaris
    #
    $MAKE

    if [[ $? != 0 ]] ; then
        warn "Failed to build Damaris.  Giving up."
        return 1
    fi

    #
    # Installing Damaris
    #
    $MAKE install

    if [[ $? != 0 ]] ; then
        warn "Failed to install Damaris.  Giving up."
        return 1
    fi

    return 0
}

function bv_damaris_is_enabled
{
    if [[ $DO_DAMARIS == "yes" ]]; then
        return 1
    fi
    return 0
}

function bv_damaris_is_installed
{
    check_if_installed "damaris" $DAMARIS_VERSION
    if [[ $? == 0 ]] ; then
        return 1
    fi
    return 0
}

function bv_damaris_build
{
    if [[ "$DO_DAMARIS" == "yes" ]] ; then
        check_if_installed "damaris" $DAMARIS_VERSION
        if [[ $? == 0 ]] ; then
            info "Skipping Damaris build.  Damaris is already installed."
        else
            info "Building Damaris (~6 minutes)"

            build_damaris
            if [[ $? != 0 ]] ; then
                error "Unable to build or install Damaris.  Bailing out."
            fi
            info "Done building Damaris"
        fi
    fi
}
