function bv_qwt_initialize
{
    if [[ "$DO_SERVER_COMPONENTS_ONLY" == "yes" ]]; then
        export DO_QT="no"
    else
        export DO_QT="yes"
    fi
    export DO_QWT="yes"
    export ON_QWT="on"
    export FORCE_QWT="no"
    export USE_SYSTEM_QWT="no"
    add_extra_commandline_args "qwt" "alt-qwt-dir" 1 "Use alternative directory for Qwt"
}

function bv_qwt_enable
{
    DO_QWT="yes"
    ON_QWT="on"
    FORCE_QWT="yes"
}

function bv_qwt_disable
{
    DO_QWT="no"
    ON_QWT="off"
    FORCE_QT="no"
}

function bv_qwt_force
{
    if [[ "$FORCE_QWT" == "yes" ]]; then
        return 0;
    fi
    return 1;
}

function bv_qwt_alt_qwt_dir
{
    bv_qwt_enable
    USE_SYSTEM_QWT="yes"
    QWT_INSTALL_DIR="$1"
}

function bv_qwt_depends_on
{
    depends_on="qt"

    if [[ "$USE_SYSTEM_QWT" == "yes" ]]; then
        echo ""
    else
        echo $depends_on
    fi
}

function bv_qwt_initialize_vars
{
    if [[ "$USE_SYSTEM_QWT" == "no" ]]; then
        QWT_INSTALL_DIR="${VISITDIR}/qwt/${QWT_VERSION}/${VISITARCH}"
    fi
}

function bv_qwt_info
{
    export QWT_FILE=${QWT_FILE:-"qwt-6.1.2.tar.bz2"}
    export QWT_VERSION=${QWT_VERSION:-"6.1.2"}
    export QWT_COMPATIBILITY_VERSION=${QWT_COMPATIBILITY_VERSION:-"6.0"}
    export QWT_BUILD_DIR=${QWT_BUILD_DIR:-"qwt-6.1.2"}
#    export QWT_MD5_CHECKSUM="9cc9125a58b905a4148e4e2fda3fabc6"
#    export QWT_SHA256_CHECKSUM=""
}

function bv_qwt_print
{
    printf "%s%s\n" "QWT_FILE=" "${QWT_FILE}"
    printf "%s%s\n" "QWT_VERSION=" "${QWT_VERSION}"
    printf "%s%s\n" "QWT_COMPATIBILITY_VERSION=" "${QWT_COMPATIBILITY_VERSION}"
    printf "%s%s\n" "QWT_BUILD_DIR=" "${QWT_BUILD_DIR}"
}

function bv_qwt_print_usage
{
    printf "%-15s %s [%s]\n" "--qwt" "Build with Qwt" "$DO_QWT"  
    printf "%-15s %s [%s]\n" "--alt-qwt-dir" "Use Qwt from an alternative directory"
}

function bv_qwt_graphical
{
    local graphical_out="Qwt     $QWT_VERSION($QWT_FILE)      $ON_QWT"
    echo "$graphical_out"
}

function bv_qwt_host_profile
{
    if [[ "$DO_DBIO_ONLY" != "yes" ]]; then
        if [[ "$DO_ENGINE_ONLY" != "yes" ]]; then
            if [[ "$DO_SERVER_COMPONENTS_ONLY" != "yes" ]]; then 
                echo >> $HOSTCONF
                echo "##" >> $HOSTCONF
                echo "## QWT" >> $HOSTCONF
                echo "##" >> $HOSTCONF
                echo "SETUP_APP_VERSION(QWT $QWT_VERSION)" >> $HOSTCONF
                if [[ "$USE_SYSTEM_QWT" == "yes" ]]; then
                    echo "VISIT_OPTION_DEFAULT(VISIT_QWT_DIR $SYSTEM_QWT_DIR)" >> $HOSTCONF
                else
                    echo "VISIT_OPTION_DEFAULT(VISIT_QWT_DIR \${VISITHOME}/qwt/\${QWT_VERSION}/\${VISITARCH})" >> $HOSTCONF
                fi
            fi
        fi
    fi
}

function bv_qwt_ensure
{    
    if [[ "$DO_QWT" == "yes" ]] ; then
        ensure_built_or_ready "qwt" $QWT_VERSION $QWT_BUILD_DIR $QWT_FILE
        if [[ $? != 0 ]] ; then
            ANY_ERRORS="yes"
            DO_QWT="no"
            error "Unable to build Qwt.  ${QWT_FILE} not found."
        fi
    fi
}

function bv_qwt_dry_run
{
    if [[ "$DO_QWT" == "yes" ]] ; then
        echo "Dry run option not set for qwt."
    fi
}

function apply_qwt_linux_patch
{
    PATCHFILE="./patchfile.patch"
    rm -rf $PATCHFILE
    touch $PATCHFILE

    echo "--- qwtconfig.pri        2014-12-11 07:13:13.000000000 -0700" >> $PATCHFILE
    echo "+++ qwtconfig.pri.new    2016-05-03 16:14:00.000000000 -0600" >> $PATCHFILE
    echo "@@ -19,7 +19,7 @@" >> $PATCHFILE
    echo " QWT_INSTALL_PREFIX = \$\$[QT_INSTALL_PREFIX]" >> $PATCHFILE
    echo " " >> $PATCHFILE
    echo " unix {" >> $PATCHFILE
    echo "-    QWT_INSTALL_PREFIX    = /usr/local/qwt-\$\$QWT_VERSION" >> $PATCHFILE
    echo "+    QWT_INSTALL_PREFIX    = ${QWT_INSTALL_DIR}" >> $PATCHFILE
    echo "     # QWT_INSTALL_PREFIX = /usr/local/qwt-\$\$QWT_VERSION-qt-\$\$QT_VERSION" >> $PATCHFILE
    echo " }" >> $PATCHFILE

    patch -p0 < $PATCHFILE

    if [[ $? != 0 ]] ; then
        warn "qwt patch failed."
        return 1
    fi

#    rm -rf $PATCHFILE
    
    return 0;
}

