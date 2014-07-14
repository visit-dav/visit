function bv_mfem_initialize
{
export DO_MFEM="no"
export ON_MFEM="off"
}

function bv_mfem_enable
{
DO_MFEM="yes"
ON_MFEM="on"
}

function bv_mfem_disable
{
DO_MFEM="no"
ON_MFEM="off"
}

function bv_mfem_depends_on
{
    local depends_on=""

    echo $depends_on
}

function bv_mfem_info
{
export MFEM_VERSION=${MFEM_VERSION:-"68e941f8fe"}
export MFEM_FILE=${MFEM_FILE:-"mfem-${MFEM_VERSION}.tgz"}
export MFEM_BUILD_DIR=${MFEM_BUILD_DIR:-"mfem-${MFEM_VERSION}"}
export MFEM_URL=${MFEM_URL:-"https://mfem.googlecode.com/files/mfem-${MFEM_VERSION}.tgz"}
export MFEM_MD5_CHECKSUM="673ec6b60626673767722562aa940eb3"
export MFEM_SHA256_CHECKSUM="ab24aa878344dfe34b3bbf7cd062f63967b99739426bbc870bc11b4bed16919f"
}

function bv_mfem_print
{
  printf "%s%s\n" "MFEM_FILE=" "${MFEM_FILE}"
  printf "%s%s\n" "MFEM_VERSION=" "${MFEM_VERSION}"
  printf "%s%s\n" "MFEM_BUILD_DIR=" "${MFEM_BUILD_DIR}"
}

function bv_mfem_print_usage
{
printf "%-15s %s [%s]\n" "--mfem" "Build mfem support" "$DO_MFEM"
}

function bv_mfem_graphical
{
local graphical_out="mfem     $MFEM_VERSION($MFEM_FILE)      $ON_MFEM"
echo "$graphical_out"
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

function bv_mfem_dry_run
{
  if [[ "$DO_MFEM" == "yes" ]] ; then
    echo "Dry run option not set for mfem."
  fi
}

function apply_mfem_patch
{
    #
    # mfem ignores CXX, CXX , CFLAGS, etc, so we have to patch them into 
    # their main makefiles (GNUMakefile & makefile)
    #


    info "Patching mfem"
    patch -p0 << \EOF
*** GNUmakefile.orig    Mon Jul 14 14:43:58 2014
--- GNUmakefile Mon Jul 14 15:00:57 2014
***************
*** 10,19 ****
  # Software Foundation) version 2.1 dated February 1999.
  
  # Serial compiler
! CC         = g++
! CCOPTS     =
! DEBUG_OPTS = -g -DMFEM_DEBUG
! OPTIM_OPTS = -O3
  DEPCC      = $(CC)
  
  # Parallel compiler
--- 10,19 ----
  # Software Foundation) version 2.1 dated February 1999.
  
  # Serial compiler
! CC         = ${CXX}
! CCOPTS     = ${CXXFLAGS}
! DEBUG_OPTS = ${CXXFLAGS} -g -DMFEM_DEBUG
! OPTIM_OPTS = ${CXXFLAGS}
  DEPCC      = $(CC)
  
  # Parallel compiler
EOF
    if [[ $? != 0 ]] ; then
      warn "mfem patch 1 failed."
      return 1
    fi

    patch -p0 << \EOF
*** makefile.orig   Mon Jul 14 14:44:03 2014
--- makefile    Mon Jul 14 14:45:03 2014
***************
*** 10,18 ****
  # Software Foundation) version 2.1 dated February 1999.
  
  # Serial compiler
! CC         = g++
! CCOPTS     = -O3
! DEBUG_OPTS = -g -DMFEM_DEBUG
  
  # Parallel compiler
  MPICC      = mpicxx
--- 10,18 ----
  # Software Foundation) version 2.1 dated February 1999.
  
  # Serial compiler
! CC         = ${CXX}
! CCOPTS     = ${CXXFLAGS}
! DEBUG_OPTS = ${CXXFLAGS} -g -DMFEM_DEBUG
  
  # Parallel compiler
  MPICC      = mpicxx
EOF
    if [[ $? != 0 ]] ; then
      warn "mfem patch 2 failed."
      return 1
    fi

return 0

}

# *************************************************************************** #
#                            Function 8, build_mfem
#
#
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
    # Call configure
    #
    #info "Configuring mfem . . ."
    cd $MFEM_BUILD_DIR || error "Can't cd to mfem build dir."



    apply_mfem_patch
    if [[ $? != 0 ]] ; then
        if [[ $untarred_MFEM == 1 ]] ; then
            warn "Giving up on MFEM build because the patch failed."
            return 1
        else
            warn "Patch failed, but continuing.  I believe that this script\n" \
                 "tried to apply a patch to an existing directory which had " \
                 "already been patched ... that is, that the patch is " \
                 "failing harmlessly on a second application."
        fi
    fi

    #
    # mfem as an scons configure, we don't support scons, so we 
    # simply use their makefile
    #

    #
    # Build mfem
    #

    info "Building mfem . . . (~2 minutes)"
    env CXX="$CXX_COMPILER" CC="$C_COMPILER" \
       CFLAGS="$CFLAGS $C_OPT_FLAGS" CXXFLAGS="$CXXFLAGS $CXX_OPT_FLAGS" \
       $MAKE $MAKE_OPT_FLAGS
    if [[ $? != 0 ]] ; then
       warn "mfem build failed.  Giving up"
       return 1
    fi

    #
    # mfem's default makefile doesn't have install logic.
    #

    #
    # Install into the VisIt third party location.
    #
    info "Installing mfem"

    mkdir "$VISITDIR/mfem"
    mkdir "$VISITDIR/mfem/$MFEM_VERSION"
    mkdir "$VISITDIR/mfem/$MFEM_VERSION/$VISITARCH"
    mkdir "$VISITDIR/mfem/$MFEM_VERSION/$VISITARCH/include"
    mkdir "$VISITDIR/mfem/$MFEM_VERSION/$VISITARCH/include/fem"
    mkdir "$VISITDIR/mfem/$MFEM_VERSION/$VISITARCH/include/general"
    mkdir "$VISITDIR/mfem/$MFEM_VERSION/$VISITARCH/include/mesh"
    mkdir "$VISITDIR/mfem/$MFEM_VERSION/$VISITARCH/include/linalg"
    mkdir "$VISITDIR/mfem/$MFEM_VERSION/$VISITARCH/lib"


    cp libmfem.a "$VISITDIR/mfem/$MFEM_VERSION/$VISITARCH/lib"
    cp *.hpp  "$VISITDIR/mfem/$MFEM_VERSION/$VISITARCH/include"
    cp fem/*.hpp  "$VISITDIR/mfem/$MFEM_VERSION/$VISITARCH/include/fem"
    cp general/*.hpp  "$VISITDIR/mfem/$MFEM_VERSION/$VISITARCH/include/general"
    cp mesh/*.hpp  "$VISITDIR/mfem/$MFEM_VERSION/$VISITARCH/include/mesh"
    cp linalg/*.hpp  "$VISITDIR/mfem/$MFEM_VERSION/$VISITARCH/include/linalg"


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

