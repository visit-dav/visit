function bv_visus_initialize
{
export DO_VISUS="no"
export ON_VISUS="off"
}

function bv_visus_enable
{
DO_VISUS="yes"
ON_VISUS="on"
}

function bv_visus_disable
{
DO_VISUS="no"
ON_VISUS="off"
}

function bv_visus_depends_on
{
echo ""
}

function bv_visus_info
{
export VISUS_FILE=${VISUS_FILE:-"visus_26Feb07.tar.gz"}
export VISUS_VERSION=${VISUS_VERSION:-"1.0.0"}
export VISUS_COMPATIBILITY_VERSION=${VISUS_COMPATIBILITY_VERSION:-"1.0"}
export VISUS_BUILD_DIR=${VISUS_BUILD_DIR:-"visus"}
export VISUS_MD5_CHECKSUM=""
export VISUS_SHA256_CHECKSUM=""

}

function bv_visus_print
{
  printf "%s%s\n" "VISUS_FILE=" "${VISUS_FILE}"
  printf "%s%s\n" "VISUS_VERSION=" "${VISUS_VERSION}"
  printf "%s%s\n" "VISUS_COMPATIBILITY_VERSION=" "${VISUS_COMPATIBILITY_VERSION}"
  printf "%s%s\n" "VISUS_BUILD_DIR=" "${VISUS_BUILD_DIR}"
}

function bv_visus_print_usage
{
printf "\t\t%15s\n" "NOTE: not available for download from web"
printf "%-15s %s [%s]\n" "--visus" "Build ViSUS (experimental!)" "$DO_VISUS"
}

function bv_visus_graphical
{
local graphical_out="VISUS    $VISUS_VERSION($VISUS_FILE)     $ON_VISUS"
echo "$graphical_out"
}

function bv_visus_host_profile
{
    if [[ "$DO_VISUS" == "yes" ]] ; then
        echo >> $HOSTCONF
        echo "##" >> $HOSTCONF
        echo "## Visus" >> $HOSTCONF
        echo "##" >> $HOSTCONF
        echo \
        "VISIT_OPTION_DEFAULT(VISIT_VISUS_DIR \${VISITHOME}/visus/$VISUS_VERSION/\${VISITARCH})" \
        >> $HOSTCONF
    fi
}

function bv_visus_ensure
{
    if [[ "$DO_VISUS" == "yes" ]] ; then
        ensure_built_or_ready "visus" $VISUS_VERSION $VISUS_BUILD_DIR $VISUS_FILE
        if [[ $? != 0 ]] ; then
            ANY_ERRORS="yes"
            DO_VISUS="no"
            error "Unable to build ViSUS.  ${VISUS_FILE} not found."
        fi
    fi
}

function bv_visus_dry_run
{
  if [[ "$DO_VISUS" == "yes" ]] ; then
    echo "Dry run option not set for visus."
  fi
}

function bv_visus_is_enabled
{
    return 0
}

function bv_visus_is_installed
{
    check_if_installed "visus" $VISUS_VERSION
    if [[ $? == 0 ]] ; then
        return 1
    fi
    return 0
}

function bv_visus_build
{
  if [[ "$DO_VISUS" == "yes" ]] ; then
    echo "Build support not available for visus.."
  fi
}

