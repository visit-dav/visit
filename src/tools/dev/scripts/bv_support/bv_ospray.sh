# Module automatically read in from construct_build_visit
# Insert header and comments
function bv_ospray_initialize
{
    export DO_OSPRAY="no"
    export USE_SYSTEM_OSPRAY="no"
    export OSPRAY_CONFIG_DIR=""
    add_extra_commandline_args "ospray" "alt-ospray-dir" 1 "Use alternative directory for ospray"
}

function bv_ospray_enable
{
    DO_OSPRAY="yes"
}

function bv_ospray_disable
{
    DO_OSPRAY="no"
}

function bv_ospray_alt_ospray_dir
{
    echo "Using alternate ospray directory"
    bv_ospray_enable
    USE_SYSTEM_OSPRAY="ospray"
    OSPRAY_CONFIG_DIR="$1"
}

function bv_ospray_check_openmp
{
    _OPENMP=$(echo | cpp -fopenmp -dM | grep -i open)
    if [[ "$_OPENMP" == "#define _OPENMP"* ]]; then
        return 0
    fi
    return -1
}

function bv_ospray_depends_on
{
    depends_on="cmake ispc embree"

    if [[ "$DO_TBB" == "yes" ]]; then
        depends_on="${depends_on} tbb"
    else
        bv_ospray_check_openmp
        if [[ $? == -1 ]]; then
            depends_on="${depends_on} tbb"
        fi
    fi

    echo ${depends_on}
}

function bv_ospray_info
{
    # versions
    export OSPRAY_VERSION=${OSPRAY_VERSION:-"1.6.1"}
    export OSPRAY_VISIT_MODULE_VERSION=${OSPRAY_VISIT_MODULE_VERSION:-"1.6.x"}
    
    # ospray source
    export OSPRAY_TARBALL=${OSPRAY_TARBALL:-"ospray-${OSPRAY_VERSION}.tar.gz"}
    export OSPRAY_BUILD_DIR=${OSPRAY_BUILD_DIR:-"ospray-${OSPRAY_VERSION}"}
    export OSPRAY_DOWNLOAD_URL=${OSPRAY_DOWNLOAD_URL:-"https://github.com/wilsonCernWq/module_visit/releases/download/v1.6.x"}

    # ospray module
    export OSPRAY_VISIT_MODULE_TARBALL=${OSPRAY_VISIT_MODULE_TARBALL:-"module_visit-${OSPRAY_VISIT_MODULE_VERSION}.zip"}
    export OSPRAY_VISIT_MODULE_UNTAR_DIR=${OSPRAY_VISIT_MODULE_UNTAR_DIR:-"module_visit-${OSPRAY_VISIT_MODULE_VERSION}"}
    export OSPRAY_VISIT_MODULE_BUILD_DIR=${OSPRAY_VISIT_MODULE_BUILD_DIR:-"${OSPRAY_BUILD_DIR}/modules/module_visit"}
    export OSPRAY_VISIT_MODULE_DOWNLOAD_URL=${OSPRAY_VISIT_MODULE_DOWNLOAD_URL:-"https://github.com/wilsonCernWq/module_visit/releases/download/v1.6.x"}
    export OSPRAY_MD5_CHECKSUM="58cfed6a24e8023389f63f65455466aa"
    export OSPRAY_SHA256_CHECKSUM="e080ca1161cbb987d889bb2ce308be7a38e0928afe7c9e952afd8273e29de432"
}

function bv_ospray_print
{
    print "%s%s\n" "OSPRAY_TARBALL=" "${OSPRAY_TARBALL}"
    print "%s%s\n" "OSPRAY_VERSION=" "${OSPRAY_VERSION}"
    print "%s%s\n" "OSPRAY_TARGET=" "${OSPRAY_TARGET}"
    print "%s%s\n" "OSPRAY_BUILD_DIR=" "${OSPRAY_BUILD_DIR}"
}

function bv_ospray_print_usage
{
    printf "%-20s %s [%s]\n" "--ospray" "Build OSPRay rendering support" "$DO_OSPRAY"
}

