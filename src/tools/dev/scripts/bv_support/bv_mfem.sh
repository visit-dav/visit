function bv_mfem_initialize
{
    export DO_MFEM="no"
}

function bv_mfem_enable
{
    DO_MFEM="yes"
}

function bv_mfem_disable
{
    DO_MFEM="no"
}

function bv_mfem_depends_on
{
    local depends_on="zlib"

    if [[ "$DO_CONDUIT" == "yes" ]] ; then
        depends_on="$depends_on conduit"
    fi
    if [[ "$DO_FMS" == "yes" ]] ; then
        depends_on="$depends_on fms"
    fi
    if [[ "$DO_HDF5" == "yes" ]] ; then
        depends_on="$depends_on hdf5"
    fi

    echo $depends_on
}

function bv_mfem_info
{
    export MFEM_VERSION=${MFEM_VERSION:-"4.6"}
    export MFEM_FILE=${MFEM_FILE:-"mfem-${MFEM_VERSION}.tgz"}
    export MFEM_BUILD_DIR=${MFEM_BUILD_DIR:-"mfem-${MFEM_VERSION}"}
    export MFEM_URL=${MFEM_URL:-"https://bit.ly/mfem-4-6"}
    export MFEM_MD5_CHECKSUM="467f246903078e10cf52242ecf3ed1e9"
    export MFEM_SHA256_CHECKSUM="5fa9465b5bec56bfb777a4d2826fba48d85fbace4aed8b64a2fd4059bf075b15"
}

function bv_mfem_print
{
    printf "%s%s\n" "MFEM_FILE=" "${MFEM_FILE}"
    printf "%s%s\n" "MFEM_VERSION=" "${MFEM_VERSION}"
    printf "%s%s\n" "MFEM_BUILD_DIR=" "${MFEM_BUILD_DIR}"
}

function bv_mfem_print_usage
{
    printf "%-20s %s [%s]\n" "--mfem" "Build mfem support" "$DO_MFEM"
}

function bv_mfem_host_profile
{
    if [[ "$DO_MFEM" == "yes" ]] ; then
        echo >> $HOSTCONF
        echo "##" >> $HOSTCONF
        echo "## MFEM " >> $HOSTCONF
        echo "##" >> $HOSTCONF
        echo \
            "VISIT_OPTION_DEFAULT(VISIT_MFEM_DIR \${VISITHOME}/mfem/$MFEM_VERSION/\${VISITARCH})" \
            >> $HOSTCONF

        ZLIB_LIBDEP="\${VISITHOME}/zlib/\${ZLIB_VERSION}/\${VISITARCH}/lib z"

        CONDUIT_LIBDEP=""
        INCDEP=""
        if [[ "$DO_CONDUIT" == "yes" ]] ; then
            CONDUIT_LIBDEP="\${VISIT_CONDUIT_LIBDEP}"
            INCDEP="CONDUIT_INCLUDE_DIR"
        fi
        if [[ "$DO_FMS" == "yes" ]] ; then
            INCDEP="$INCDEP FMS_INCLUDE_DIR"
        fi

        if [[ "$INCDEP" != "" ]] ; then
             echo \
                "VISIT_OPTION_DEFAULT(VISIT_MFEM_INCDEP $INCDEP TYPE STRING)" \
                    >> $HOSTCONF
        fi
        echo \
            "VISIT_OPTION_DEFAULT(VISIT_MFEM_LIBDEP $CONDUIT_LIBDEP $ZLIB_LIBDEP TYPE STRING)" \
                >> $HOSTCONF
    fi
}

function bv_mfem_ensure
{
    if [[ "$DO_MFEM" == "yes" ]] ; then
        ensure_built_or_ready "mfem" $MFEM_VERSION $MFEM_BUILD_DIR $MFEM_FILE $MFEM_URL
        if [[ $? != 0 ]] ; then
            ANY_ERRORS="yes"
            DO_MFEM="no"
            error "Unable to build mfem.  ${MFEM_FILE} not found."
        fi
    fi
}

function apply_mfem_patch
{
    # On IBM PPC systems the system defines "__VSX__" but some of the
    # VSX functions are not defined with gcc, which VisIt typically
    # uses. To avoid this we disable all the VSX coding. This is ok
    # since VSX just optimizes performance, so no functionality is lost.
    patch -p0 << \EOF
diff -c ./linalg/simd/vsx128.hpp.orig ./linalg/simd/vsx128.hpp
*** ./linalg/simd/vsx128.hpp.orig	Tue Mar  9 06:55:45 2021
--- ./linalg/simd/vsx128.hpp	Tue Mar  9 06:57:37 2021
***************
*** 12,18 ****
  #ifndef MFEM_SIMD_VSX128_HPP
  #define MFEM_SIMD_VSX128_HPP
  
! #ifdef __VSX__
  
  #include "../../config/tconfig.hpp"
  #include <altivec.h>
--- 12,18 ----
  #ifndef MFEM_SIMD_VSX128_HPP
  #define MFEM_SIMD_VSX128_HPP
  
! #ifdef __VSX_NOMATCH__
  
  #include "../../config/tconfig.hpp"
  #include <altivec.h>
EOF
    if [[ $? != 0 ]] ; then
        warn "MFEM patch failed."
        return 1
    fi

    return 0;
}

