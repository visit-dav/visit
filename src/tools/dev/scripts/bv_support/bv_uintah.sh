function bv_uintah_initialize
{
    export FORCE_UINTAH="no"
    export DO_UINTAH="no"
    export USE_SYSTEM_UINTAH="no"
    add_extra_commandline_args "uintah" "alt-uintah-dir" 1 "Use alternative directory for uintah"
}

function bv_uintah_enable
{
    if [[ "$1" == "force" ]]; then
        FORCE_UINTAH="yes"
    fi

    DO_UINTAH="yes"
}

function bv_uintah_disable
{
    DO_UINTAH="no"
}

function bv_uintah_alt_uintah_dir
{
    echo "Using alternate Uintah directory"

    # Check to make sure the directory or a particular include file exists.
    [ ! -e "$1/../src/VisIt/interfaces/datatypes.h" ] && error "Uintah not found in $1"

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
    
    if [[ "$DO_ZLIB" == "yes" ]] ; then
        echo "zlib"
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
    export UINTAH_VERSION=${UINTAH_VERSION:-"2.6.1"}
    export UINTAH_FILE=${UINTAH_FILE:-"Uintah-${UINTAH_VERSION}.tar.gz"}
    export UINTAH_COMPATIBILITY_VERSION=${UINTAH_COMPATIBILITY_VERSION:-"2.6"}
    export UINTAH_URL=${UINTAH_URL:-"https://gforge.sci.utah.edu/svn/uintah/releases/uintah_v${UINTAH_VERSION}"}
    export UINTAH_BUILD_DIR=${UINTAH_BUILD_DIR:-"Uintah-${UINTAH_VERSION}/optimized"}
    export UINTAH_MD5_CHECKSUM="09cad7b2fcc7b1f41dabcf7ecae21f54"
    export UINTAH_SHA256_CHECKSUM="0801da6e5700fa826f2cbc6ed01f81f743f92df3e946cc6ba3748458f36f674e"
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
    printf "%-20s %s [%s]\n" "--uintah" "Build Uintah" "${DO_UINTAH}"
    printf "%-20s %s [%s]\n" "--alt-uintah-dir" "Use Uintah from an alternative directory"
}

