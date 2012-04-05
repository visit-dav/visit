function bv_icet_initialize
{
export DO_ICET="no"
export ON_ICET="off"
}

function bv_icet_enable
{
DO_ICET="yes"
ON_ICET="on"
}

function bv_icet_disable
{
DO_ICET="no"
ON_ICET="off"
}

function bv_icet_depends_on
{
echo ""
}

function bv_icet_info
{
export ICET_FILE=${ICET_FILE:-"IceT-1-0-0.tar.gz"}
export ICET_VERSION=${ICET_VERSION:-"1.0.0"}
export ICET_COMPATIBILITY_VERSION=${ICET_COMPATIBILITY_VERSION:-"1.0.0"}
export ICET_BUILD_DIR=${ICET_BUILD_DIR:-"IceT-1-0-0"}
export ICET_MD5_CHECKSUM="90a93507b8fdc88f46b9a8d7ed651c6c"
export ICET_SHA256_CHECKSUM=""
}

function bv_icet_print
{
  printf "%s%s\n" "ICET_FILE=" "${ICET_FILE}"
  printf "%s%s\n" "ICET_VERSION=" "${ICET_VERSION}"
  printf "%s%s\n" "ICET_COMPATIBILITY_VERSION=" "${ICET_COMPATIBILITY_VERSION}"
  printf "%s%s\n" "ICET_BUILD_DIR=" "${ICET_BUILD_DIR}"
}

function bv_icet_print_usage
{
printf "%-15s %s [%s]\n" "--icet" "Build Ice-T (parallel rendering lib)" "$DO_ICET"
printf "%-15s %s [%s]\n" "--no-icet" "Ice-T is automatically built with --enable-parallel.  Prevent it from being built" "$PREVENT_ICET"  
}

function bv_icet_graphical
{
local graphical_out="IceT     $ICET_VERSION($ICET_FILE)      $ON_ICET"
echo $graphical_out
}

function bv_icet_host_profile
{
    if [[ "$DO_ICET" == "yes" && "$PREVENT_ICET" != "yes" ]] ; then
        echo >> $HOSTCONF
        echo "##" >> $HOSTCONF
        echo "## Ice-T" >> $HOSTCONF
        echo "##" >> $HOSTCONF
        echo \
        "VISIT_OPTION_DEFAULT(VISIT_ICET_DIR \${VISITHOME}/icet/$ICET_VERSION/\${VISITARCH})" \
        >> $HOSTCONF
    fi
}

function bv_icet_ensure
{
    if [[ "$DO_ICET" == "yes" && "$PREVENT_ICET" != "yes" ]] ; then
        ensure_built_or_ready "icet" $ICET_VERSION $ICET_BUILD_DIR $ICET_FILE "http://www.cs.unm.edu/~kmorel/IceT"
        if [[ $? != 0 ]] ; then
            ANY_ERRORS="yes"
            DO_ICET="no"
            error "Unable to build Ice-T.  ${ICET_FILE} not found."
        fi
    fi
}

function bv_icet_dry_run
{
  if [[ "$DO_ICET" == "yes" ]] ; then
    echo "Dry run option not set for icet."
  fi
}

# *************************************************************************** #
#                           Function 8.13, build_icet                         #
# *************************************************************************** #

function apply_icet_100_patch
{
   patch -p0 <<\EOF
diff -c a/src/CMakeLists.txt IceT-1-0-0/src/CMakeLists.txt
*** a/src/CMakeLists.txt
--- IceT-1-0-0/src/CMakeLists.txt
***************
*** 18,35 ****
        "${CMAKE_CURRENT_SOURCE_DIR}/communication"
        "${CMAKE_CURRENT_SOURCE_DIR}/strategies")
    SET(filesToInstall)
!   FOREACH(p IN ${resPath})
        SET(tmpFilesToInstall)
        SET(exts "${p}/*.h;${p}/*.hxx;${p}/*.txx")
!       FOREACH(ext IN ${exts})
            FILE(GLOB tmpFilesToInstall
            RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}"
            "${ext}")
            IF(tmpFilesToInstall)
                SET(filesToInstall "${filesToInstall};${tmpFilesToInstall}")
            ENDIF(tmpFilesToInstall)
