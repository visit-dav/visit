function bv_qwt_initialize
{
    export DO_QWT="yes"
    export FORCE_QWT="no"
    export USE_SYSTEM_QWT="no"
    add_extra_commandline_args "qwt" "alt-qwt-dir" 1 "Use alternative directory for Qwt"
}

function bv_qwt_enable
{
    DO_QWT="yes"
    FORCE_QWT="yes"
}

function bv_qwt_disable
{
    DO_QWT="no"
    FORCE_QWT="no"
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
    if [[ "$USE_SYSTEM_QWT" == "yes" ]]; then
        echo ""
    else
        if [[ "$DO_QT6" == "no" ]]; then
            echo "qt"
        else
            echo "qt6"
        fi
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
    if [[ "$DO_QT6" == "no" ]]; then
        export QWT_FILE=${QWT_FILE:-"qwt-6.1.2.tar.bz2"}
        export QWT_VERSION=${QWT_VERSION:-"6.1.2"}
        export QWT_COMPATIBILITY_VERSION=${QWT_COMPATIBILITY_VERSION:-"6.0"}
        export QWT_BUILD_DIR=${QWT_BUILD_DIR:-"qwt-6.1.2"}
        export QWT_SHA256_CHECKSUM="2b08f18d1d3970e7c3c6096d850f17aea6b54459389731d3ce715d193e243d0c"
    else
        export QWT_FILE=${QWT_FILE:-"qwt-git-d3706f6e7f0351d278be2d989a4caaf92b399bbd.tar.xz"}
        export QWT_VERSION=${QWT_VERSION:-"6.3.0"}
        export QWT_COMPATIBILITY_VERSION=${QWT_COMPATIBILITY_VERSION:-"6.3"}
        export QWT_BUILD_DIR=${QWT_BUILD_DIR:-"qwt-6.3.0"}
        export QWT_SHA256_CHECKSUM="39839f3aa83f41d09109296d41659e04bb234d9e41ab551af9f4e9b4fceed251"
    fi
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
    printf "%-20s %s [%s]\n" "--qwt" "Build with Qwt" "$DO_QWT"  
    printf "%-20s %s [%s]\n" "--alt-qwt-dir" "Use Qwt from an alternative directory"
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
    if [[ "$DO_QWT" == "yes" && "$DO_SERVER_COMPONENTS_ONLY" == "no" ]] ; then
        ensure_built_or_ready "qwt" $QWT_VERSION $QWT_BUILD_DIR $QWT_FILE
        if [[ $? != 0 ]] ; then
            ANY_ERRORS="yes"
            DO_QWT="no"
            error "Unable to build Qwt.  ${QWT_FILE} not found."
        fi
    fi
}

function apply_qwt_612_linux_patch
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

function apply_qwt_630_linux_patch
{
    PATCHFILE="./patchfile.patch"
    rm -rf $PATCHFILE
    touch $PATCHFILE

    echo "--- qwtconfig.pri       2023-04-13 07:57:21.545486000 -0700" >> $PATCHFILE
    echo "+++ qwtconfig.pri.new   2023-04-13 07:58:43.382979000 -0700" >> $PATCHFILE
    echo "@@ -19,7 +19,7 @@" >> $PATCHFILE
    echo " QWT_INSTALL_PREFIX = \$\$[QT_INSTALL_PREFIX]" >> $PATCHFILE
    echo " " >> $PATCHFILE
    echo " unix {" >> $PATCHFILE
    echo "-    QWT_INSTALL_PREFIX    = /usr/local/qwt-\$\$QWT_VERSION-dev" >> $PATCHFILE
    echo "+    QWT_INSTALL_PREFIX    = ${QWT_INSTALL_DIR}" >> $PATCHFILE
    echo "     # QWT_INSTALL_PREFIX = /usr/local/qwt-\$\$QWT_VERSION-dev-qt-\$\$QT_VERSION" >> $PATCHFILE
    echo " }" >> $PATCHFILE


    patch -p0 < $PATCHFILE

    if [[ $? != 0 ]] ; then
        warn "qwt 6.3.0 linux patch failed."
        return 1
    fi

    return 0;
}

