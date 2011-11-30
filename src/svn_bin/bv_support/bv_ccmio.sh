function bv_ccmio_initialize
{
export DO_CCMIO="no"
export ON_CCMIO="off"
}

function bv_ccmio_enable
{
DO_CCMIO="yes"
ON_CCMIO="on"
}

function bv_ccmio_disable
{
DO_CCMIO="no"
ON_CCMIO="off"
}

function bv_ccmio_depends_on
{
return ""
}

function bv_ccmio_info
{
export CCMIO_FILE=${CCMIO_FILE:-"libccmio-2.6.1.tar.gz"}
export CCMIO_VERSION=${CCMIO_VERSION:-"2.6.1"}
export CCMIO_COMPATIBILITY_VERSION=${CCMIO_COMPATIBILITY_VERSION:-"2.0"}
export CCMIO_BUILD_DIR=${CCMIO_BUILD_DIR:-"libccmio-2.6.1"}
}

function bv_ccmio_print
{
  printf "%s%s\n" "CCMIO_FILE=" "${CCMIO_FILE}"
  printf "%s%s\n" "CCMIO_VERSION=" "${CCMIO_VERSION}"
  printf "%s%s\n" "CCMIO_COMPATIBILITY_VERSION=" "${CCMIO_COMPATIBILITY_VERSION}"
  printf "%s%s\n" "CCMIO_BUILD_DIR=" "${CCMIO_BUILD_DIR}"
}

function bv_ccmio_print_usage
{
printf "%-15s %s [%s]\n" "--ccmio"   "Build Star-CD support" "$DO_CCMIO"
}

function bv_ccmio_graphical
{
local graphical_out="CCMIO    $CCMIO_VERSION($CCMIO_FILE) $ON_CCMIO"
echo $graphical_out
}

function bv_ccmio_host_profile
{
    if [[ "$DO_CCMIO" == "yes" ]] ; then
        echo >> $HOSTCONF
        echo "##" >> $HOSTCONF
        echo "## CCMIO" >> $HOSTCONF
        echo "##" >> $HOSTCONF
        echo \
        "VISIT_OPTION_DEFAULT(VISIT_CCMIO_DIR \${VISITHOME}/ccmio/$CCMIO_VERSION/\${VISITARCH})" \
        >> $HOSTCONF
    fi
}

function bv_ccmio_ensure
{
    if [[ "$DO_CCMIO" == "yes" ]] ; then
        ensure_built_or_ready "ccmio"  $CCMIO_VERSION $CCMIO_BUILD_DIR  $CCMIO_FILE
        if [[ $? != 0 ]] ; then
            ANY_ERRORS="yes"
            DO_CCMIO="no"
            error "Unable to build CCMIO.  ${CCMIO_FILE} not found."
        fi
    fi
}

function bv_ccmio_dry_run
{
  if [[ "$DO_CCMIO" == "yes" ]] ; then
    echo "Dry run option not set for ccmio."
  fi
}
# *************************************************************************** #
#                         Function 8.11, build_ccmio                          #
# *************************************************************************** #

function apply_ccmio_261_darwin_patch
{
   patch -p0 << \EOF
diff -c a/config/config.gnu.to.star libccmio-2.6.1/config/config.gnu.to.star
*** a/config/config.gnu.to.star
--- libccmio-2.6.1/config/config.gnu.to.star
*************** case ${1:-null}-${2:-null} in
*** 34,39 ****
--- 34,40 ----
      x86_64-unknown-linux-gnu-null)     echo linux64_2.4-x86-glibc_2.2.5     ;;
      ppc64-unknown-linux-gnu-null)      echo linux64_2.6-pwr4-glibc_2.3.3    ;;
      i386-apple-darwin8-null)           echo i386-apple-darwin8              ;;
+     powerpc-apple-darwin7-null)        echo powerpc-apple-darwin7           ;;
      *)                                 echo unknown                         ;;
  esac
  
diff -c a/config/config.system libccmio-2.6.1/config/config.system 
*** a/config/config.system
--- libccmio-2.6.1/config/config.system
*************** case ${SYSTEM} in
*** 85,93 ****
      ppc64-unknown-linux-gnu)
  	echo ppc64-unknown-linux-gnu ;;
  
