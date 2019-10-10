function bv_xsd_initialize
{
    export DO_XSD="no"
}

function bv_xsd_enable
{ 
    DO_XSD="yes"
}

function bv_xsd_disable
{
    DO_XSD="no"
}

function bv_xsd_depends_on
{
    echo "xercesc"
}

function bv_xsd_info
{
    export XSD_FILE=${XSD_FILE:-"xsd-4.0.0+dep.tar.bz2"}
    export XSD_VERSION=${XSD_VERSION:-"4.0.0"}
    export XSD_COMPATIBILITY_VERSION=${XSD_COMPATIBILITY_VERSION:-"4.0"}
    export XSD_BUILD_DIR=${XSD_BUILD_DIR:-"xsd-${XSD_VERSION}+dep"}
    export XSD_URL=${XSD_URL:-"http://www.codesynthesis.com/download/xsd/4.0"}
    export XSD_MD5_CHECKSUM="ae64d7fcd258addc9b045fe3f96208bb"
    export XSD_SHA256_CHECKSUM="eca52a9c8f52cdbe2ae4e364e4a909503493a0d51ea388fc6c9734565a859817"
}

function bv_xsd_print
{
    printf "%s%s\n" "XSD_FILE=" "${XSD_FILE}"
    printf "%s%s\n" "XSD_VERSION=" "${XSD_VERSION}"
    printf "%s%s\n" "XSD_COMPATIBILITY_VERSION=" "${XSD_COMPATIBILITY_VERSION}"
    printf "%s%s\n" "XSD_BUILD_DIR=" "${XSD_BUILD_DIR}"
}

function bv_xsd_print_usage
{
    printf "%-20s %s [%s]\n" "--xsd"   "Build XSD" "$DO_XSD"
}

function bv_xsd_host_profile
{
    if [[ "$DO_XSD" == "yes" ]] ; then
        echo >> $HOSTCONF
        echo "##" >> $HOSTCONF
        echo "## XSD" >> $HOSTCONF
        echo "##" >> $HOSTCONF
        echo "VISIT_OPTION_DEFAULT(VISIT_XSD_DIR \${VISITHOME}/xsd/$XSD_VERSION/\${VISITARCH})" \
             >> $HOSTCONF
    fi
}

function bv_xsd_ensure
{
    if [[ "$DO_XSD" == "yes" ]] ; then
        ensure_built_or_ready "XSD" $XSD_VERSION $XSD_BUILD_DIR $XSD_FILE $XSD_URL
        if [[ $? != 0 ]] ; then
            ANY_ERRORS="yes"
            DO_XSD="no"
            error "Unable to build XSD.  ${XSD_FILE} not found."
        fi
    fi
}

function bv_xsd_dry_run
{
    if [[ "$DO_XSD" == "yes" ]] ; then
        echo "Dry run option not set for XSD."
    fi
}

function  apply_xsd_patch
{  
    #for any future patches
    return 0
}


function build_xsd
{
    #
    # Prepare build dir
    #
    prepare_build_dir $XSD_BUILD_DIR $XSD_FILE
    untarred_xsd=$?
    if [[ $untarred_xsd == -1 ]] ; then
        warn "Unable to prepare XSD build directory. Giving Up!"
        return 1
    fi
    cd $XSD_BUILD_DIR/xsd

    #
    # For Damaris, we only need to install the XSD headers
    #
    mkdir -p $VISITDIR/xsd/$XSD_VERSION/$VISITARCH/include
    cp -r libxsd/xsd $VISITDIR/xsd/$XSD_VERSION/$VISITARCH/include/

    if [[ "$DO_GROUP" == "yes" ]] ; then
        chmod -R ug+w,a+rX "$VISITDIR/xsd"
        chgrp -R ${GROUP} "$VISITDIR/xsd"
    fi
    cd "$START_DIR"
    return 0
}

function bv_xsd_is_enabled
{
    if [[ $DO_XSD == "yes" ]]; then
        return 1
    fi
    return 0
}

function bv_xsd_is_installed
{
    check_if_installed "xsd" $XSD_VERSION
    if [[ $? == 0 ]] ; then
        return 1
    fi
    return 0
}

function bv_xsd_build
{

    if [[ "$DO_XSD" == "yes" ]] ; then
        check_if_installed "xsd" $XSD_VERSION
        if [[ $? == 0 ]] ; then
            info "Skipping XSD build.  XSD is already installed."
        else
            build_xsd
            if [[ $? != 0 ]] ; then
                error "Unable to build or install XSD.  Bailing out."
            fi
            info "Done building XSD"
        fi
    fi
}