# *************************************************************************** #
#                            Function 8, build_mfem
# *************************************************************************** #
function build_mfem
{
    #
    # Prepare build dir
    #
    prepare_build_dir $MFEM_BUILD_DIR $MFEM_FILE
    untarred_mfem=$?
    if [[ $untarred_mfem == -1 ]] ; then
        warn "Unable to prepare mfem build directory. Giving Up!"
        return 1
    fi

    #
    # Apply patches.
    #
    cd $MFEM_BUILD_DIR || error "Can't cd to mfem build dir."

    info "Patching MFEM"
    apply_mfem_patch
    if [[ $? != 0 ]] ; then
        if [[ $untarred_mfem == 1 ]] ; then
            warn "Giving up on MFEM build because the patch failed."
            return 1
        else
            warn "Patch failed, but continuing.  I believe that this script\n" \
                 "tried to apply a patch to an existing directory that had\n" \
                 "already been patched ... that is, the patch is\n" \
                 "failing harmlessly on a second application."
        fi
    fi

    #
    # Build MFEM.
    #
    mkdir build
    cd build || error "Can't cd to MFEM build dir."

    vopts="-DCMAKE_C_COMPILER:STRING=${C_COMPILER}"
    vopts="${vopts} -DCMAKE_C_FLAGS:STRING=\"${C_OPT_FLAGS} $CFLAGS\""
    vopts="${vopts} -DCMAKE_CXX_COMPILER:STRING=${CXX_COMPILER}"
    # Version 4.0 now requires c++11
    vopts="${vopts} -DCMAKE_CXX_FLAGS:STRING=\"${CXX_OPT_FLAGS} $CXXFLAGS -std=c++11\""
    vopts="${vopts} -DCMAKE_INSTALL_PREFIX:PATH=${VISITDIR}/mfem/${MFEM_VERSION}/${VISITARCH}"
    if test "x${DO_STATIC_BUILD}" = "xyes" ; then
        vopts="${vopts} -DBUILD_SHARED_LIBS:BOOL=OFF"
    else
        vopts="${vopts} -DBUILD_SHARED_LIBS:BOOL=ON"
    fi
    vopts="${vopts} -DMFEM_USE_EXCEPTIONS:BOOL=ON"
    if [[ "$DO_CONDUIT" == "yes" ]] ; then
        vopts="${vopts} -DMFEM_USE_CONDUIT=ON -DCONDUIT_DIR=${VISITDIR}/conduit/${CONDUIT_VERSION}/${VISITARCH}"
    fi

    # when using conduit, mfem's cmake logic requires HDF5_DIR to find HDF5
    # (NOTE: mfem could use CONDUIT_HDF5_DIR)
    if [[ "$DO_HDF5" == "yes" ]] ; then
        vopts="${vopts} -DHDF5_DIR=${VISITDIR}/hdf5/${HDF5_VERSION}/${VISITARCH}"
    fi

    if [[ "$DO_FMS" == "yes" ]] ; then
        vopts="${vopts} -DMFEM_USE_FMS=ON -DFMS_DIR=${VISITDIR}/fms/${FMS_VERSION}/${VISITARCH}"
    else
        vopts="${vopts} -DMFEM_USE_FMS=OFF"
    fi
    vopts="${vopts} -DMFEM_USE_ZLIB=ON"

    #
    # Call configure
    #
    info "Configuring mfem . . ."
    CMAKE_BIN="${CMAKE_INSTALL}/cmake"
    if test -e bv_run_cmake.sh ; then
        rm -f bv_run_cmake.sh
    fi
    CMS=bv_run_cmake.sh
    echo "#!/bin/bash" > $CMS
    echo "# Find the right ZLIB" >> $CMS
    echo "export CMAKE_PREFIX_PATH=${VISITDIR}/zlib/${ZLIB_VERSION}/${VISITARCH}" >> $CMS
    if [[ "$DO_HDF5" == "yes" ]] ; then
        echo "# Find the right HDF5" >> $CMS
        echo "export HDF5_ROOT=${VISITDIR}/hdf5/${HDF5_VERSION}/${VISITARCH}" >> $CMS
    fi
    echo "\"${CMAKE_BIN}\" ${vopts} .." >> $CMS
    cat $CMS
    issue_command bash $CMS || error "FMS configuration failed."

    #
    # Build mfem
    #
    info "Building mfem . . . (~2 minutes)"
    $MAKE $MAKE_OPT_FLAGS
    if [[ $? != 0 ]] ; then
        warn "mfem build failed.  Giving up"
        return 1
    fi

    #
    # Install into the VisIt third party location.
    #
    info "Installing mfem"
    $MAKE install

    if [[ "$DO_GROUP" == "yes" ]] ; then
        chmod -R ug+w,a+rX "$VISITDIR/mfem"
        chgrp -R ${GROUP} "$VISITDIR/mfem"
    fi
    cd "$START_DIR"
    info "Done with mfem"
    return 0
}


function bv_mfem_is_enabled
{
    if [[ $DO_MFEM == "yes" ]]; then
        return 1    
    fi
    return 0
}

function bv_mfem_is_installed
{
    check_if_installed "mfem" $MFEM_VERSION
    if [[ $? == 0 ]] ; then
        return 1
    fi
    return 0
}

function bv_mfem_build
{
    cd "$START_DIR"
    if [[ "$DO_MFEM" == "yes" ]] ; then
        check_if_installed "mfem" $MFEM_VERSION
        if [[ $? == 0 ]] ; then
            info "Skipping mfem build.  mfem is already installed."
        else
            info "Building mfem (~2 minutes)"
            build_mfem
            if [[ $? != 0 ]] ; then
                error "Unable to build or install mfem.  Bailing out."
            fi
            info "Done building mfem"
        fi
    fi
}
