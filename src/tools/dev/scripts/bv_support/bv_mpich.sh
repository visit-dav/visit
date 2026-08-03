function bv_mpich_initialize
{
    export DO_MPICH="no"
}

function bv_mpich_enable
{
    DO_MPICH="yes"
}

function bv_mpich_disable
{
    DO_MPICH="no"
}

function bv_mpich_depends_on
{
    local depends_on=""

    echo $depends_on
}

function bv_mpich_info
{
    export MPICH_VERSION=${MPICH_VERSION:-"3.3.1"}
    export MPICH_FILE=${MPICH_FILE:-"mpich-${MPICH_VERSION}.tar.gz"}
    export MPICH_COMPATIBILITY_VERSION=${MPICH_COMPATIBILITY_VERSION:-"3.3"}
    export MPICH_BUILD_DIR=${MPICH_BUILD_DIR:-"mpich-${MPICH_VERSION}"}
    export MPICH_SHA256_CHECKSUM="fe551ef29c8eea8978f679484441ed8bb1d943f6ad25b63c235d4b9243d551e5"
}

function bv_mpich_print
{
    printf "%s%s\n" "MPICH_FILE=" "${MPICH_FILE}"
    printf "%s%s\n" "MPICH_VERSION=" "${MPICH_VERSION}"
    printf "%s%s\n" "MPICH_COMPATIBILITY_VERSION=" "${MPICH_COMPATIBILITY_VERSION}"
    printf "%s%s\n" "MPICH_BUILD_DIR=" "${MPICH_BUILD_DIR}"
}

function bv_mpich_print_usage
{
    printf "%-20s %s [%s]\n" "--mpich" "Build MPICH support" "$DO_MPICH"
}

function bv_mpich_host_profile
{
    if [[ "$DO_MPICH" == "yes" ]] ; then
        echo >> $HOSTCONF
        echo "##" >> $HOSTCONF
        echo "## MPICH" >> $HOSTCONF
        echo "##" >> $HOSTCONF
        echo "SETUP_APP_VERSION(MPICH $MPICH_VERSION)" >> $HOSTCONF
        echo \
            "VISIT_OPTION_DEFAULT(VISIT_MPICH_DIR \${VISITHOME}/mpich/\${MPICH_VERSION}/\${VISITARCH})" \
            >> $HOSTCONF
        echo "VISIT_OPTION_DEFAULT(VISIT_MPICH_INSTALL ON TYPE BOOL)" >> $HOSTCONF
        echo "" >> $HOSTCONF
        echo "VISIT_OPTION_DEFAULT(VISIT_MPI_HOME \${VISIT_MPICH_DIR} TYPE PATH)"  >> $HOSTCONF
        echo "VISIT_OPTION_DEFAULT(VISIT_PARALLEL ON TYPE BOOL)" >> $HOSTCONF
    fi
}

function bv_mpich_ensure
{
    if [[ "$DO_MPICH" == "yes" ]] ; then
        ensure_built_or_ready "mpich" $MPICH_VERSION $MPICH_BUILD_DIR $MPICH_FILE $MPICH_URL
        if [[ $? != 0 ]] ; then
            ANY_ERRORS="yes"
            DO_MPICH="no"
            error "Unable to build MPICH.  ${MPICH_FILE} not found."
        fi
    fi
}