function apply_qwt_static_patch
{
    # must patch a file in order to create static library
    info "Patching qwt for static build"
    patch -p0 << \EOF
diff -c qwtconfig.pri.orig qwtconfig.pri
*** qwtconfig.pri.orig  2016-05-24 14:09:02.000000000 -0700
--- qwtconfig.pri       2016-05-24 14:10:06.268628351 -0700
***************
*** 72,78 ****
  # it will be a static library.
  ######################################################################

! QWT_CONFIG           += QwtDll

  ######################################################################
  # QwtPlot enables all classes, that are needed to use the QwtPlot
--- 72,78 ----
  # it will be a static library.
  ######################################################################

! #QWT_CONFIG           += QwtDll

  ######################################################################
  # QwtPlot enables all classes, that are needed to use the QwtPlot
***************
*** 93,99 ****
  # export a plot to a SVG document
  ######################################################################

! QWT_CONFIG     += QwtSvg

  ######################################################################
  # If you want to use a OpenGL plot canvas
--- 93,99 ----
  # export a plot to a SVG document
  ######################################################################

! #QWT_CONFIG     += QwtSvg

  ######################################################################
  # If you want to use a OpenGL plot canvas


EOF
    if [[ $? != 0 ]] ; then
        warn "qwt static patch failed."
        return 1
    fi

    return 0;

}

function apply_qwt_patch
{
    if [[ "$OPSYS" == "Linux" || "$OPSYS" == "Darwin" ]]; then
        apply_qwt_linux_patch
    fi

    if [[ "$DO_STATIC_BUILD" == "yes" ]] ; then
        apply_qwt_static_patch
    fi
}

# *************************************************************************** #
#                          Function 8.0, build_qwt                           #
# *************************************************************************** #

function build_qwt
{
    #
    # Prepare build dir
    #
    prepare_build_dir $QWT_BUILD_DIR $QWT_FILE
    untarred_qwt=$?
    if [[ $untarred_qwt == -1 ]] ; then
        warn "Unable to prepare Qwt build directory. Giving Up!"
        return 1
    fi

    #
    # Apply patches
    #
    info "Patching Qwt . . ."
    cd $QWT_BUILD_DIR || error "Can't cd to Qwt build dir."
    apply_qwt_patch
    if [[ $? != 0 ]] ; then
        if [[ $untarred_qwt == 1 ]] ; then
            warn "Giving up on Qwt build because the patch failed."
            return 1
        else
            warn "Patch failed, but continuing.  I believe that this script\n" \
                 "tried to apply a patch to an existing directory that had\n" \
                 "already been patched ... that is, the patch is\n" \
                 "failing harmlessly on a second application."
        fi
    fi

    #
    # Build Qwt
    #
    info "Building Qwt project. . . (~1 minute)"
    ${QT_BIN_DIR}/qmake qwt.pro
    if [[ $? != 0 ]] ; then
        warn "Qwt project build failed.  Giving up"
        return 1
    fi
    
    info "Building Qwt. . . (~2 minutes)"
    $MAKE
    if [[ $? != 0 ]] ; then
        warn "Qwt build failed.  Giving up"
        return 1
    fi
    #
    # Install into the VisIt third party location.
    #
    info "Installing Qwt . . ."

    $MAKE install
    if [[ $? != 0 ]] ; then
        warn "Qwt install failed.  Giving up"
        return 1
    fi

    if [[ "$DO_STATIC_BUILD" == "no" && "$OPSYS" == "Darwin" ]]; then
        #
        # Make dynamic executable, need to patch up the install path and
        # version information.
        #
        info "Creating dynamic libraries for Qwt . . ."

	fulllibname="${QWT_INSTALL_DIR}/lib/qwt.framework/Versions/6/qwt"
	
        install_name_tool -id $fulllibname $fulllibname
	
        if [[ $? != 0 ]] ; then
            warn "Qwt dynamic library build failed.  Giving up"
            return 1
        fi
    fi

    if [[ "$DO_GROUP" == "yes" ]] ; then
        chmod -R ug+w,a+rX "$VISITDIR/qwt"
        chgrp -R ${GROUP} "$VISITDIR/qwt"
    fi
    cd "$START_DIR"
    info "Done with Qwt"
    return 0
}

function bv_qwt_is_enabled
{
    if [[ $DO_QWT == "yes" ]]; then
        return 1    
    fi
    return 0
}

function bv_qwt_is_installed
{
    check_if_installed "qwt" $QWT_VERSION
    if [[ $? == 0 ]] ; then
        return 1
    fi
    return 0
}

function bv_qwt_build
{
    cd "$START_DIR"
    if [[ "$DO_QWT" == "yes" ]] ; then
        check_if_installed "qwt" $QWT_VERSION
        if [[ $? == 0 ]] ; then
            info "Skipping QWT build. Qwt is already installed."
        else
            info "Building QWT (~3 minutes)"
            build_qwt
            if [[ $? != 0 ]] ; then
                error "Unable to build or install Qwt. Bailing out."
            fi
            info "Done building Qwt"
        fi
    fi
}