!     i386-apple-darwin8.11.1)
  	echo i386-apple-darwin8 ;;
  
      *)
          echo unknown
          echo System type ${SYSTEM} not supported! 1>&2 ;;
--- 85,96 ----
      ppc64-unknown-linux-gnu)
  	echo ppc64-unknown-linux-gnu ;;
  
!     i386-apple-darwin8* | i386-apple-darwin9* | i386-apple-darwin10*)
  	echo i386-apple-darwin8 ;;
  
+     powerpc-apple-darwin7* | powerpc-apple-darwin8* | powerpc-apple-darwin9* )
+ 	echo powerpc-apple-darwin7 ;;
+ 
      *)
          echo unknown
          echo System type ${SYSTEM} not supported! 1>&2 ;;
EOF
   if [[ $? != 0 ]] ; then
       warn "Unable to patch CCMIO. Wrong version?"
       return 1
   fi

   return 0
}

function apply_ccmio_261_linux_ppc_patch
{
  patch -p1 << \EOF
--- visit-build-old/libccmio-2.6.1/config/linux64_2.6-pwr4-glibc_2.3.3/qmake.conf	2009-03-19 21:31:31.207392275 -0400
+++ visit-build/libccmio-2.6.1/config/linux64_2.6-pwr4-glibc_2.3.3/qmake.conf	2009-03-19 21:31:54.522915173 -0400
@@ -72,7 +72,7 @@
 QMAKE_UIC		= $(QTDIR)/bin/uic
 
 QMAKE_AR		= ar cq
-QMAKE_RANLIB		= ranlib -X64
+QMAKE_RANLIB		= ranlib
 
 QMAKE_TAR		= tar -cf
 QMAKE_GZIP		= gzip -9f
EOF
   if [[ $? != 0 ]] ; then
        warn "Unable to apply linux ppc patch to CCMIO 2.6.1."
        return 1
   else
        return 0
   fi
}

function apply_ccmio_261_aix_patch
{
  patch -p0 <<\EOF
diff -c a/config/aix64_5.1-pwr4/qmake.conf libccmio-2.6.1/config/aix64_5.1-pwr4/qmake.conf
*** a/config/aix64_5.1-pwr4/qmake.conf
--- libccmio-2.6.1/config/aix64_5.1-pwr4/qmake.conf
***************
*** 15,21 ****
  QMAKE_LEXFLAGS		= 
  QMAKE_YACC		= yacc
  QMAKE_YACCFLAGS		= -d
! QMAKE_CFLAGS		= -q64 -ma -qrndsngl -qnomaf -qstrict -DLARGE_FILES
  # -qwarn64 turns on too many bogus warnings and shadows real warnings
  #QMAKE_CFLAGS_WARN_ON	= -qwarn64
  QMAKE_CFLAGS_WARN_ON    =
--- 15,21 ----
  QMAKE_LEXFLAGS		= 
  QMAKE_YACC		= yacc
  QMAKE_YACCFLAGS		= -d
! QMAKE_CFLAGS		= -ma -qrndsngl -qnomaf -qstrict -DLARGE_FILES
  # -qwarn64 turns on too many bogus warnings and shadows real warnings
  #QMAKE_CFLAGS_WARN_ON	= -qwarn64
  QMAKE_CFLAGS_WARN_ON    =
***************
*** 49,61 ****
  QMAKE_LINK		= xlC
  QMAKE_LINK_THREAD	= xlC_r
  QMAKE_LINK_SHLIB	= ld
! QMAKE_LINK_SHLIB_CMD	= makeC++SharedLib -p 0 -X64 \
  			    -o $(TARGETD) \
  			    $(LFLAGS) $(OBJECTS) $(OBJMOC) $(LIBS); \
  			  $(AR) lib$(QMAKE_TARGET).a $(TARGETD); \
  			  $(RANLIB) lib$(QMAKE_TARGET).a; \
  			  mv lib$(QMAKE_TARGET).a $(DESTDIR)
! QMAKE_LFLAGS		= -q64 -qnotempinc
  QMAKE_LFLAGS_RELEASE	=
  QMAKE_LFLAGS_DEBUG	=
  QMAKE_LFLAGS_SHLIB	=
--- 49,61 ----
  QMAKE_LINK		= xlC
  QMAKE_LINK_THREAD	= xlC_r
  QMAKE_LINK_SHLIB	= ld
! QMAKE_LINK_SHLIB_CMD	= makeC++SharedLib -p 0 \
  			    -o $(TARGETD) \
  			    $(LFLAGS) $(OBJECTS) $(OBJMOC) $(LIBS); \
  			  $(AR) lib$(QMAKE_TARGET).a $(TARGETD); \
  			  $(RANLIB) lib$(QMAKE_TARGET).a; \
  			  mv lib$(QMAKE_TARGET).a $(DESTDIR)
! QMAKE_LFLAGS		= -qnotempinc
  QMAKE_LFLAGS_RELEASE	=
  QMAKE_LFLAGS_DEBUG	=
  QMAKE_LFLAGS_SHLIB	=
***************
*** 77,84 ****
  QMAKE_MOC		= $(QTDIR)/bin/moc
  QMAKE_UIC		= $(QTDIR)/bin/uic
  
! QMAKE_AR		= ar -X64 cq
! QMAKE_RANLIB		= ranlib -X64
  
  QMAKE_TAR		= tar -cf
  QMAKE_GZIP		= gzip -9f
--- 77,84 ----
  QMAKE_MOC		= $(QTDIR)/bin/moc
  QMAKE_UIC		= $(QTDIR)/bin/uic
  
! QMAKE_AR		= ar cq
! QMAKE_RANLIB		= ranlib
  
  QMAKE_TAR		= tar -cf
  QMAKE_GZIP		= gzip -9f
EOF
   if [[ $? != 0 ]] ; then
        warn "Unable to apply aix patch to CCMIO 2.6.1."
        return 1
   else
        return 0
   fi
}

