function bv_uintah_initialize
{
export FORCE_UINTAH="no"
export DO_UINTAH="no"
export ON_UINTAH="off"
export USE_SYSTEM_UINTAH="no"
add_extra_commandline_args "uintah" "alt-uintah-dir" 1 "Use alternative directory for uintah"
}

function bv_uintah_enable
{
    if [[ "$1" == "force" ]]; then
        FORCE_UINTAH="yes"
    fi

    DO_UINTAH="yes"
    ON_UINTAH="on"
}

function bv_uintah_disable
{
    DO_UINTAH="no"
    ON_UINTAH="off"
}

function bv_uintah_alt_uintah_dir
{
    echo "Using alternate Uintah directory"

    # Check to make sure the directory or a particular include file exists.
    [ ! -e "$1/../src/StandAlone/tools/uda2vis/udaData.h" ] && error "Uintah not found in $1"

    bv_uintah_enable
    USE_SYSTEM_UINTAH="yes"
    UINTAH_INSTALL_DIR="$1"
}

function bv_uintah_depends_on
{
    if [[ "$USE_SYSTEM_UINTAH" == "yes" ]]; then
        echo ""
    else
        echo ""
    fi
}

function bv_uintah_initialize_vars
{
    if [[ "$FORCE_UINTAH" == "no" && "$parallel" == "no" ]]; then
        bv_uintah_disable
        warn "Uintah requested by default but the parallel flag has not been set. Uintah will not be built."
        return
    fi

    if [[ "$USE_SYSTEM_UINTAH" == "no" ]]; then
        UINTAH_INSTALL_DIR="${VISITDIR}/uintah/$UINTAH_VERSION/$VISITARCH"
    fi
}

function bv_uintah_info
{
export UINTAH_VERSION=${UINTAH_VERSION:-"1.6.0"}
export UINTAH_FILE=${UINTAH_FILE:-"Uintah-${UINTAH_VERSION}.tar.gz"}
export UINTAH_COMPATIBILITY_VERSION=${UINTAH_COMPATIBILITY_VERSION:-"1.6"}
export UINTAH_BUILD_DIR=${UINTAH_BUILD_DIR:-"Uintah-${UINTAH_VERSION}/optimized"}
#export UINTAH_URL=${UINTAH_URL:-"http://www.sci.utah.edu/releases/uintah_v${UINTAH_VERSION}/${UINTAH_FILE}"}
export UINTAH_URL=${UINTAH_URL:-"http://www.sci.utah.edu/devbuilds/icse/uintah/${UINTAH_VERSION}"}

export UINTAH_MD5_CHECKSUM=""
export UINTAH_SHA256_CHECKSUM=""
}

function bv_uintah_print
{
  printf "%s%s\n" "UINTAH_FILE=" "${UINTAH_FILE}"
  printf "%s%s\n" "UINTAH_VERSION=" "${UINTAH_VERSION}"
  printf "%s%s\n" "UINTAH_COMPATIBILITY_VERSION=" "${UINTAH_COMPATIBILITY_VERSION}"
  printf "%s%s\n" "UINTAH_BUILD_DIR=" "${UINTAH_BUILD_DIR}"
}

function bv_uintah_print_usage
{
printf "%-15s %s [%s]\n" "--uintah" "Build UINTAH" "${DO_UINTAH}"
printf "%-15s %s [%s]\n" "--alt-uintah-dir"  "Use Uintah" "Use Uintah from alternative directory"
}

function bv_uintah_graphical
{
local graphical_out="UINTAH     $UINTAH_VERSION($UINTAH_FILE)      $ON_UINTAH"
echo $graphical_out
}

function bv_uintah_host_profile
{
    if [[ "$DO_UINTAH" == "yes" ]] ; then
        echo >> $HOSTCONF
        echo "##" >> $HOSTCONF
        echo "## Uintah" >> $HOSTCONF
        echo "##" >> $HOSTCONF

        if [[ "$USE_SYSTEM_UINTAH" == "yes" ]]; then
            echo "VISIT_OPTION_DEFAULT(VISIT_UINTAH_DIR $UINTAH_INSTALL_DIR)" >> $HOSTCONF 
            echo "SET(VISIT_USE_SYSTEM_UINTAH TRUE)" >> $HOSTCONF
        else
            echo \
            "VISIT_OPTION_DEFAULT(VISIT_UINTAH_DIR \${VISITHOME}/uintah/$UINTAH_VERSION/\${VISITARCH})" \
            >> $HOSTCONF 
        fi
    fi
}

