function bv_mxml_initialize
{
export DO_MXML="no"
export ON_MXML="off"
}

function bv_mxml_enable
{
DO_MXML="yes"
ON_MXML="on"
}

function bv_mxml_disable
{
DO_MXML="no"
ON_MXML="off"
}

function bv_mxml_depends_on
{
echo ""
}

function bv_mxml_info
{
export MXML_FILE=${MXML_FILE:-"mxml-2.6.tar.gz"}
export MXML_VERSION=${MXML_VERSION:-"2.6"}
export MXML_COMPATIBILITY_VERSION=${MXML_COMPATIBILITY_VERSION:-"2.6"}
export MXML_BUILD_DIR=${MXML_BUILD_DIR:-"mxml-2.6"}
export MXML_MD5_CHECKSUM="68977789ae64985dddbd1a1a1652642e"
export MXML_SHA256_CHECKSUM=""
}

function bv_mxml_print
{
  printf "%s%s\n" "MXML_FILE=" "${MXML_FILE}"
  printf "%s%s\n" "MXML_VERSION=" "${MXML_VERSION}"
  printf "%s%s\n" "MXML_COMPATIBILITY_VERSION=" "${MXML_COMPATIBILITY_VERSION}"
  printf "%s%s\n" "MXML_BUILD_DIR=" "${MXML_BUILD_DIR}"
}

function bv_mxml_host_profile
{
#nothing to be done for now..
echo "##" >> $HOSTCONF
}

function bv_mxml_print_usage
{
#mxml does not have an option, it is only dependent on mxml.
printf "%-15s %s [%s]\n" "--mxml" "Build Mxml" "$DO_MXML"
}

function bv_mxml_ensure
{
    if [[ "$DO_MXML" == "yes" ]] ; then
        ensure_built_or_ready "mxml" $MXML_VERSION $MXML_BUILD_DIR $MXML_FILE
        if [[ $? != 0 ]] ; then
            ANY_ERRORS="yes"
            DO_MXML="no"
            error "Unable to build MXML.  ${MXML_FILE} not found."
        fi
    fi
}

function bv_mxml_dry_run
{
  if [[ "$DO_MXML" == "yes" ]] ; then
    echo "Dry run option not set for mxml."
  fi
}

function apply_mxml_26_darwin_patch
{
   patch -p0 << \EOF
diff -c mxml-2.6/Makefile.in mxml-2.6.new/Makefile.in
*** mxml-2.6/Makefile.in	2008-12-05 20:20:38.000000000 -0800
--- mxml-2.6.new/Makefile.in	2012-11-21 11:14:45.000000000 -0800
***************
*** 344,353 ****
--- 344,355 ----
  			--header doc/docset.header --intro doc/docset.intro \
  			--css doc/docset.css --title "Mini-XML API Reference" \
  			mxml.xml || exit 1; \
+         if test -e /Developer/usr/bin/docsetutil; then \
  		/Developer/usr/bin/docsetutil package --output org.minixml.xar \
  			--atom org.minixml.atom \
  			--download-url http://www.minixml.org/org.minixml.xar \
  			org.minixml.docset || exit 1; \
+         fi \
  	fi
EOF
   if [[ $? != 0 ]] ; then
       warn "Unable to patch MXML. Wrong version?"
       return 1
   fi

   return 0
}

function apply_mxml_26_patch
{
   if [[ "$OPSYS" == "Darwin" ]]; then
       apply_mxml_26_darwin_patch
       if [[ $? != 0 ]] ; then
           return 1
       fi
   fi

   return 0
}

function apply_mxml_patch
{
   if [[ ${MXML_VERSION} == 2.6 ]] ; then
       apply_mxml_26_patch
       if [[ $? != 0 ]] ; then
           return 1
       fi
   fi

   return 0
}

# ***************************************************************************
#                         Function 8.21, build_mxml
# Required by ADIOS.
#
# Modifications:
#
# ***************************************************************************

function build_mxml
{
    #
    # Prepare build dir
    #
    prepare_build_dir $MXML_BUILD_DIR $MXML_FILE
    untarred_mxml=$?
    if [[ $untarred_mxml == -1 ]] ; then
       warn "Unable to prepare mxml Build Directory. Giving Up"
       return 1
    fi

    #
    # Apply patches
    #
    info "Patching MXML . . ."
    apply_mxml_patch
    if [[ $? != 0 ]] ; then
       if [[ $untarred_mxml == 1 ]] ; then
          warn "Giving up on MXML build because the patch failed."
          return 1
       else
          warn "Patch failed, but continuing.  I believe that this script" \
               "tried to apply a patch to an existing directory which had" \
               "already been patched ... that is, that the patch is" \
               "failing harmlessly on a second application."
       fi
    fi
    
    #
    info "Configuring mxml . . ."
    cd $MXML_BUILD_DIR || error "Can't cd to mxml build dir."
    info "Invoking command to configure mxml"
    ./configure ${OPTIONAL} CXX="$CXX_COMPILER" \
       CC="$C_COMPILER" CFLAGS="$CFLAGS $C_OPT_FLAGS" CXXFLAGS="$CXXFLAGS $CXX_OPT_FLAGS" \
       --prefix="$VISITDIR/mxml/$MXML_VERSION/$VISITARCH" --disable-threads
    if [[ $? != 0 ]] ; then
       warn "mxml configure failed.  Giving up"
       return 1
    fi

    #
    # Build mxml
    #
    info "Building mxml . . . (~1 minutes)"

    $MAKE $MAKE_OPT_FLAGS
    if [[ $? != 0 ]] ; then
       warn "mxml build failed.  Giving up"
       return 1
    fi
    info "Installing ADIOS . . ."

    $MAKE install
    if [[ $? != 0 ]] ; then
       warn "mxml build (make install) failed.  Giving up"
       return 1
    fi

    if [[ "$DO_GROUP" == "yes" ]] ; then
       chmod -R ug+w,a+rX "$VISITDIR/mxml"
       chgrp -R ${GROUP} "$VISITDIR/mxml"
    fi
    cd "$START_DIR"
    info "Done with mxml"
    return 0
}

function bv_mxml_is_enabled
{
    if [[ $DO_MXML == "yes" ]]; then
        return 1    
    fi
    return 0
}

function bv_mxml_is_installed
{
    check_if_installed "mxml" $MXML_VERSION
    if [[ $? == 0 ]] ; then
        return 1
    fi
    return 0
}

function bv_mxml_build
{
    if [[ "$DO_MXML" == "yes" ]] ; then
        check_if_installed "mxml" $MXML_VERSION
        if [[ $? == 0 ]] ; then
            info "Skipping build of MXML"
        else
            build_mxml
            if [[ $? != 0 ]] ; then
                 error "Unable to build or install mxml.  Bailing out."
            fi
            info "Done building mxml"
        fi
    fi
}

function bv_mxml_graphical
{
local graphical_out="MXML    $MXML_VERSION($MXML_FILE)     $ON_MXML"
#echo "$graphical_out"
echo ""
}