function apply_ccmio_261_patch
{
   if [[ "$OPSYS" == "Darwin" ]]; then
       apply_ccmio_261_darwin_patch
       if [[ $? != 0 ]] ; then
           return 1
       fi
   fi
   if [[ `uname -m` == "ppc64" ]]; then
       apply_ccmio_261_linux_ppc_patch
       if [[ $? != 0 ]] ; then
           return 1
       fi
   fi
   apply_ccmio_261_aix_patch
   if [[ $? != 0 ]] ; then
       return 1
   fi

   return 0
}

function apply_ccmio_patch
{
   if [[ ${CCMIO_VERSION} == 2.6.1 ]] ; then
       apply_ccmio_261_patch
       if [[ $? != 0 ]] ; then
           return 1
       fi
   fi

   return 0
}

function build_ccmio
{
    #
    # Prepare build dir
    #
    prepare_build_dir $CCMIO_BUILD_DIR $CCMIO_FILE
    untarred_ccmio=$?
    if [[ $untarred_ccmio == -1 ]] ; then
       warn "Unable to prepare CCMIO Build Directory. Giving Up"
       return 1
    fi

    #
    # Apply patches
    #
    info "Patching CCMIO . . ."
    apply_ccmio_patch
    if [[ $? != 0 ]] ; then
       if [[ $untarred_ccmio == 1 ]] ; then
          warn "Giving up on CCMIO build because the patch failed."
          return 1
       else
          warn "Patch failed, but continuing.  I believe that this script" \
               "tried to apply a patch to an existing directory which had" \
               "already been patched ... that is, that the patch is" \
               "failing harmlessly on a second application."
       fi
    fi

    #
    # Do some extra copies
    #
    if [[ ${CCMIO_VERSION} == 2.6.1 ]] ; then
       if [[ "$OPSYS" == "Darwin" ]]; then
           # Check for PPC
           MACH=$(uname -m)
           if [[ "$MACH" == "Power Macintosh" ]] ; then
               mkdir ${CCMIO_BUILD_DIR}/config/powerpc-apple-darwin7
               cp ${QT_BUILD_DIR}/bin/qmake \
	           ${CCMIO_BUILD_DIR}/config/powerpc-apple-darwin7
               cp ${CCMIO_BUILD_DIR}/config/i386-apple-darwin8/qmake.conf \
                   ${CCMIO_BUILD_DIR}/config/powerpc-apple-darwin7
               cp ${QT_BUILD_DIR}/mkspecs/${QT_PLATFORM}/qplatformdefs.h \
                   ${CCMIO_BUILD_DIR}/config/powerpc-apple-darwin7
           fi
       fi
    fi
    #
    info "Configuring CCMIO . . ."
    cd $CCMIO_BUILD_DIR || error "Can't cd to CCMIO build dir."

    #
    # Build CCMIO
    #
    info "Building CCMIO . . . (~1 minutes)"

    if [[ "$OPSYS" == "Darwin" ]]; then
        env RELEASE=1 SHARED=1 $MAKE CXX="$CXX_COMPILER" CC="$C_COMPILER" \
           CFLAGS="$CFLAGS $C_OPT_FLAGS" CXXFLAGS="$CXXFLAGS $CXX_OPT_FLAGS"
    else
        env RELEASE=1 STATIC=1 $MAKE CXX="$CXX_COMPILER" CC="$C_COMPILER" \
           CFLAGS="$CFLAGS $C_OPT_FLAGS" CXXFLAGS="$CXXFLAGS $CXX_OPT_FLAGS"
    fi
    if [[ $? != 0 ]] ; then
       echo "CCMIO build failed.  Giving up"
       return 1
    fi
    info "Installing CCMIO . . ."

    mkdir $VISITDIR/ccmio
    mkdir $VISITDIR/ccmio/${CCMIO_VERSION}
    mkdir $VISITDIR/ccmio/${CCMIO_VERSION}/$VISITARCH
    mkdir $VISITDIR/ccmio/${CCMIO_VERSION}/$VISITARCH/lib
    mkdir $VISITDIR/ccmio/${CCMIO_VERSION}/$VISITARCH/include
    ln -s $VISITDIR/ccmio/${CCMIO_VERSION}/$VISITARCH/include \
        $VISITDIR/ccmio/${CCMIO_VERSION}/$VISITARCH/include/libccmio
    cp libccmio/ccmio*.h $VISITDIR/ccmio/${CCMIO_VERSION}/$VISITARCH/include

    if [[ "$DO_STATIC_BUILD" == "no" && "$OPSYS" == "Darwin" ]]; then
        #
        # Make dynamic executable
        #
        info "Creating dynamic libraries for CCMIO . . ."

        INSTALLNAMEPATH="$VISITDIR/ccmio/${CCMIO_VERSION}/$VISITARCH/lib"

        cp lib/release-shared/libadf.dylib .
        cp lib/release-shared/libccmio.dylib .
        install_name_tool -id \
          "$INSTALLNAMEPATH/libadf.$SO_EXT" "libadf.$SO_EXT"
        install_name_tool -id \
          "$INSTALLNAMEPATH/libccmio.$SO_EXT" "libccmio.$SO_EXT"
        install_name_tool -change \
          libadf.dylib.5.01.000 "$INSTALLNAMEPATH/libadf.$SO_EXT" \
         "libccmio.$SO_EXT"
        cp "libadf.$SO_EXT" \
          "$VISITDIR/ccmio/${CCMIO_VERSION}/$VISITARCH/lib/libadf.$SO_EXT"
        cp "libccmio.$SO_EXT" \
          "$VISITDIR/ccmio/${CCMIO_VERSION}/$VISITARCH/lib/libccmio.$SO_EXT"
    else
        cp `ls lib/*/release-static/libadf.a` \
           $VISITDIR/ccmio/${CCMIO_VERSION}/$VISITARCH/lib
        cp `ls lib/*/release-static/libccmio.a` \
           $VISITDIR/ccmio/${CCMIO_VERSION}/$VISITARCH/lib
    fi

    if [[ "$DO_GROUP" == "yes" ]] ; then
       chmod -R ug+w,a+rX "$VISITDIR/ccmio"
       chgrp -R ${GROUP} "$VISITDIR/ccmio"
    fi
    cd "$START_DIR"
    info "Done with CCMIO"
    return 0
}


function bv_ccmio_build
{
cd "$START_DIR"
if [[ "$DO_CCMIO" == "yes" ]] ; then
    check_if_installed "ccmio" $CCMIO_VERSION
    if [[ $? == 0 ]] ; then
        info "Skipping CCMIO build.  CCMIO is already installed."
    else
        info "Building CCMIO (~1 minutes)"
        build_ccmio
        if [[ $? != 0 ]] ; then
            error "Unable to build or install CCMIO.  Bailing out."
        fi
        info "Done building CCMIO"
    fi
fi
}