function bv_ospray_host_profile
{
    if [[ "$DO_OSPRAY" == "yes" ]]; then
        echo >> $HOSTCONF
        echo "##" >> $HOSTCONF
        echo "## OSPRay" >> $HOSTCONF
        echo "##" >> $HOSTCONF
        echo "VISIT_OPTION_DEFAULT(VISIT_OSPRAY ON TYPE BOOL)" >> $HOSTCONF
        if [[ "$USE_SYSTEM_OSPRAY" == "no" ]]; then
            echo "SETUP_APP_VERSION(OSPRAY ${OSPRAY_VERSION})" >> $HOSTCONF
            echo "VISIT_OPTION_DEFAULT(VISIT_OSPRAY_DIR \${VISITHOME}/ospray/$OSPRAY_VERSION/\${VISITARCH})" >> $HOSTCONF
        else
            local _tmp_=$(basename ${OSPRAY_CONFIG_DIR})
            echo "SETUP_APP_VERSION(OSPRAY ${_tmp_:7})" >> $HOSTCONF
            echo "VISIT_OPTION_DEFAULT(VISIT_OSPRAY_DIR ${OSPRAY_INSTALL_DIR})" >> $HOSTCONF
        fi
    fi
}

function bv_ospray_is_enabled
{
    if [[ $DO_OSPRAY == "yes" ]]; then
        return 1
    fi
    return 0
}

function bv_ospray_ensure
{
    if [[ "$DO_OSPRAY" == "yes" && "$USE_SYSTEM_OSPRAY" == "no" ]]; then
        ensure_built_or_ready "ospray" \
            $OSPRAY_VERSION \
            $OSPRAY_BUILD_DIR \
            $OSPRAY_TARBALL \
            $OSPRAY_DOWNLOAD_URL 
        if [[ $? != 0 ]] ; then
            return 1
        fi
        ensure_built_or_ready "ospray-visit-module" \
            $OSPRAY_VISIT_MODULE_VERSION \
            $OSPRAY_VISIT_MODULE_BUILD_DIR \
            $OSPRAY_VISIT_MODULE_TARBALL \
            $OSPRAY_VISIT_MODULE_DOWNLOAD_URL
        if [[ $? != 0 ]] ; then
            return 1
        fi       
    fi
}

function bv_ospray_initialize_vars
{
    info "initializing ospray vars"
    if [[ "$DO_OSPRAY" == "yes" ]]; then
        if [[ "$USE_SYSTEM_OSPRAY" == "no" ]]; then
            OSPRAY_INSTALL_DIR="${VISITDIR}/ospray/${OSPRAY_VERSION}/${VISITARCH}"
        else
            OSPRAY_INSTALL_DIR="${OSPRAY_CONFIG_DIR}/../../../"
        fi

        # Qi's Note: Are those variables necessary ?
        OSPRAY_INCLUDE_DIR="${OSPRAY_INSTALL_DIR}/include"
        if [[ -d $OSPRAY_INSTALL_DIR/lib64 ]]; then
            OSPRAY_LIB_DIR="${OSPRAY_INSTALL_DIR}/lib64"
        else
            OSPRAY_LIB_DIR="${OSPRAY_INSTALL_DIR}/lib"
        fi        
        OSPRAY_LIB="${OSPRAY_LIB_DIR}/libospray.so"            

        VTK_USE_OSPRAY="yes"
    fi
}

function bv_ospray_dry_run
{
    if [[ "$DO_OSPRAY" == "yes" ]] ; then
        echo "Dry run option not set for ospray."
    fi
}

function bv_ospray_is_installed
{
    if [[ "$USE_SYSTEM_OSPRAY" == "yes" ]]; then   
        return 1
    fi

    check_if_installed "ospray" $OSPRAY_VERSION
    if [[ $? == 0 ]] ; then
        return 1
    fi
    return 0
}

