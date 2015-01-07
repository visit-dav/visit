function bv_boost_initialize
{
export DO_BOOST="no"
export ON_BOOST="off"
export USE_SYSTEM_BOOST="no"
add_extra_commandline_args "boost" "alt-boost-dir" 1 "Use alternative directory for boost"
}

function bv_boost_enable
{
DO_BOOST="yes"
ON_BOOST="on"
}

function bv_boost_disable
{
DO_BOOST="no"
ON_BOOST="off"
}

function bv_boost_alt_boost_dir
{
    bv_boost_enable
    USE_SYSTEM_BOOST="yes"
    BOOST_INSTALL_DIR="$1"
}

function bv_boost_depends_on
{
    if [[ "$USE_SYSTEM_BOOST" == "yes" ]]; then
        echo ""
    else
        echo ""
    fi
}

function bv_boost_initialize_vars
{
    if [[ "$USE_SYSTEM_BOOST" == "no" ]]; then
        BOOST_INSTALL_DIR="${VISITDIR}/boost/$BOOST_VERSION/${VISITARCH}"
    fi
}

function bv_boost_info
{
export BOOST_VERSION=${BOOST_VERSION:-"1_57_0"}
export BOOST_FILE=${BOOST_FILE:-"boost_${BOOST_VERSION}.tar.gz"}
export BOOST_COMPATIBILITY_VERSION=${BOOST_COMPATIBILITY_VERSION:-"1_57"}
export BOOST_BUILD_DIR=${BOOST_BUILD_DIR:-"boost_${BOOST_VERSION}"}
export BOOST_URL=${BOOST_URL:-"http://sourceforge.net/projects/boost/files/boost/1.57.0"}
}

function bv_boost_print
{
  printf "%s%s\n" "BOOST_FILE=" "${BOOST_FILE}"
  printf "%s%s\n" "BOOST_VERSION=" "${BOOST_VERSION}"
  printf "%s%s\n" "BOOST_COMPATIBILITY_VERSION=" "${BOOST_COMPATIBILITY_VERSION}"
  printf "%s%s\n" "BOOST_BUILD_DIR=" "${BOOST_BUILD_DIR}"
}

function bv_boost_print_usage
{
printf "%-15s %s [%s]\n" "--boost" "Build BOOST" "${DO_BOOST}"
}

function bv_boost_graphical
{
local graphical_out="BOOST     $BOOST_VERSION($BOOST_FILE)      $ON_BOOST"
echo $graphical_out
}

function bv_boost_host_profile
{
    if [[ "$DO_BOOST" == "yes" ]] ; then
        echo >> $HOSTCONF
        echo "##" >> $HOSTCONF
        echo "## BOOST" >> $HOSTCONF
        echo "##" >> $HOSTCONF

        echo "SETUP_APP_VERSION(BOOST $BOOST_VERSION)" >> $HOSTCONF
        if [[ "$USE_SYSTEM_BOOST" == "yes" ]]; then
            echo \
            "VISIT_OPTION_DEFAULT(VISIT_BOOST_DIR $BOOST_INSTALL_DIR)" \
            >> $HOSTCONF 
        else
            echo \
            "VISIT_OPTION_DEFAULT(VISIT_BOOST_DIR \${VISITHOME}/boost/$BOOST_VERSION/\${VISITARCH})" \
            >> $HOSTCONF 
        fi
    fi
}

function bv_boost_ensure
{
    if [[ "$DO_BOOST" == "yes" && "$USE_SYSTEM_BOOST" == "no" ]] ; then
        ensure_built_or_ready "boost" $BOOST_VERSION $BOOST_BUILD_DIR $BOOST_FILE $BOOST_URL 
        if [[ $? != 0 ]] ; then
            ANY_ERRORS="yes"
            DO_BOOST="no"
            error "Unable to build BOOST.  ${BOOST_FILE} not found."
        fi
    fi
}

function bv_boost_dry_run
{
  if [[ "$DO_BOOST" == "yes" ]] ; then
    echo "Dry run option not set for boost."
  fi
}

function apply_boost_patch
{
    return 0
}

# *************************************************************************** #
#                          Function 8.1, build_boost                           #
# *************************************************************************** #

