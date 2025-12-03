function bv_meson_initialize
{
    export DO_MESON="no"
    export USE_SYSTEM_MESON="no"
    add_extra_commandline_args "meson" "alt-meson-dir" 1 "Use alternative directory for meson"
}

function bv_meson_enable
{
    DO_MESON="yes"
}

function bv_meson_disable
{
    DO_MESON="no"
}

function bv_meson_alt_meson_dir
{
    bv_meson_enable
    USE_SYSTEM_MESON="yes"
    MESON_INSTALL_DIR="$1"
    info "Using Alternate meson: $MESON_INSTALL_DIR"
}

function bv_meson_depends_on
{
    local depends_on="ninja python"

    echo ${depends_on}
}

function bv_meson_initialize_vars
{
    if [[ "$USE_SYSTEM_MESON" == "no" ]]; then
        MESON_INSTALL_DIR="${VISITDIR}/meson/${MESON_VERSION}/${VISITARCH}"
    fi
}

function bv_meson_info
{
    export MESON_VERSION=${MESON_VERSION:-"1.9.1"}
    export MESON_FILE=${MESON_FILE:-"meson-${MESON_VERSION}.tar.gz"}
    export MESON_BUILD_DIR=${MESON_BUILD_DIR:-"meson-${MESON_VERSION}"}
    export MESON_SHA256_CHECKSUM=""
}

function bv_meson_print
{
    printf "%s%s\n" "MESON_FILE=" "${MESON_FILE}"
    printf "%s%s\n" "MESON_VERSION=" "${MESON_VERSION}"
    printf "%s%s\n" "MESON_BUILD_DIR=" "${MESON_BUILD_DIR}"
}

function bv_meson_print_usage
{
    printf "%-20s %s [%s]\n" "--meson" "Build meson support" "$DO_MESON"
    printf "%-20s %s [%s]\n" "--alt-meson-dir" "Use meson from an alternative directory"
}

function bv_meson_host_profile
{
    # Nothing added to the host profile since meson is only used for
    # building third party libraries.
    return 0
}

function bv_meson_ensure
{
    if [[ "$DO_MESON" == "yes" && "$USE_SYSTEM_MESON" == "no" ]] ; then
        ensure_built_or_ready "meson" $MESON_VERSION $MESON_BUILD_DIR $MESON_FILE $MESON_URL
        if [[ $? != 0 ]] ; then
            ANY_ERRORS="yes"
            DO_MESON="no"
            error "Unable to build meson. ${MESON_FILE} not found."
        fi
    fi
}

# *************************************************************************** #
#                            Function 8, build_meson
#
#
# *************************************************************************** #
function build_meson
{
    #
    # Build meson
    #
    info "Installing Meson . . . (~2 minutes)"

    # Install the python meson package.
    check_if_py_module_installed "meson"
    if [[ $? != 0 ]] ; then
        download_py_module ${MESON_FILE} ${MESON_URL}
        if [[ $? != 0 ]] ; then
            return 1
        fi

        extract_py_module ${MESON_BUILD_DIR} ${MESON_FILE} "meson"
        if [[ $? != 0 ]] ; then
            return 1
        fi

        install_py_module ${MESON_BUILD_DIR} "meson"
        if [[ $? != 0 ]] ; then
            return 1
        fi

        fix_py_permissions
    fi

    # Create a python script that loads the meson package.
    if [[ ! -d ${MESON_INSTALL_DIR}/bin ]] ; then
        mkdir -p ${MESON_INSTALL_DIR}/bin
    fi
    MESON_CMD="${MESON_INSTALL_DIR}/bin/meson"
    if [[ -f $MESON_CMD ]] ; then
        rm $MESON_CMD
    fi
    echo "#!${PYTHON_COMMAND}" > $MESON_CMD
    echo "# -*- coding: utf-8 -*-" >> $MESON_CMD
    echo "import re" >> $MESON_CMD
    echo "import sys" >> $MESON_CMD
    echo "from mesonbuild.mesonmain import main" >> $MESON_CMD
    echo "if __name__ == '__main__':" >> $MESON_CMD
    echo "    sys.argv[0] = re.sub(r'(-script\.pyw|\.exe)?$', '', sys.argv[0])" >> $MESON_CMD
    echo "    sys.exit(main())" >> $MESON_CMD

    chmod 700 $MESON_CMD

    if [[ "$DO_GROUP" == "yes" ]] ; then
        chmod -R ug+w,a+rX "$VISITDIR/meson"
        chgrp -R ${GROUP} "$VISITDIR/meson"
    fi
    cd "$START_DIR"
    info "Done with meson"
    return 0
}

function bv_meson_is_enabled
{
    if [[ $DO_MESON == "yes" ]]; then
        return 1
    fi
    return 0
}

function bv_meson_is_installed
{
    if [[ "$USE_SYSTEM_MESON" == "yes" ]]; then
        return 1
    fi

    check_if_installed "meson" $MESON_VERSION
    if [[ $? == 0 ]] ; then
        return 1
    fi
    return 0
}

function bv_meson_build
{
    cd "$START_DIR"
    if [[ "$DO_MESON" == "yes" && "$USE_SYSTEM_MESON" == "no" ]] ; then
        check_if_installed "meson" $MESON_VERSION
        if [[ $? == 0 ]] ; then
            info "Skipping meson build. Meson is already installed."
        else
            info "Building meson (~2 minutes)"
            build_meson
            if [[ $? != 0 ]] ; then
                error "Unable to build or install meson.  Bailing out."
            fi
            info "Done building meson"
        fi
    fi
}
