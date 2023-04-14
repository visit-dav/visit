function bv_boost_initialize
{
    export DO_BOOST="no"
    export USE_SYSTEM_BOOST="no"
    add_extra_commandline_args "boost" "alt-boost-dir" 1 "Use alternative directory for boost"
}

function bv_boost_enable
{
    DO_BOOST="yes"
}

function bv_boost_disable
{
    DO_BOOST="no"
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
        BOOST_INSTALL_DIR="${VISITDIR}/boost/${BOOST_VERSION}/${VISITARCH}"
    fi
}

function bv_boost_info
{
    export BOOST_VERSION=${BOOST_VERSION:-"1_67_0"}
    export BOOST_FILE=${BOOST_FILE:-"boost_${BOOST_VERSION}.tar.gz"}
    export BOOST_COMPATIBILITY_VERSION=${BOOST_COMPATIBILITY_VERSION:-"1_67"}
    export BOOST_URL=${BOOST_URL:-"http://sourceforge.net/projects/boost/files/boost/1.67.0"}
    export BOOST_BUILD_DIR=${BOOST_BUILD_DIR:-"boost_${BOOST_VERSION}"}
    export BOOST_MD5_CHECKSUM="4850fceb3f2222ee011d4f3ea304d2cb"
    export BOOST_SHA256_CHECKSUM="8aa4e330c870ef50a896634c931adf468b21f8a69b77007e45c444151229f665"
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
    printf "%-20s %s [%s]\n" "--boost" "Build BOOST" "${DO_BOOST}"
    printf "%-20s %s [%s]\n" "--alt-boost-dir" "Use Boost from an alternative directory"
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
                "VISIT_OPTION_DEFAULT(VISIT_BOOST_DIR \${VISITHOME}/boost/\${BOOST_VERSION}/\${VISITARCH})" \
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

function apply_boost_ppc_rounding_control_patch
{
   # resolves a C++11 narrowing error

   patch -p0 << \EOF
*** boost/numeric/interval/detail/ppc_rounding_control.hpp.orig       Mon Aug 17 15:26:50 2020
--- boost/numeric/interval/detail/ppc_rounding_control.hpp    Mon Aug 17 15:27:12 2020
***************
*** 28,37 ****
    double dmode;
  } rounding_mode_struct;
  
! static const rounding_mode_struct mode_upward      = { 0xFFF8000000000002LL };
! static const rounding_mode_struct mode_downward    = { 0xFFF8000000000003LL };
! static const rounding_mode_struct mode_to_nearest  = { 0xFFF8000000000000LL };
! static const rounding_mode_struct mode_toward_zero = { 0xFFF8000000000001LL };
  
  struct ppc_rounding_control
  {
--- 28,37 ----
    double dmode;
  } rounding_mode_struct;
  
! static const rounding_mode_struct mode_upward      = { (long long int)0xFFF8000000000002LL };
! static const rounding_mode_struct mode_downward    = { (long long int)0xFFF8000000000003LL };
! static const rounding_mode_struct mode_to_nearest  = { (long long int)0xFFF8000000000000LL };
! static const rounding_mode_struct mode_toward_zero = { (long long int)0xFFF8000000000001LL };
  
  struct ppc_rounding_control
  {
EOF

   if [[ $? != 0 ]] ; then
      warn "boost patch for ppc_rounding_control failed."
      return 1
    fi
    return 0;
}

function apply_boost_fcoalesce_templates_patch
{
    info "Patching boost 1.67.0 for -fcoalesce-templates"
    patch -p0 << \EOF
--- tools/build/src/tools/darwin.jam.orig	2023-04-14 10:31:45.000000000 -0700
+++ tools/build/src/tools/darwin.jam	2023-04-07 10:15:50.000000000 -0700
@@ -138,10 +138,10 @@
     common.handle-options darwin : $(condition) : $(command) : $(options) ;
     
     # - GCC 4.0 and higher in Darwin does not have -fcoalesce-templates.
-    if $(real-version) < "4.0.0"
-    {
-        flags darwin.compile.c++ OPTIONS $(condition) : -fcoalesce-templates ;
-    }
+#    if $(real-version) < "4.0.0"
+#    {
+#        flags darwin.compile.c++ OPTIONS $(condition) : -fcoalesce-templates ;
+#    }
     # - GCC 4.2 and higher in Darwin does not have -Wno-long-double.
     if $(real-version) < "4.2.0"
     {
EOF

    if [[ $? != 0 ]] ; then
      warn "boost patch for -fcoalesce-templates failed."
      return 1
    fi
    return 0;
}

function apply_boost_clang_specific_storage_type
{
    info "Patching boost 1.67.0 for clang-specific storage type"
    patch -p0 << \EOF
--- boost/atomic/detail/ops_gcc_x86_dcas.hpp.orig	2023-04-14 10:41:18.000000000 -0700
+++ boost/atomic/detail/ops_gcc_x86_dcas.hpp	2023-04-07 10:37:15.000000000 -0700
@@ -401,13 +401,7 @@
 
     static BOOST_FORCEINLINE storage_type load(storage_type const volatile& storage, memory_order) BOOST_NOEXCEPT
     {
-#if defined(__clang__)
-
-        // Clang cannot allocate rax:rdx register pairs but it has sync intrinsics
-        storage_type value = storage_type();
-        return __sync_val_compare_and_swap(&storage, value, value);
-
-#elif defined(BOOST_ATOMIC_DETAIL_X86_NO_ASM_AX_DX_PAIRS)
+#if defined(BOOST_ATOMIC_DETAIL_X86_NO_ASM_AX_DX_PAIRS)
 
         // Some compilers can't allocate rax:rdx register pair either and also don't support 128-bit __sync_val_compare_and_swap
         uint64_t value_bits[2];
EOF

    if [[ $? != 0 ]] ; then
      warn "boost patch for clang-specific storage type failed."
      return 1
    fi
    return 0;
}

function apply_boost_patch
{
    apply_boost_ppc_rounding_control_patch
    if [[ $? != 0 ]] ; then
       return 1
    fi
    if [[ "$OPSYS" == "Darwin" ]]; then
        apply_boost_fcoalesce_templates_patch
        if [[ $? != 0 ]] ; then
           return 1
        fi
        apply_boost_clang_specific_storage_type
        if [[ $? != 0 ]] ; then
           return 1
        fi
    fi
    return 0;
}

# *************************************************************************** #
#                          Function 8.1, build_boost                           #
# *************************************************************************** #

function build_boost
{
    #
    # Prepare build dir
    #
    prepare_build_dir $BOOST_BUILD_DIR $BOOST_FILE
    untarred_boost=$?
    # 0, already exists, 1 untarred src, 2 error

    if [[ $untarred_boost == -1 ]] ; then
        warn "Unable to prepare BOOST Build Directory. Giving Up"
        return 1
    fi

    #
    # Apply patches
    #
    cd $BOOST_BUILD_DIR || error "Can't cd to BOOST build dir."
    apply_boost_patch
    if [[ $? != 0 ]] ; then
        if [[ $untarred_boost == 1 ]] ; then
            warn "Giving up on Boost build because the patch failed."
            return 1
        else
            warn "Patch failed, but continuing.  I believe that this script\n" \
                 "tried to apply a patch to an existing directory that had\n" \
                 "already been patched ... that is, the patch is\n" \
                 "failing harmlessly on a second application."
        fi
    fi

    # Get a list of libraries to build. This list of libraries is used
    # for the OS X name fix up.

    # This list must include dependent libraries also. For instance,
    # the serialization library requires the wserialization
    # library. So it too must be listed. However, it can not be in the
    # build_libs list otherwise boost barfs.
    libs=""

    if [[ "$DO_DAMARIS" == "yes" ]] ; then
        libs="$libs \
              date_time system filesystem"
    fi

    if [[ "$DO_NEKTAR_PLUS_PLUS" == "yes" ]] ; then
        libs="$libs \
              chrono iostreams thread date_time filesystem \
              system program_options regex timer"
    fi
    
#    if [[ "$DO_UINTAH" == "yes" ]] ; then
#        libs="$libs \
#              chrono filesystem wserialization serialization system thread signals date_time program_options"
#    fi

    # Remove all of the duplicate libs.
    libs=`echo $libs | tr ' ' '\n' | sort -u | tr '\n' ' ' | sed s'/.$//'`

    # Note: the library name 'wserialization' can not be in the list
    # of build libraries but must be part of the name fixup for OS X.
    build_libs=`echo $libs | sed s'/ wserialization//' | tr ' ' ','`

    if [[ "$build_libs" != ""  ]] ; then

        build_libs=" --with-libraries=\"$build_libs\" "

        info "Configuring BOOST . . . $build_libs"

        #        if [[ "$DO_STATIC_BUILD" == "yes" ]]; then
        #            cf_build_type="--disable-shared --enable-static"
        #        else
        #            cf_build_type="--enable-shared --disable-static"
        #        fi

        #        if [[ "$DO_THREAD_BUILD" == "yes" ]]; then
        #            cf_build_thread="--enable-threadsafe --with-pthread"
        #        else
        #            cf_build_thread=""
        #        fi

        # In order to ensure $FORTRANARGS is expanded to build the arguments to
        # configure, we wrap the invokation in 'sh -c "..."' syntax
        info "Invoking command to configure BOOST"

        set -x
        sh -c "./bootstrap.sh $build_libs \
            --prefix=\"$VISITDIR/boost/$BOOST_VERSION/$VISITARCH\" "

        set +x
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
        sh -c "./b2 install \
              --prefix=\"$VISITDIR/boost/$BOOST_VERSION/$VISITARCH\" "

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
            INSTALLNAMEPATH="${BOOST_INSTALL_DIR}/lib"

            for lib in $libs;
            do
                fulllibname=$INSTALLNAMEPATH/libboost_${lib}.${SO_EXT}

                install_name_tool -id $fulllibname $fulllibname

                # Find all the dependent libraries (more or less)
                deplibs=`otool -L $fulllibname | sed "s/(.*)//g"`

                for deplib in $deplibs;
                do
                    # Only get the libraries related to boost and not itself.
                    if [[ `echo $deplib | grep -c libboost_` == 1 && \
                                `echo $deplib | grep -c libboost_${lib}` == 0 ]] ; then

                        # Get the library name sans the directory path
                        deplibname=`echo $deplib | sed "s/.*\///"`
                        
                        # Set the library path
                        install_name_tool -change $deplib \
                                          ${INSTALLNAMEPATH}/$deplibname \
                                          $fulllibname

                    fi
                done            
            done
        fi

    else
        info "Installing BOOST . . . headers only"

        mkdir "$VISITDIR/boost"
        mkdir "$VISITDIR/boost/$BOOST_VERSION"
        mkdir "$VISITDIR/boost/$BOOST_VERSION/$VISITARCH"
        mkdir "$VISITDIR/boost/$BOOST_VERSION/$VISITARCH/include"

        cp -r boost $VISITDIR/boost/$BOOST_VERSION/$VISITARCH/include

        if [[ $? != 0 ]] ; then
            warn "BOOST install failed.  Giving up"
            return 1
        fi
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



# Notes to Windows developers on building boost:
# grab the .zip or .7z tarball and extract
# Open command prompt in the extracted boost_<version> directory
# To build everything and install to default C:\Boost location:
#   .\bootstrap
#   .\b2
#   .\b2 install
#
# To change install location, add --prefix="\path\to\boost" to
# all commands. (All might be overkill, but I experienced problems
# when specified for only bootrap or b2, so I added it to all).
#
# If you want shared libs only, linked with shared CRT, release only, 64-bit:
#
#   .\boostrap --prefix="C:\path\to\where\you\want\boost"
#   .\b2 --prefix="C:\path\to\where\you\want\boost" link=shared runtime-link=shared variant=release threading=multi address-model=64
#   .\b2 --prefix="C:\path\to\where\you\want\boost" link=shared runtime-link=shared variant=release threading=multi address-model=64 install
#
# If you only want a subset of the libraries add a '--with-<lib>' for each 
# library you want:
#   .\boostrap --prefix="C:\path\to\where\you\want\boost"
#   .\b2 --with-system --prefix="C:\path\to\where\you\want\boost" link=shared runtime-link=shared variant=release threading=multi address-model=64
#   .\b2 --with-system --prefix="C:\path\to\where\you\want\boost" link=shared runtime-link=shared variant=release threading=multi address-model=64 install
#
# Still not certain that all the arguments are needed for the 'install' step
# of running b2, but I ran into problems without using them, so ...
#
# I found the following links helpful, as well as running '.\b2 --help'
# once I had bootstrapped.
#
# http://www.boost.org/doc/libs/1_57_0/more/getting_started/windows.html#simplified-build-from-source
# 
# http://www.boost.org/build/doc/html/bbv2/overview/invocation.html