function apply_qwt_612_static_patch
{
    # must patch a file in order to create static library
    info "Patching qwt for static build"
    patch -p0 << \EOF
*** qwtconfig.pri.orig	2019-02-07 09:54:46.000000000 -0800
--- qwtconfig.pri	2019-02-07 09:54:58.000000000 -0800
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
***************
*** 118,124 ****
  # Otherwise you have to build it from the designer directory.
  ######################################################################
  
! QWT_CONFIG     += QwtDesigner
  
  ######################################################################
  # Compile all Qwt classes into the designer plugin instead
--- 118,124 ----
  # Otherwise you have to build it from the designer directory.
  ######################################################################
  
! #QWT_CONFIG     += QwtDesigner
  
  ######################################################################
  # Compile all Qwt classes into the designer plugin instead


EOF
    if [[ $? != 0 ]] ; then
        warn "qwt static patch failed."
        return 1
    fi

    return 0;

}

function apply_qwt_patch
{
    if [[ "$QWT_VERSION" == "6.1.2" ]]; then
        if [[ "$OPSYS" == "Linux" || "$OPSYS" == "Darwin" ]]; then
            apply_qwt_612_linux_patch
        fi

        if [[ "$DO_STATIC_BUILD" == "yes" ]] ; then
            apply_qwt_612_static_patch
        fi
    elif [[ "$QWT_VERSION" == "6.3.0" ]]; then
        if [[ "$OPSYS" == "Linux" || "$OPSYS" == "Darwin" ]]; then
            apply_qwt_630_linux_patch
        fi
    fi
}

# *************************************************************************** #
#                          Function 8.0, build_qwt                           #
# *************************************************************************** #

function build_qwt
{
    #
    # we need or patch to work for any successive configure to build qwt
    # the easiest and most robust way to tackle this is to always delete
    # the source dir if it exists
    
    if [[ -d ${QWT_BUILD_DIR} ]] ; then
        info "Removing old Qwt build dir ${QWT_BUILD_DIR} . . ."
        rm -rf ${QWT_BUILD_DIR}
    fi

    
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
    info "Configuring Qwt . . . (~1 minute)"
    if [[ "$DO_QT6" == "yes" ]]; then
        info "qt6 is being used"
        ${QT6_BIN_DIR}/qmake qwt.pro
    else
        info "qt6 is being used"
        ${QT_BIN_DIR}/qmake qwt.pro
    fi
    if [[ $? != 0 ]] ; then
        warn "Qwt project build failed.  Giving up"
        return 1
    fi
    
    info "Building Qwt . . . (~2 minutes)"
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

    if [[ "$OPSYS" == "Darwin" ]]; then
        if [[ "$DO_STATIC_BUILD" == "no" ]]; then
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
        else
            # Static build. For whatever reason, it was not installing headers.
            mkdir "${QWT_INSTALL_DIR}/include"
            cp -f src/*.h "${QWT_INSTALL_DIR}/include"
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
    if [[ "$DO_SERVER_COMPONENTS_ONLY" == "yes" ]]; then
        return 0
    fi
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
    if [[ "$DO_QWT" == "yes" && "$DO_SERVER_COMPONENTS_ONLY" == "no" ]] ; then
        check_if_installed "qwt" $QWT_VERSION
        if [[ $? == 0 ]] ; then
            info "Skipping Qwt build. Qwt is already installed."
        else
            info "Building Qwt (~3 minutes)"
            build_qwt
            if [[ $? != 0 ]] ; then
                error "Unable to build or install Qwt. Bailing out."
            fi
            info "Done building Qwt"
        fi
    fi
}
