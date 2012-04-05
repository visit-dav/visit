function bv_fastbit_initialize
{
export DO_FASTBIT="no"
export ON_FASTBIT="off"
}

function bv_fastbit_enable
{
DO_FASTBIT="yes"
ON_FASTBIT="on"
}

function bv_fastbit_disable
{
DO_FASTBIT="no"
ON_FASTBIT="off"
}

function bv_fastbit_depends_on
{
echo ""
}

function bv_fastbit_info
{
export FASTBIT_VERSION=${FASTBIT_VERSION:-"1.2.0"}
export FASTBIT_FILE=${FASTBIT_FILE:-"fastbit-ibis${FASTBIT_VERSION}.tar.gz"}
# Note: last 3-digit field in URL changes with version.
export FASTBIT_URL=${FASTBIT_URL:-"https://codeforge.lbl.gov/frs/download.php/208"}
export FASTBIT_BUILD_DIR=${FASTBIT_BUILD_DIR:-"fastbit-ibis${FASTBIT_VERSION}"}
export FASTBIT_MD5_CHECKSUM="380de470b856ecaf19d296aab6406811"
export FASTBIT_SHA256_CHECKSUM=""
}

function bv_fastbit_print
{
  printf "%s%s\n" "FASTBIT_FILE=" "${FASTBIT_FILE}"
  printf "%s%s\n" "FASTBIT_VERSION=" "${FASTBIT_VERSION}"
  printf "%s%s\n" "FASTBIT_BUILD_DIR=" "${FASTBIT_BUILD_DIR}"
}

function bv_fastbit_print_usage
{
printf "%-15s %s [%s]\n" "--fastbit" "Build FastBit" "$DO_FASTBIT"
}

function bv_fastbit_graphical
{
local graphical_out="FastBit  $FASTBIT_VERSION($FASTBIT_FILE)   $ON_FASTBIT"
echo $graphical_out
}

function bv_fastbit_host_profile
{
    if [[ "$DO_FASTBIT" == "yes" ]] ; then
        echo >> $HOSTCONF
        echo "##" >> $HOSTCONF
        echo "## FastBit" >> $HOSTCONF
        echo "##" >> $HOSTCONF
        echo \
        "VISIT_OPTION_DEFAULT(VISIT_FASTBIT_DIR \${VISITHOME}/fastbit/$FASTBIT_VERSION/\${VISITARCH})" \
        >> $HOSTCONF
    fi

}

function bv_fastbit_ensure
{
    if [[ "$DO_FASTBIT" == "yes" ]] ; then
        ensure_built_or_ready "fastbit" $FASTBIT_VERSION $FASTBIT_BUILD_DIR $FASTBIT_FILE $FASTBIT_URL
        if [[ $? != 0 ]] ; then
            warn "Unable to build FastBit.  ${FASTBIT_FILE} not found."
            warn "FastBit is not available for download from the VisIt build site"
            ANY_ERRORS="yes"
            DO_FASTBIT="no"
            error "Try going to https://codeforge.lbl.gov/frs/?group_id=44"
        fi
    fi
}

function bv_fastbit_dry_run
{
  if [[ "$DO_FASTBIT" == "yes" ]] ; then
    echo "Dry run option not set for fastbit."
  fi
}

# *************************************************************************** #
#                         Function 8.14, build_fastbit                        #
# *************************************************************************** #

function build_fastbit
{
    #
    # Unzip the file, provided a gzipped file exists.
    #
    if [[ -d ${FASTBIT_BUILD_DIR} ]] ; then
       info_box "Found ${FASTBIT_BUILD_DIR} . . ." 1>&3
    elif [[ -f ${FASTBIT_FILE} ]] ; then
       info_box "Unzipping/Tarring ${FASTBIT_FILE} . . ." 1>&3
       uncompress_untar ${FASTBIT_FILE}
       if [[ $? != 0 ]] ; then
          echo \
"Unable to untar ${FASTBIT_FILE}.  Corrupted file or out of space on device?"
          return 1
       fi
    elif [[ -f ${FASTBIT_FILE%.*} ]] ; then
       info_box "Tarring ${FASTBIT_FILE%.*} . . ." 1>&3
       $TAR xf ${FASTBIT_FILE%.*}
       if [[ $? != 0 ]] ; then
          echo \
"Unable to untar ${FASTBIT_FILE%.*}.  Corrupted file or out of space on device?"
          return 1
       fi
    fi

    #
    info_box "Configuring FastBit . . ." 1>&3
    cd ${FASTBIT_BUILD_DIR} || error "Can't cd to fastbit build dir."
    echo "Invoking command to configure FastBit"
    ./configure \
       CXX="$CXX_COMPILER" CC="$C_COMPILER" \
       CFLAGS="$CFLAGS $C_OPT_FLAGS" CXXFLAGS="$CXXFLAGS $CXX_OPT_FLAGS" \
       --prefix="$VISITDIR/fastbit/$FASTBIT_VERSION/$VISITARCH" \
       --disable-shared --with-java=no
    if [[ $? != 0 ]] ; then
       echo "FastBit configure failed.  Giving up"
       return 1
    fi

    #
    # Build FastBit
    #
    info_box "Building FastBit . . . (~7 minutes)" 1>&3
    echo "Building FastBit"
    $MAKE $MAKE_OPT_FLAGS
    if [[ $? != 0 ]] ; then
       echo "FastBit build failed.  Giving up"
       return 1
    fi
    info_box "Installing FastBit . . ." 1>&3
    echo "Installing FastBit"
    $MAKE install
    if [[ $? != 0 ]] ; then
       echo "FastBit build (make install) failed.  Giving up"
       return 1
    fi

    if [[ "$DO_GROUP" == "yes" ]] ; then
       chmod -R ug+w,a+rX "$VISITDIR/fastbit"
       chgrp -R ${GROUP} "$VISITDIR/fastbit"
    fi
    cd "$START_DIR"
    echo "Done with FastBit"
    return 0
}

function bv_fastbit_is_enabled
{
    if [[ $DO_FASTBIT == "yes" ]]; then
        return 1    
    fi
    return 0
}

function bv_fastbit_is_installed
{
    check_if_installed "fastbit" $FASTBIT_VERSION
    if [[ $? == 0 ]] ; then
        return 1
    fi
    return 0
}

function bv_fastbit_build
{
cd "$START_DIR"
if [[ "$DO_FASTBIT" == "yes" ]] ; then
    check_if_installed "fastbit" $FASTBIT_VERSION
    if [[ $? == 0 ]] ; then
        info "Skipping FastBit build.  FastBit is already installed."
    else
        info "Building FastBit (~7 minutes)"
        build_fastbit
        if [[ $? != 0 ]] ; then
            error "Unable to build or install FastBit.  Bailing out."
        fi
        info "Done building FastBit"
    fi
fi
}