function bv_uintah_ensure
{
    if [[ "$DO_UINTAH" == "yes" && "$USE_SYSTEM_UINTAH" == "no" ]] ; then
        ensure_built_or_ready "uintah" $UINTAH_VERSION $UINTAH_BUILD_DIR $UINTAH_FILE $UINTAH_URL 
        if [[ $? != 0 ]] ; then
            ANY_ERRORS="yes"
            DO_UINTAH="no"
            error "Unable to build UINTAH.  ${UINTAH_FILE} not found."
        fi
    fi
}

function bv_uintah_dry_run
{
  if [[ "$DO_UINTAH" == "yes" ]] ; then
    echo "Dry run option not set for uintah."
  fi
}

# **************************************************************************** #
#                          Function 8.1, build_uintah                          #
#                                                                              #
# Kevin Griffin, Mon Nov 24 12:33:02 PST 2014                                  #
# Changed the -showme:compile to -show for OS X Mavericks. The -showme:compile #
# was being reported as an invalid option.                                     #
# **************************************************************************** #

function build_uintah
{

    if [[ "$OPSYS" == "Linux" && "$PAR_COMPILER" == "" && "$PAR_INCLUDE" == "" ]]; then
       warn "For Linux builds the PAR_COMPILER and PAR_INCLUDE environment variables must be set."
       warn "PAR_COMPILER should be of the form \"/path/to/mpi/bin/mpicc\""
       warn "PAR_INCLUDE should be of the form \"-I/path/to/mpi/include\""
       warn "Giving Up!"
       return 1
    fi

    PAR_INCLUDE_STRING=""
    if [[ "$PAR_INCLUDE" != "" ]] ; then
        PAR_INCLUDE_STRING=$PAR_INCLUDE
    fi

    if [[ "$PAR_COMPILER" != "" ]] ; then
        if [[ "$OPSYS" == "Darwin" && "$PAR_COMPILER" == "/usr/bin/mpicc" ]]; then
            PAR_INCLUDE_STRING="-I/usr/include/"
        elif [[ "$OPSYS" == "Linux" && "$PAR_COMPILER" == "mpixlc" ]]; then
            PAR_INCLUDE_STRING=`$PAR_COMPILER -show`
        else
            if [[ -z "$PAR_INCLUDE_STRING" ]]; then
                if [[ "$OPSYS" == "Darwin" && `sw_vers -productVersion` == 10.9.[0-9]* ]] ; then
                   PAR_INCLUDE_STRING=`$PAR_COMPILER -show`
                else
                    PAR_INCLUDE_STRING=`$PAR_COMPILER -showme:compile`
                    if [[ $? != 0 ]] ; then
                       PAR_INCLUDE_STRING=`$PAR_COMPILER -show`
                    fi
                fi
            fi
        fi
    fi

    if [[ "$PAR_INCLUDE_STRING" == "" ]] ; then
       warn "You must set either the PAR_COMPILER or PAR_INCLUDE environment variables."
       warn "PAR_COMPILER should be of the form \"/path/to/mpi/bin/mpicc\""
       warn "PAR_INCLUDE should be of the form \"-I/path/to/mpi/include\""
       warn "Giving Up!"
       return 1
    fi

    # Uintah's config doesn't take the compiler options, but rather the
    # paths to the root, and then it tries to build all of the appropriate
    # options itself.  Because we only have the former, we need to guess at the
    # latter.
    # Our current guess is to take the first substring in PAR_INCLUDE, assume
    # it's the appropriate -I option, and use it with the "-I" removed.  This
    # is certainly not ideal -- for example, it will break if the user's
    # MPI setup requires multiple include directories.

    # Search all of the -I directories and take the first one containing mpi.h
    PAR_INCLUDE_DIR=""
    for arg in $PAR_INCLUDE_STRING ; do
        if [[ "$arg" != "${arg#-I}" ]] ; then
            if test -e "${arg#-I}/mpi.h" ; then
                PAR_INCLUDE_DIR=${arg#-I}
                break
            fi
        fi
    done
    # If we did not get a valid include directory, take the first -I directory.
    if test -z "${PAR_INCLUDE_DIR}"  ; then
        for arg in $PAR_INCLUDE_STRING ; do
            if [[ "$arg" != "${arg#-I}" ]] ; then
                PAR_INCLUDE_DIR=${arg#-I}
                break
            fi
        done
    fi

    if test -z "${PAR_INCLUDE_DIR}"  ; then
        if test -n "${PAR_INCLUDE}" ; then
            warn "This script believes you have defined PAR_INCLUDE as: $PAR_INCLUDE"
            warn "However, to build Uintah, this script expects to parse a -I/path/to/mpi out of PAR_INCLUDE"
        fi
        warn "Could not determine the MPI include information which is needed to compile Uintah."
        if test -n "${PAR_INCLUDE}" ; then
            error "Please re-run with the required \"-I\" option included in PAR_INCLUDE"
        else
            error "You need to specify either PAR_COMPILER or PAR_INCLUDE variable.  On many "
                  " systems, the output of \"mpicc -showme\" is good enough."
            error ""
        fi
    fi

    if [[ "$FC_COMPILER" == "no" ]] ; then

        warn "Uintah may require fortran to be enabled. It does not appear that the --fortran "
        warn "agrument was set. If Uintah fails to build try adding the --fortra
	n argument"
        FORTRANARGS="--without-fortran"
        #return 1

    else
        FORTRANARGS="FC=\"$FC_COMPILER\" F77=\"$FC_COMPILER\" FCFLAGS=\"$FCFLAGS\" FFLAGS=\"$FCFLAGS\" --enable-fortran"
    fi

    #
    # Prepare build dir
    #
    prepare_build_dir $UINTAH_BUILD_DIR $UINTAH_FILE
    untarred_uintah=$?
    if [[ $untarred_uintah == -1 ]] ; then
       warn "Unable to prepare UINTAH Build Directory. Giving Up"
       return 1
    fi

    #
    mkdir $UINTAH_BUILD_DIR
    cd $UINTAH_BUILD_DIR || error "Can't cd to UINTAH build dir."

    info "Configuring UINTAH . . ."
    cf_darwin=""
    if [[ "$DO_STATIC_BUILD" == "yes" ]]; then
            cf_build_type="--disable-shared --enable-static"
        else
            cf_build_type="--enable-shared --disable-static"
    fi

    # In order to ensure $FORTRANARGS is expanded to build the arguments to
    # configure, we wrap the invokation in 'sh -c "..."' syntax

    if [[ "$OPSYS" == "Darwin" ]]; then

      info "Invoking command to configure UINTAH"
      info "../src/configure CXX=\"$CXX_COMPILER\" CC=\"$C_COMPILER\" \
        CFLAGS=\"$CFLAGS $C_OPT_FLAGS\" CXXFLAGS=\"$CXXFLAGS $CXX_OPT_FLAGS\" \
        MPI_EXTRA_LIB_FLAG=\"$PAR_LIBRARY_NAMES\" \
        $FORTRANARGS \
        --prefix=\"$VISITDIR/uintah/$UINTAH_VERSION/$VISITARCH\" \
        ${cf_darwin} \
        ${cf_build_type} \
        --enable-optimize \
        --with-mpi="${PAR_INCLUDE_DIR}/.." "

#        --with-mpi-include="${PAR_INCLUDE_DIR}/" \
#        --with-mpi-lib="${PAR_INCLUDE_DIR}/../lib" "

      sh -c "../src/configure CXX=\"$CXX_COMPILER\" CC=\"$C_COMPILER\" \
        CFLAGS=\"$CFLAGS $C_OPT_FLAGS\" CXXFLAGS=\"$CXXFLAGS $CXX_OPT_FLAGS\" \
        MPI_EXTRA_LIB_FLAG=\"$PAR_LIBRARY_NAMES\" \
        $FORTRANARGS \
        --prefix=\"$VISITDIR/uintah/$UINTAH_VERSION/$VISITARCH\" \
        ${cf_darwin} \
        ${cf_build_type} \
        --enable-optimize \
        --with-mpi="${PAR_INCLUDE_DIR}/.." "

#        --with-mpi-include="${PAR_INCLUDE_DIR}/" \
#        --with-mpi-lib="${PAR_INCLUDE_DIR}/../lib" "

    else

      info "Invoking command to configure UINTAH"
      info "../src/configure CXX=\"$PAR_COMPILER_CXX\" CC=\"$PAR_COMPILER\" \
        CFLAGS=\"$CFLAGS $C_OPT_FLAGS\" CXXFLAGS=\"$CXXFLAGS $CXX_OPT_FLAGS\" \
        MPI_EXTRA_LIB_FLAG=\"$PAR_LIBRARY_NAMES\" \
        $FORTRANARGS \
        --prefix=\"$VISITDIR/uintah/$UINTAH_VERSION/$VISITARCH\" \
        ${cf_build_type} \
        --enable-optimize"

      sh -c "../src/configure CXX=\"$PAR_COMPILER_CXX\" CC=\"$PAR_COMPILER\" \
        CFLAGS=\"$CFLAGS $C_OPT_FLAGS\" CXXFLAGS=\"$CXXFLAGS $CXX_OPT_FLAGS\" \
        MPI_EXTRA_LIB_FLAG=\"$PAR_LIBRARY_NAMES\" \
        $FORTRANARGS \
        --prefix=\"$VISITDIR/uintah/$UINTAH_VERSION/$VISITARCH\" \
        ${cf_build_type} \
        --enable-optimize"
    fi


    if [[ $? != 0 ]] ; then
       warn "UINTAH configure failed.  Giving up"
       return 1
    fi

    #
    # Build UINTAH
    #
    info "Making UINTAH . . ."
    $MAKE $MAKE_OPT_FLAGS
    if [[ $? != 0 ]] ; then
       warn "UINTAH build failed.  Giving up"
       return 1
    fi
    #
    # Install into the VisIt third party location.
    #
    info "Installing UINTAH . . ."

    if [[ ! -e $VISITDIR/uintah ]] ; then
        mkdir $VISITDIR/uintah || error "Can't make UINTAH install dir."
    fi

    if [[ -e $VISITDIR/uintah/$UINTAH_VERSION ]] ; then
        rm -rf $VISITDIR/uintah/$UINTAH_VERSION || error "Can't remove old UINTAH install dir."
    fi

    mkdir $VISITDIR/uintah/$UINTAH_VERSION/ || error "Can't make UINTAH install dir."
    mkdir $VISITDIR/uintah/$UINTAH_VERSION/$VISITARCH || error "Can't make UINTAH install dir."
    mkdir $VISITDIR/uintah/$UINTAH_VERSION/$VISITARCH/lib || error "Can't make UINTAH install dir."
    mkdir $VISITDIR/uintah/$UINTAH_VERSION/$VISITARCH/include || error "Can't make UINTAH install dir."
    mkdir $VISITDIR/uintah/$UINTAH_VERSION/$VISITARCH/include/StandAlone || error "Can't make UINTAH install dir."
    mkdir $VISITDIR/uintah/$UINTAH_VERSION/$VISITARCH/include/StandAlone/tools || error "Can't make UINTAH install dir."
    mkdir $VISITDIR/uintah/$UINTAH_VERSION/$VISITARCH/include/StandAlone/tools/uda2vis || error "Can't make UINTAH install dir."

    cp lib/* $VISITDIR/uintah/$UINTAH_VERSION/$VISITARCH/lib
    cp ../src/StandAlone/tools/uda2vis/udaData.h $VISITDIR/uintah/$UINTAH_VERSION/$VISITARCH/include/StandAlone/tools/uda2vis

#    $MAKE install
    if [[ $? != 0 ]] ; then
       warn "UINTAH install failed.  Giving up"
       return 1
    fi

    if [[ "$DO_STATIC_BUILD" == "no" && "$OPSYS" == "Darwin" ]]; then
        #
        # Make dynamic executable, need to patch up the install path and
        # version information.
        #
        info "Creating dynamic libraries for UINTAH . . ."
    fi

    if [[ "$DO_GROUP" == "yes" ]] ; then
       chmod -R ug+w,a+rX "$VISITDIR/uintah"
       chgrp -R ${GROUP} "$VISITDIR/uintah"
    fi
    cd "$START_DIR"
    info "Done with UINTAH"
    return 0
}

function bv_uintah_is_enabled
{
    if [[ $DO_UINTAH == "yes" ]]; then
        return 1    
    fi
    return 0
}

function bv_uintah_is_installed
{
    if [[ "$USE_SYSTEM_UINTAH" == "yes" ]]; then
        return 1
    fi

    check_if_installed "uintah" $UINTAH_VERSION
    if [[ $? == 0 ]] ; then
        return 1
    fi
    return 0
}

function bv_uintah_build
{
cd "$START_DIR"

if [[ "$DO_UINTAH" == "yes" && "$USE_SYSTEM_UINTAH" == "no" ]] ; then
    check_if_installed "uintah" $UINTAH_VERSION
    if [[ $? == 0 ]] ; then
        info "Skipping UINTAH build.  UINTAH is already installed."
    else
        info "Building UINTAH (~10 minutes)"
        build_uintah
        if [[ $? != 0 ]] ; then
            error "Unable to build or install UINTAH.  Bailing out."
        fi
        info "Done building UINTAH"
    fi
fi
}