!       ENDFOREACH(ext IN ${exts})
!   ENDFOREACH(p IN ${resPath})
    INSTALL(
        FILES ${filesToInstall}
        DESTINATION "${ICET_INSTALL_INCLUDE_DIR}/ice-t"
--- 18,35 ----
        "${CMAKE_CURRENT_SOURCE_DIR}/communication"
        "${CMAKE_CURRENT_SOURCE_DIR}/strategies")
    SET(filesToInstall)
!   FOREACH(p ${resPath})
        SET(tmpFilesToInstall)
        SET(exts "${p}/*.h;${p}/*.hxx;${p}/*.txx")
!       FOREACH(ext ${exts})
            FILE(GLOB tmpFilesToInstall
            RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}"
            "${ext}")
            IF(tmpFilesToInstall)
                SET(filesToInstall "${filesToInstall};${tmpFilesToInstall}")
            ENDIF(tmpFilesToInstall)
!       ENDFOREACH(ext)
!   ENDFOREACH(p)
    INSTALL(
        FILES ${filesToInstall}
        DESTINATION "${ICET_INSTALL_INCLUDE_DIR}/ice-t"
EOF
   if [[ $? != 0 ]] ; then
        warn "Unable to apply patch to IceT 1.0.0."
        return 1
   else
        return 0
   fi
}

function apply_icet_patch
{
   info "Patching IceT . . ."
   if [[ ${ICET_VERSION} == "1.0.0" ]] ; then
      apply_icet_100_patch
      if [[ $? != 0 ]] ; then
        return 1
      fi
   fi

   return 0
}