function apply_mpich_slurm_patch
{
    #
    # Patch for building on a system with slurm. The type of hostlist_t
    # was changed from a pointer to an opaque structure between 23.2.8
    # and 23.11.0. This was determined by comparing releases from the
    # site https://download.schedmd.com/slurm/.
    # 
   patch -p0 << \EOF
diff -c src/pm/hydra/tools/bootstrap/external/slurm_query_node_list.c.orig src/pm/hydra/tools/bootstrap/external/slurm_query_node_list.c
*** src/pm/hydra/tools/bootstrap/external/slurm_query_node_list.c.orig	2025-02-18 09:23:02.009141000 -0800
--- src/pm/hydra/tools/bootstrap/external/slurm_query_node_list.c	2025-02-18 09:24:54.077110000 -0800
***************
*** 10,16 ****
  #include "slurm.h"
  
  #if defined(HAVE_SLURM_SLURM_H)
! #include <slurm/slurm.h>        /* for slurm_hostlist_create */
  #elif defined(HAVE_POSIX_REGCOMP)
  #include <regex.h>      /* for POSIX regular expressions */
  
--- 10,17 ----
  #include "slurm.h"
  
  #if defined(HAVE_SLURM_SLURM_H)
! #include <slurm/slurm.h>         /* for slurm_hostlist_create */
! #include <slurm/slurm_version.h> /* for slurm version macros */
  #elif defined(HAVE_POSIX_REGCOMP)
  #include <regex.h>      /* for POSIX regular expressions */
  
***************
*** 26,32 ****
--- 27,37 ----
  #if defined(HAVE_LIBSLURM)
  static HYD_status list_to_nodes(char *str)
  {
+ #if SLURM_VERSION_NUMBER > SLURM_VERSION_NUM(23,2,8)
+     hostlist_t *hostlist;
+ #else
      hostlist_t hostlist;
+ #endif
      char *host;
      int k = 0;
      HYD_status status = HYD_SUCCESS;
EOF

    if [[ $? != 0 ]] ; then
      warn "mpich slurm patch failed."
      return 1
    fi
    return 0;
}

function apply_mpich_patch
{
    info "Patching MPICH . . ."

    apply_mpich_slurm_patch
    if [[ $? != 0 ]] ; then
        return 1
    fi

    return 0
}

# *************************************************************************** #
#                            Function 8, build_mpich
#
# Modfications:
#   Eric Brugger, Tue Feb 18 09:40:26 PST 2025
#   I added a patch for building on a system with a newer slurm.
#
# *************************************************************************** #

function build_mpich
{
    #
    # Prepare build dir
    #
    prepare_build_dir $MPICH_BUILD_DIR $MPICH_FILE SHA256 $MPICH_SHA256_CHECKSUM
    untarred_mpich=$?
    if [[ $untarred_mpich == -1 ]] ; then
        warn "Unable to prepare MPICH build directory. Giving Up!"
        return 1
    fi
    
    cd $MPICH_BUILD_DIR || error "Can't cd to MPICH build dir."

    #
    # Apply patches
    #
    apply_mpich_patch
    if [[ $? != 0 ]] ; then
        if [[ $untarred_mpich == 1 ]] ; then
            warn "Giving up on MPICH build because the patch failed."
            return 1
        else
            warn "Patch failed, but continuing.  I believe that this script\n" \
                 "tried to apply a patch to an existing directory that had\n" \
                 "already been patched ... that is, the patch is\n" \
                 "failing harmlessly on a second application."
        fi
    fi

    #
    # Call configure
    #
    info "Configuring MPICH . . ."
    info "Invoking command to configure MPICH"

    #
    # Turning off maintainer mode avoids need for any local autotools tools.
    # We should not ever need them because we are not developing mpich.
    # I guess we need shared libs.
    #
    mpich_opts="--disable-maintainer-mode --enable-shared"
    if [[ "$OPSYS" == "Darwin" ]]; then
        mpich_opts="${mpich_opts} --enable-threads=single"
    fi

    #
    # MPICH will fail to build if we disable common blocks '-fno-common'
    # Screen the flags vars to make sure we don't use this option for MPICH
    #
    MPICH_CFLAGS=`echo $CFLAGS | sed -e 's/-fno-common//g'`
    MPICH_C_OPT_FLAGS=`echo $C_OPT_FLAGS | sed -e 's/-fno-common//g'`
    MPICH_CXXFLAGS=`echo $CXXFLAGS | sed -e 's/-fno-common//g'`
    MPICH_CXX_OPT_FLAGS=`echo $CXX_OPT_FLAGS | sed -e 's/-fno-common//g'`
    MPICH_FCFLAGS=`echo $FCFLAGS | sed -e 's/-fno-common//g'`

    #
    # Enable/disable fortran as needed.
    #
    if [[ "$FC_COMPILER" == "no" ]] ; then
        mpich_opts="${mpich_opts} --enable-fortran=no"
    else
        mpich_opts="${mpich_opts} --enable-fortran=all"	
    fi

    set -x
    issue_command env CXX="$CXX_COMPILER" \
                  CC="$C_COMPILER" \
                  CFLAGS="$MPICH_CFLAGS $MPICH_C_OPT_FLAGS" \
                  CXXFLAGS="$MPICH_CXXFLAGS $MPICH_CXX_OPT_FLAGS"\
                  FFLAGS="$MPICH_FCFLAGS"\
                  ./configure ${mpich_opts} \
                  --prefix="$VISITDIR/mpich/$MPICH_VERSION/$VISITARCH"
    set +x
    if [[ $? != 0 ]] ; then
        warn "MPICH configure failed.  Giving up"
        return 1
    fi

    #
    # Build MPICH
    #
    info "Building MPICH . . . (~5 minutes)"
    env $MAKE $MAKE_OPT_FLAGS
    if [[ $? != 0 ]] ; then
        if [[ "$OPSYS" == "Darwin" ]]; then
            warn "MPICH build failed but maybe due to LDFLAGS.\n" \
                 "Retrying MPICH build with LDFLAGS set."
            env $MAKE $MAKE_OPT_FLAGS LDFLAGS="-Wl,-flat_namespace -Wl,-undefined -Wl,suppress"
            if [[ $? != 0 ]] ; then
                warn "MPICH build failed.  Giving up"
                return 1
            fi
        else
            warn "MPICH build failed.  Giving up"
            return 1
        fi
    fi

    #
    # Install into the VisIt third party location.
    #
    info "Installing MPICH"
    $MAKE install
    if [[ $? != 0 ]] ; then
        warn "MPICH install failed.  Giving up"
        return 1
    fi

    cleanup_build_dirs $MPICH_BUILD_DIR

    change_install_dir_perms  "$VISITDIR/mpich"

    cd "$START_DIR"
    info "Done with MPICH"
    return 0
}

function bv_mpich_is_enabled
{
    if [[ $DO_MPICH == "yes" ]]; then
        return 1    
    fi
    return 0
}

function bv_mpich_is_installed
{
    check_if_installed "mpich" $MPICH_VERSION
    if [[ $? == 0 ]] ; then
        return 1
    fi
    return 0
}

function bv_mpich_build
{
    cd "$START_DIR"
    if [[ "$DO_MPICH" == "yes" ]] ; then
        check_if_installed "mpich" $MPICH_VERSION
        if [[ $? == 0 ]] ; then
            info "Skipping MPICH build.  MPICH is already installed."
        else
            info "Building MPICH (~2 minutes)"
            build_mpich
            if [[ $? != 0 ]] ; then
                error "Unable to build or install MPICH.  Bailing out."
            fi
            info "Done building MPICH"
        fi
    fi
}
