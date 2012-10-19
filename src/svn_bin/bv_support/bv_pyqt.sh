function bv_pyqt_initialize
{
export DO_PYQT="no"
export ON_PYQT="off"
export USE_SYSTEM_PYQT="no"
add_extra_commandline_args "pyqt" "alt-pyqt-dir" 1 "Use alternative directory for PyQt" 
}

function bv_pyqt_enable
{
DO_PYQT="yes"
ON_PYQT="on"
}

function bv_pyqt_disable
{
DO_PYQT="no"
ON_PYQT="off"
}

function bv_pyqt_alt_pyqt_dir
{
    bv_pyqt_enable
    USE_SYSTEM_PYQT="yes"
    PYQT_INSTALL_DIR="$1"
}

function bv_pyqt_depends_on
{
echo ""
}

function bv_pyqt_info
{
export PYQT_FILE=${PYQT_FILE:-"pyqt"}
export PYQT_VERSION=${PYQT_VERSION:-"0"}
export PYQT_COMPATIBILITY_VERSION=${PYQT_COMPATIBILITY_VERSION:-"0"}
export PYQT_BUILD_DIR=${PYQT_BUILD_DIR:-"pyqt"}
export PYQT_MD5_CHECKSUM=""
export PYQT_SHA256_CHECKSUM=""
}

function bv_pyqt_print
{
  printf "%s%s\n" "PYQT_FILE=" "${PYQT_FILE}"
  printf "%s%s\n" "PYQT_VERSION=" "${PYQT_VERSION}"
  printf "%s%s\n" "PYQT_COMPATIBILITY_VERSION=" "${PYQT_COMPATIBILITY_VERSION}"
  printf "%s%s\n" "PYQT_BUILD_DIR=" "${PYQT_BUILD_DIR}"
}

function bv_pyqt_print_usage
{
printf "%-15s %s [%s]\n" "--alt-pyqt-dir"   "Use alternative pyqt dir" "$DO_PYQT"
}

function bv_pyqt_graphical
{
local graphical_out="PyQt    $PYQT_VERSION($PYQT_FILE)    $ON_PYQT"
echo $graphical_out
}

function bv_pyqt_host_profile
{
    if [[ "$DO_PYQT" == "yes"  && "$USE_SYSTEM_PYQT" == "yes" ]] ; then
        echo >> $HOSTCONF
        echo "##" >> $HOSTCONF
        echo "## PYQT" >> $HOSTCONF
        echo "##" >> $HOSTCONF
        if [[ "$USE_SYSTEM_PYQT" == "yes" ]]; then
            echo "VISIT_OPTION_DEFAULT(HAVE_PYQT ON TYPE BOOL)" >> $HOSTCONF
            echo "VISIT_OPTION_DEFAULT(VISIT_SIP_DIR $PYQT_INSTALL_DIR)" >> $HOSTCONF
            echo "VISIT_OPTION_DEFAULT(VISIT_PYQT_DIR $PYQT_INSTALL_DIR)" >> $HOSTCONF
        fi
    fi
}

function bv_pyqt_ensure
{
    #if [[ "$DO_PYQT" == "yes" && "$USE_SYSTEM_PYQT" == "no" ]] ; then
    #    ensure_built_or_ready "pyqt" $PYQT_VERSION $PYQT_BUILD_DIR $PYQT_FILE
    #    if [[ $? != 0 ]] ; then
    #        ANY_ERRORS="yes"
    #        DO_PYQT="no"
    #        error "Unable to build PyQt.  ${PYQT_FILE} not found."
    #    fi
    #fi
    info "nothing to ensure for PyQt"
}

function bv_pyqt_dry_run
{
  if [[ "$DO_PYQT" == "yes" ]] ; then
    echo "Dry run option not set for PyQt."
  fi
}

# ***************************************************************************
#                         Function 8.22, build_PYQT
#
# Modifications:
#
# ***************************************************************************

function build_pyqt
{
    info "nothing to build for PyQt"
}

function bv_pyqt_is_enabled
{
    if [[ $DO_PYQT == "yes" && "$USE_SYSTEM_PYQT" == "yes" ]]; then
        return 1    
    fi
    return 0
}

function bv_pyqt_is_installed
{
    if [[ "$USE_SYSTEM_PYQT" == "yes" ]]; then
        return 1
    fi

    #check_if_installed "pyqt" $PYQT_VERSION
    #if [[ $? == 0 ]] ; then
    #    return 1
    #fi
    return 0
}

function bv_pyqt_build
{
cd "$START_DIR"
if [[ "$DO_PYQT" == "yes" && "$USE_SYSTEM_PYQT" == "no" ]] ; then
    check_if_installed "pyqt" $PYQT_VERSION
    if [[ $? == 0 ]] ; then
        info "Skipping PyQt build.  PyQt is already installed."
    else
        info "Building PyQt (~20 minutes)"
        build_pyqt
        if [[ $? != 0 ]] ; then
            error "Unable to build or install PyQt.  Bailing out."
        fi
        info "Done building PyQt"
   fi
fi
}