function build_icet
{
    PAR_INCLUDE_STRING=""
    if [[ "$PAR_INCLUDE" != "" ]] ; then
        PAR_INCLUDE_STRING=$PAR_INCLUDE
    fi
    
    if [[ "$PAR_COMPILER" != "" ]] ; then
        if [[ "$OPSYS" == "Darwin" && "$PAR_COMPILER" == "/usr/bin/mpic++" ]]; then
            PAR_INCLUDE_STRING="-I/usr/include/"
        else
            if [[ -z "$PAR_INCLUDE_STRING" ]]; then
                PAR_INCLUDE_STRING=`$PAR_COMPILER --showme:compile`    
            fi
        fi
    fi
    
    if [[ "$PAR_INCLUDE_STRING" == "" ]] ; then
       warn "You must set either the PAR_COMPILER or PAR_INCLUDE environment variable to be Ice-T."
       warn "PAR_COMPILER should be of the form \"/path/to/mpi/bin/mpic++\""
       warn "PAR_INCLUDE should be of the form \"-I/path/to/mpi/include\""
       warn "Giving Up!"
       return 1
    fi

    # IceT's CMake config doesn't take the compiler options, but rather the
    # paths to certain files, and then it tries to build all of the appropriate
    # options itself.  Since we only have the former, we need to guess at the
    # latter.
    # Our current guess is to take the first substring in PAR_INCLUDE, assume
    # it's the appropriate -I option, and use it with the "-I" removed.  This
    # is certainly not ideal -- for example, it will break if the user's
    # MPI setup requires multiple include directories.

    # split string at space, grab the front/first string in it.
    PAR_INCLUDE_DIR=""
    for arg in $PAR_INCLUDE_STRING ; do
       if [[ "$arg" != "${arg#-I}" ]] ; then
          PAR_INCLUDE_DIR=${arg#-I}
          break
       fi
    done
    if test -z "${PAR_INCLUDE_DIR}"  ; then
        if test -n "${PAR_INCLUDE}" ; then
            warn "This script believes you have defined PAR_INCLUDE as: $PAR_INCLUDE"
            warn "However, to build Ice-T, this script expects to parse a -I/path/to/mpi out of PAR_INCLUDE"
        fi
        warn "Could not determine the MPI include information which is needed to compile IceT."
        if test -n "${PAR_INCLUDE}" ; then
            error "Please re-run with the required \"-I\" option included in PAR_INCLUDE"
        else
            error "You need to specify either PAR_COMPILER or PAR_INCLUDE variable.  On many "
                  " systems, the output of \"mpicxx -showme\" is good enough."
            error ""
        fi
    fi

    #
    # CMake is the build system for IceT.  We already required CMake to be
    # built, so it should be there.
    #
    CMAKE_BIN="$VISITDIR/cmake/${CMAKE_VERSION}/$VISITARCH/bin/cmake"
    if [[ ! -f ${CMAKE_BIN} ]] ; then
       warn "Unable to locate CMake.  Giving Up!"
       return 1
    fi

    prepare_build_dir $ICET_BUILD_DIR $ICET_FILE
    untarred_icet=$?
    if [[ $untarred_icet == -1 ]] ; then
       warn "Unable to prepare Ice-T build directory. Giving Up!"
       return 1
    fi

    apply_icet_patch

    info "Executing CMake on Ice-T"
    cd $ICET_BUILD_DIR || error "Can't cd to IceT build dir."
    if [[ "$DO_STATIC_BUILD" == "no" ]]; then
        LIBEXT="${SO_EXT}"
    else
        LIBEXT="a"
    fi
    touch fakempi.${LIBEXT}
    ${CMAKE_BIN} \
        -DCMAKE_VERBOSE_MAKEFILE:BOOL=ON \
        -DCMAKE_INSTALL_PREFIX:PATH="$VISITDIR/icet/${ICET_VERSION}/${VISITARCH}"\
        -DOPENGL_INCLUDE_DIR:PATH="$VISITDIR/mesa/${MESA_VERSION}/${VISITARCH}/include"\
        -DOPENGL_gl_LIBRARY:FILEPATH="$VISITDIR/mesa/${MESA_VERSION}/${VISITARCH}/lib/libOSMesa.${LIBEXT}"\
        -DCMAKE_C_FLAGS:STRING="-fPIC -DUSE_MGL_NAMESPACE ${CFLAGS} ${C_OPT_FLAGS}"\
        -DMPI_INCLUDE_PATH:PATH="${PAR_INCLUDE_DIR}"\
        -DMPI_LIBRARY:FILEPATH="./fakempi.${LIBEXT}"\
        -DBUILD_TESTING:BOOL=OFF\
        .
    rm fakempi.${LIBEXT}

    if [[ $? != 0 ]] ; then
       warn "Cannot get CMAKE to create the makefiles.  Giving up."
       return 1
    fi

    #
    # Now build Ice-T.
    #
    info "Building Ice-T . . . (~2 minutes)"
    $MAKE $MAKE_OPT_FLAGS
    if [[ $? != 0 ]] ; then
       warn "Ice-T did not build correctly.  Giving up."
       return 1
    fi

    info "Installing Ice-T . . ."

    $MAKE install
    if [[ $? != 0 ]] ; then
       warn "Ice-T: 'make install' failed.  Giving up"
       return 1
    fi

    if [[ "$DO_GROUP" == "yes" ]] ; then
       chmod -R ug+w,a+rX "$VISITDIR/icet"
       chgrp -R ${GROUP} "$VISITDIR/icet"
    fi

    cd "$START_DIR"
    echo "Done with Ice-T"
    return 0
}

function bv_icet_is_enabled
{
    if [[ $DO_ICET == "yes" ]]; then
        return 1    
    fi
    return 0
}

function bv_icet_is_installed
{
    check_if_installed "icet" $ICET_VERSION
    if [[ $? == 0 ]] ; then
        return 1
    fi
    return 0
}

function bv_icet_build
{
cd "$START_DIR"
if [[ "$DO_ICET" == "yes" && "$PREVENT_ICET" != "yes" ]] ; then
    check_if_installed "icet" $ICET_VERSION
    if [[ $? == 0 ]] ; then
        info "Skipping Ice-T build.  Ice-T is already installed."
    else
        info "Building Ice-T (~2 minutes)"
        build_icet
        if [[ $? != 0 ]] ; then
            error "Unable to build or install Ice-T.  Bailing out."
        fi
        info "Done building Ice-T"
    fi
fi
}