function bv_uintah_host_profile
{
    if [[ "$DO_UINTAH" == "yes" ]] ; then
        echo >> $HOSTCONF
        echo "##" >> $HOSTCONF
        echo "## Uintah" >> $HOSTCONF
        echo "##" >> $HOSTCONF

        if [[ "$USE_SYSTEM_UINTAH" == "yes" ]]; then
            warn "Assuming version 2.7.0 for Uintah"
            echo "SETUP_APP_VERSION(UINTAH 2.7.0)" >> $HOSTCONF
            echo "VISIT_OPTION_DEFAULT(VISIT_UINTAH_DIR $UINTAH_INSTALL_DIR)" >> $HOSTCONF 
            echo "SET(VISIT_USE_SYSTEM_UINTAH TRUE)" >> $HOSTCONF
        else
            echo "SETUP_APP_VERSION(UINTAH $UINTAH_VERSION)" >> $HOSTCONF
            echo \
                "VISIT_OPTION_DEFAULT(VISIT_UINTAH_DIR \${VISITHOME}/uintah/\${UINTAH_VERSION}/\${VISITARCH})" \
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
#                                                                              #
# Kevin Griffin, Wed Aug 28 10:25:30 PDT 2019                                  #
# Added the --with-libxml2 option to ensure that the /usr/lib/ version is used #
# **************************************************************************** #

function build_uintah
{
    if [[ "$OPSYS" == "Linux"  ]]; then
        if [[ "$PAR_COMPILER" == "" || "$PAR_COMPILER_CXX" == "" || "$PAR_INCLUDE" == "" ]]; then
            warn "For Linux builds the PAR_COMPILER, PAR_COMPILER_CXX, and PAR_INCLUDE environment variables must be set."
            if [[ "$PAR_COMPILER" == "" ]]; then
                warn "PAR_COMPILER should be of the form \"/path/to/mpi/bin/mpicc\""
            fi
            if [[ "$PAR_COMPILER_CXX" == "" ]]; then
                warn "PAR_COMPILER_CXX should be of the form \"/path/to/mpi/bin/mpicxx\""
            fi
            if [[ "$PAR_INCLUDE" == "" ]]; then
                warn "PAR_INCLUDE should be of the form \"-I/path/to/mpi/include\""
            fi
            warn "Giving Up!"
            return 1
        fi
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
    if [[ ! -d $UINTAH_BUILD_DIR ]] ; then
        echo "Making build directory $UINTAH_BUILD_DIR"
        mkdir $UINTAH_BUILD_DIR
    fi
    cd $UINTAH_BUILD_DIR || error "Can't cd to UINTAH build dir."

    info "Configuring UINTAH . . ."
    cf_darwin=""
    if [[ "$DO_STATIC_BUILD" == "yes" ]]; then
        cf_build_type="--enable-static"
    else
        cf_build_type="--disable-static"
    fi

    ZLIB_ARGS=""
    
    if [[ "$DO_ZLIB" == "yes" ]]; then
        ZLIB_ARGS="--with-zlib=$VISITDIR/zlib/$ZLIB_VERSION/$VISITARCH"
    fi

    if [[ "$OPSYS" == "Darwin" ]]; then

        info "Invoking command to configure UINTAH"
        info "../src/configure CXX=\"$CXX_COMPILER\" CC=\"$C_COMPILER\" \
        CFLAGS=\"$CFLAGS $C_OPT_FLAGS -headerpad_max_install_names\" CXXFLAGS=\"$CXXFLAGS $CXX_OPT_FLAGS\" \
        MPI_EXTRA_LIB_FLAG=\"$PAR_LIBRARY_NAMES\" \
        $ZLIB_ARGS \
        --prefix=\"$VISITDIR/uintah/$UINTAH_VERSION/$VISITARCH\" \
        ${cf_darwin} \
        ${cf_build_type} \
	--enable-minimal --enable-optimize \
	--with-fortran=no --with-petsc=no --with-hypre=no \
	--with-lapack=no --with-blas=no \
        --with-mpi=\"$PAR_INCLUDE_DIR/..\" \
        --with-libxml2=\"/usr\" "

        #        --with-mpi-include="${PAR_INCLUDE_DIR}/" \
        #        --with-mpi-lib="${PAR_INCLUDE_DIR}/../lib" "

        sh -c "../src/configure CXX=\"$CXX_COMPILER\" CC=\"$C_COMPILER\" \
        CFLAGS=\"$CFLAGS $C_OPT_FLAGS -headerpad_max_install_names\" CXXFLAGS=\"$CXXFLAGS $CXX_OPT_FLAGS\" \
        MPI_EXTRA_LIB_FLAG=\"$PAR_LIBRARY_NAMES\" \
        $ZLIB_ARGS \
        --prefix=\"$VISITDIR/uintah/$UINTAH_VERSION/$VISITARCH\" \
        ${cf_darwin} \
        ${cf_build_type} \
        --enable-minimal --enable-optimize \
	--with-fortran=no --with-petsc=no --with-hypre=no \
	--with-lapack=no --with-blas=no \
        --with-mpi=\"$PAR_INCLUDE_DIR/..\" \
        --with-libxml2=\"/usr\" "

        #        --with-mpi-include="${PAR_INCLUDE_DIR}/" \
        #        --with-mpi-lib="${PAR_INCLUDE_DIR}/../lib" "

    else

        info "Invoking command to configure UINTAH"
        info "../src/configure CXX=\"$PAR_COMPILER_CXX\" CC=\"$PAR_COMPILER\" \
        CFLAGS=\"$CFLAGS $C_OPT_FLAGS\" CXXFLAGS=\"$CXXFLAGS $CXX_OPT_FLAGS\" \
        MPI_EXTRA_LIB_FLAG=\"$PAR_LIBRARY_NAMES\" \
        $ZLIB_ARGS \
        --prefix=\"$VISITDIR/uintah/$UINTAH_VERSION/$VISITARCH\" \
        ${cf_build_type} \
        --enable-minimal --enable-optimize \
	--with-fortran=no --with-petsc=no --with-hypre=no \
	--with-lapack=no --with-blas=no \
        --with-mpi=built-in"

        sh -c "../src/configure CXX=\"$PAR_COMPILER_CXX\" CC=\"$PAR_COMPILER\" \
        CFLAGS=\"$CFLAGS $C_OPT_FLAGS\" CXXFLAGS=\"$CXXFLAGS $CXX_OPT_FLAGS\" \
        MPI_EXTRA_LIB_FLAG=\"$PAR_LIBRARY_NAMES\" \
        $ZLIB_ARGS \
        --prefix=\"$VISITDIR/uintah/$UINTAH_VERSION/$VISITARCH\" \
        ${cf_build_type} \
        --enable-minimal --enable-optimize \
        --with-fortran=no --with-petsc=no --with-hypre=no \
	--with-lapack=no --with-blas=no \
        --with-mpi=built-in"
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

    if [[ ! -e $VISITDIR/uintah/$UINTAH_VERSION ]] ; then
        mkdir $VISITDIR/uintah/$UINTAH_VERSION || error "Can't make UINTAH install dir."
    fi

    if [[ ! -e $VISITDIR/uintah/$UINTAH_VERSION/$VISITARCH ]] ; then
        mkdir $VISITDIR/uintah/$UINTAH_VERSION/$VISITARCH || error "Can't make UINTAH install dir."
    else        
        rm -rf $VISITDIR/uintah/$UINTAH_VERSION/$VISITARCH/* || error "Can't remove old UINTAH install dir."
    fi

    mkdir $VISITDIR/uintah/$UINTAH_VERSION/$VISITARCH/lib || error "Can't make UINTAH install lib dir."
    mkdir $VISITDIR/uintah/$UINTAH_VERSION/$VISITARCH/include || error "Can't make UINTAH install include dir."
    mkdir $VISITDIR/uintah/$UINTAH_VERSION/$VISITARCH/include/VisIt || error "Can't make UINTAH install include/VisIt dir."
    mkdir $VISITDIR/uintah/$UINTAH_VERSION/$VISITARCH/include/VisIt/interfaces || error "Can't make UINTAH install include/VisIt/interfaces dir."

    cp lib/* $VISITDIR/uintah/$UINTAH_VERSION/$VISITARCH/lib
    cp ../src/VisIt/interfaces/datatypes.h $VISITDIR/uintah/$UINTAH_VERSION/$VISITARCH/include/VisIt/interfaces/datatypes.h

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
        INSTALLNAMEPATH="${UINTAH_INSTALL_DIR}/lib"

        libs=`ls ${INSTALLNAMEPATH}/*.${SO_EXT}`

        for lib in $libs;
        do
            # Get the library path right
            install_name_tool -id $lib $lib

            # Find all the dependent libraries (more or less)
            deplibs=`otool -L $lib | sed "s/(.*)//g"`

            for deplib in $deplibs;
            do
                # Only get the libraries related to Uintah
                if [[ `echo $deplib | grep -c ${UINTAH_BUILD_DIR}` == 1 ]] ; then

                    # Get the library name sans the directory path
                    deplibname=`echo $deplib | sed "s/.*\///"`

                    # Finally set the library path
                    install_name_tool -change \
                                      $deplib ${INSTALLNAMEPATH}/$deplibname $lib
                fi
            done
        done
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