function build_ospray_in_source
{
    # set compiler if the user hasn't explicitly set CC and CXX
    if [ -z $CC ]; then
        echo "***NOTE: using compiler $C_COMPILER/$CXX_COMPILER!"
        export CC=$C_COMPILER
        export CXX=$CXX_COMPILER
    fi

    #### Build OSPRay ####
    mkdir -p build
    cd build

    # Clean out build directory to be sure we are doing a fresh build
    rm -rf *

    # set release and RPM settings
    info "Configure OSPRay . . . "
    CMAKE_INSTALL=${CMAKE_INSTALL:-"$VISITDIR/cmake/${CMAKE_VERSION}/$VISITARCH/bin"}

    CMAKE_VARS=""
    CMAKE_VARS=${CMAKE_VARS}" -D CMAKE_INSTALL_PREFIX=${OSPRAY_INSTALL_DIR} "
    CMAKE_VARS=${CMAKE_VARS}" -D OSPRAY_BUILD_ISA=ALL "
    CMAKE_VARS=${CMAKE_VARS}" -D OSPRAY_MODULE_VISIT=ON "
    CMAKE_VARS=${CMAKE_VARS}" -D OSPRAY_MODULE_MPI=OFF "
    CMAKE_VARS=${CMAKE_VARS}" -D OSPRAY_MODULE_MPI_APPS=OFF "
    CMAKE_VARS=${CMAKE_VARS}" -D OSPRAY_APPS_EXAMPLEVIEWER=OFF "
    CMAKE_VARS=${CMAKE_VARS}" -D OSPRAY_APPS_BENCHMARK=OFF "
    CMAKE_VARS=${CMAKE_VARS}" -D OSPRAY_SG_CHOMBO=OFF "
    CMAKE_VARS=${CMAKE_VARS}" -D OSPRAY_SG_OPENIMAGEIO=OFF "
    CMAKE_VARS=${CMAKE_VARS}" -D OSPRAY_SG_VTK=OFF "
    CMAKE_VARS=${CMAKE_VARS}" -D OSPRAY_ZIP_MODE=OFF "
    CMAKE_VARS=${CMAKE_VARS}" -D embree_DIR=${EMBREE_INSTALL_DIR} "
    CMAKE_VARS=${CMAKE_VARS}" -D ISPC_EXECUTABLE=${ISPC_INSTALL_DIR}/ispc "
    if [[ "${TBB_INSTALL_DIR}" == "" ]]; then
        bv_ospray_check_openmp
        if [[ $? == 0 ]]; then
            CMAKE_VARS=${CMAKE_VARS}" -D OSPRAY_TASKING_SYSTEM=OpenMP "
        else
            error "OSPRay cannot find neither TBB nor OpenMP."
        fi
    else
        CMAKE_VARS=${CMAKE_VARS}" -D TBB_ROOT=${TBB_INSTALL_DIR} "
    fi
    ${CMAKE_INSTALL}/cmake ${CMAKE_VARS} \
        .. || error "OSPRay did not configure correctly.  Giving up."

    #
    # Now build OSPRay
    #
    info "Building OSPRay (~10 minute)"
    env DYLD_LIBRARY_PATH=`pwd`/bin $MAKE $MAKE_OPT_FLAGS || \
        error "OSPRay did not build correctly.  Giving up."

    info "Installing OSPRay . . . "
    $MAKE install || error "OSPRay did not install correctly."
}

function build_ospray
{
    # prepare directories
    prepare_build_dir $OSPRAY_BUILD_DIR $OSPRAY_TARBALL
    untarred_ospray=$?
    if [[ $untarred_ospray == -1 ]]; then
        warn "Unable to prepare OSPRay build directory. Giving up!"
        return 1
    fi
    prepare_build_dir $OSPRAY_VISIT_MODULE_BUILD_DIR $OSPRAY_VISIT_MODULE_TARBALL
    untarred_ospray_visit_module=$?
    if [[ $untarred_ospray_visit_module == -1 ]]; then
        warn "Unable to prepare OSPRay build directory. Giving up!"
        return 1
    elif [[ $untarred_ospray_visit_module == 1 ]]; then
        rm -fr $OSPRAY_VISIT_MODULE_BUILD_DIR
        mv $OSPRAY_VISIT_MODULE_UNTAR_DIR $OSPRAY_VISIT_MODULE_BUILD_DIR \
            || error "Couldn't find module_visit for OSPRay"
    fi

    # build and install
    cd $OSPRAY_BUILD_DIR || error "Couldn't cd to OSPRay build dir."
    build_ospray_in_source

    # others
    if [[ "$DO_GROUP" == "yes" ]]; then
        chmod -R ug+w,a+rX "$VISITDIR/ospray"
        chgrp -R ${GROUP} "$VISITDIR/ospray"
    fi
    cd "$START_DIR"
    info "Done with OSPRay"
    return 0
}

function bv_ospray_build
{
    cd "$START_DIR"
    if [[ "$DO_OSPRAY" == "yes" && "$USE_SYSTEM_OSPRAY" == "no" ]]; then
        check_if_installed "ospray" $OSPRAY_VERSION
        if [[ $? == 0 ]] ; then
            info "Skipping OSPRay build. OSPRay is already installed."
        else
            build_ospray
            if [[ $? != 0 ]]; then
                error "Unable to build or install OSPRay. Bailing out."
            fi
        fi
    fi
}