function build_boost
{

    build_libs="$build_libs --with-libraries=\"system\" "

    if [[ "$DO_NEKTAR_PLUS_PLUS" == "yes" ]] ; then
        build_libs="$build_libs --with-libraries=\"iostreams,thread,date_time,filesystem,system,program_options\" "
    fi

#    if [[ "$build_libs" == ""  ]] ; then
#        build_libs="--without-libraries=\"chrono,context,filesystem,graph_parallel,iostreams,locale,mpi,program_options,python,regex,serialization,signals,system,thread,timer,wave,date_time,graph,math,random,test,exception\" "
#    fi

    #
    # Prepare build dir
    #
    prepare_build_dir $BOOST_BUILD_DIR $BOOST_FILE
    untarred_boost=$?
    if [[ $untarred_boost == -1 ]] ; then
       warn "Unable to prepare BOOST Build Directory. Giving Up"
       return 1
    fi

    #
    cd $BOOST_BUILD_DIR || error "Can't cd to BOOST build dir."
    apply_boost_patch
    if [[ $? != 0 ]]; then
        warn "Patch failed, but continuing."
    fi
    info "Configuring BOOST . . . $build_libs"

    if [[ "$DO_STATIC_BUILD" == "yes" ]]; then
            cf_build_type="--disable-shared --enable-static"
        else
            cf_build_type="--enable-shared --disable-static"
    fi

    cf_build_thread=""
    if [[ "$DO_THREAD_BUILD" == "yes" ]]; then
        cf_build_thread="--enable-threadsafe --with-pthread"
    fi

    # In order to ensure $FORTRANARGS is expanded to build the arguments to
    # configure, we wrap the invokation in 'sh -c "..."' syntax
    info "Invoking command to configure BOOST"
#    info  "./bootstrap.sh $build_libs \
#        --prefix=\"$VISITDIR/boost/$BOOST_VERSION/$VISITARCH\" "

    sh -c "./bootstrap.sh $build_libs \
        --prefix=\"$VISITDIR/boost/$BOOST_VERSION/$VISITARCH\" "

    if [[ $? != 0 ]] ; then
       warn "BOOST configure failed.  Giving up"
       return 1
    fi

    #
    # Build BOOST
    #
    info "Making BOOST . . ."

    sh -c "./b2"
    if [[ $? != 0 ]] ; then
       warn "BOOST build failed.  Giving up"
       return 1
    fi
    #
    # Install into the VisIt third party location.
    #
    info "Installing BOOST . . ."
    sh -c "./b2 install"
    if [[ $? != 0 ]] ; then
       warn "BOOST install failed.  Giving up"
       return 1
    fi

    if [[ "$DO_STATIC_BUILD" == "no" && "$OPSYS" == "Darwin" ]]; then
        #
        # Make dynamic executable, need to patch up the install path and
        # version information.
        #
        info "Creating dynamic libraries for BOOST . . ."
    fi

    if [[ "$DO_GROUP" == "yes" ]] ; then
       chmod -R ug+w,a+rX "$VISITDIR/boost"
       chgrp -R ${GROUP} "$VISITDIR/boost"
    fi
    cd "$START_DIR"
    info "Done with BOOST"
    return 0
}

function bv_boost_is_enabled
{
    if [[ $DO_BOOST == "yes" ]]; then
        return 1    
    fi
    return 0
}

function bv_boost_is_installed
{

    if [[ "$USE_SYSTEM_BOOST" == "yes" ]]; then
        return 1
    fi

    check_if_installed "boost" $BOOST_VERSION
    if [[ $? == 0 ]] ; then
        return 1
    fi
    return 0
}

function bv_boost_build
{
cd "$START_DIR"

if [[ "$DO_BOOST" == "yes" && "$USE_SYSTEM_BOOST" == "no" ]] ; then
    check_if_installed "boost" $BOOST_VERSION
    if [[ $? == 0 ]] ; then
        info "Skipping BOOST build.  BOOST is already installed."
    else
        info "Building BOOST (~15 minutes)"
        build_boost
        if [[ $? != 0 ]] ; then
            error "Unable to build or install BOOST.  Bailing out."
        fi
        info "Done building BOOST"
    fi
fi
}
