function bv_boxlib_initialize
{
    export DO_BOXLIB="no"
}

function bv_boxlib_enable
{
    DO_BOXLIB="yes"
}

function bv_boxlib_disable
{
    DO_BOXLIB="no"
}

function bv_boxlib_depends_on
{
    echo ""
}

function bv_boxlib_info
{
    export BOXLIB_VERSION=${BOXLIB_VERSION:-"1.3.5"}
    export BOXLIB_FILE=${BOXLIB_FILE:-"ccse-${BOXLIB_VERSION}.tar.gz"}
    export BOXLIB_COMPATIBILITY_VERSION=${BOXLIB_COMPATIBILITY_VERSION:-"1.3.5"}
    export BOXLIB_URL=${BOXLIB_URL:-"https://ccse.lbl.gov/Software/tarfiles/"}
    export BOXLIB_BUILD_DIR=${BOXLIB_BUILD_DIR:-"ccse-${BOXLIB_VERSION}/Src/C_BaseLib"}
    export BOXLIB_SHA256_CHECKSUM="2dd2496d27dc84d9171be06b44e3968fa481867d936174e7d49a547da5f6f755"
}

function bv_boxlib_print
{
    printf "%s%s\n" "BOXLIB_FILE=" "${BOXLIB_FILE}"
    printf "%s%s\n" "BOXLIB_VERSION=" "${BOXLIB_VERSION}"
    printf "%s%s\n" "BOXLIB_COMPATIBILITY_VERSION=" "${BOXLIB_COMPATIBILITY_VERSION}"
    printf "%s%s\n" "BOXLIB_BUILD_DIR=" "${BOXLIB_BUILD_DIR}"
}

function bv_boxlib_print_usage
{
    printf "%-20s %s [%s]\n" "--boxlib"  "Build Boxlib" "$DO_BOXLIB" 
}

function bv_boxlib_host_profile
{
    if [[ "$DO_BOXLIB" == "yes" ]] ; then
        echo >> $HOSTCONF
        echo "##" >> $HOSTCONF
        echo "## Boxlib" >> $HOSTCONF
        echo "##" >> $HOSTCONF
        echo \
            "VISIT_OPTION_DEFAULT(VISIT_BOXLIB_DIR \${VISITHOME}/boxlib/$BOXLIB_VERSION/\${VISITARCH})" \
            >> $HOSTCONF
    fi

}

function bv_boxlib_ensure
{
    if [[ "$DO_BOXLIB" == "yes" ]] ; then
        ensure_built_or_ready "boxlib" $BOXLIB_VERSION $BOXLIB_BUILD_DIR $BOXLIB_FILE $BOXLIB_URL
        if [[ $? != 0 ]] ; then
            ANY_ERRORS="yes"
            DO_BOXLIB="no"
            error "Unable to build Boxlib.  ${BOXLIB_FILE} not found."
        fi
    fi
}
# *************************************************************************** #
#                         Function 8.8, build_boxlib                          #
# *************************************************************************** #

function apply_nan_inf_patch
{
    patch -p0 << \EOF
diff -c a/Src/C_BaseLib/FArrayBox.cpp ccse-1.3.5/Src/C_BaseLib/FArrayBox.cpp
*** a/Src/C_BaseLib/FArrayBox.cpp
--- ccse-1.3.5/Src/C_BaseLib/FArrayBox.cpp
***************
*** 21,30 ****
  #include <BL_CXX11.H>
  #include <MemPool.H>
  
- #ifdef BL_Darwin
  using std::isinf;
  using std::isnan;
- #endif
  
  #if defined(DEBUG) || defined(BL_TESTING)
  bool FArrayBox::do_initval = true;
--- 21,28 ----
EOF
    if [[ $? != 0 ]] ; then
        warn "boxlib patch failed."
        return 1
    fi

    return 0;

}

function apply_endian_patch
{
    patch -p0 << \EOF
--- ccse-1.3.5/Src/C_BaseLib/FPC.cpp	2016-02-18 18:15:45.000000000 -0800
+++ ccse-1.3.5-patched/Src/C_BaseLib/FPC.cpp	2024-02-16 09:03:09.837102000 -0800
@@ -23,7 +23,8 @@
 IntDescriptor&
 FPC::NativeLongDescriptor ()
 {
-#if defined(__i486__) || \
+#if (defined(__BYTE_ORDER__)&&(__BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__)) || \
+    defined(__i486__) || \
     defined(WIN32) || \
     defined(i386) || \
     defined(__i386__) || \
@@ -33,7 +34,8 @@
     static const IntDescriptor nld(sizeof(long), IntDescriptor::ReverseOrder);
 #endif
 
-#if defined(__sgi) || \
+#if (defined(__BYTE_ORDER__)&&(__BYTE_ORDER__ == __ORDER_BIG_ENDIAN__)) || \
+    defined(__sgi) || \
     defined(__sun) || \
     defined(_AIX)  || \
     defined(__ppc__) || \
@@ -50,7 +52,8 @@
 RealDescriptor&
 FPC::NativeRealDescriptor ()
 {
-#if defined(__i486__) || \
+#if (defined(__BYTE_ORDER__)&&(__BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__)) || \
+    defined(__i486__) || \
     defined(WIN32) || \
     defined(i386) || \
     defined(__i386__) || \
@@ -63,7 +66,8 @@
 #endif
 #endif
 
-#if defined(__sgi) || \
+#if (defined(__BYTE_ORDER__)&&(__BYTE_ORDER__ == __ORDER_BIG_ENDIAN__)) || \
+    defined(__sgi) || \
     defined(__sun) || \
     defined(_AIX)  || \
     defined(__ppc__) || \
@@ -85,7 +89,8 @@
 RealDescriptor&
 FPC::Native32RealDescriptor ()
 {
-#if defined(__i486__) || \
+#if (defined(__BYTE_ORDER__)&&(__BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__)) || \
+    defined(__i486__) || \
     defined(WIN32) || \
     defined(i386) || \
     defined(__i386__) || \
@@ -94,7 +99,8 @@
     static const RealDescriptor n32rd(ieee_float, reverse_float_order, 4);
 #endif
 
-#if defined(__sgi) || \
+#if (defined(__BYTE_ORDER__)&&(__BYTE_ORDER__ == __ORDER_BIG_ENDIAN__)) || \
+    defined(__sgi) || \
     defined(__sun) || \
     defined(_AIX)  || \
     defined(__ppc__) || \
@@ -138,6 +144,7 @@
       defined(__hpux)   || \
       defined(powerpc)  || \
       defined(_MSC_VER) || \
+      defined(__BYTE_ORDER__) || \
       defined(_AIX))
 #error We do not yet support FAB I/O on this machine
 #endif
EOF
    if [[ $? != 0 ]] ; then
        warn "endianness patch failed."
        return 1
    fi

    return 0;
}


function apply_darwin_patch_1
{
    patch -p0 << \EOF
--- ccse-1.3.5/Src/C_BaseLib/VisMF.cpp	2021-03-30 08:21:05.000000000 -0700
+++ VisMF.cpp.new	2021-03-30 08:21:16.000000000 -0700
@@ -200,16 +200,20 @@
     {
         ar[i].resize(M);
 
+        std::string line;
+        std::string delimiter = ",";
+        is >> line;
+
         for (long j = 0; j < M; j++)
         {
+            std::string nextValue = line.substr(0, line.find(delimiter));
+            line = line.substr(line.find(delimiter) + 1);
 #ifdef BL_USE_FLOAT
-            is >> dtemp >> ch;
+            dtemp = std::atof(nextValue.c_str()); 
             ar[i][j] = static_cast<Real>(dtemp);
 #else
-            is >> ar[i][j] >> ch;
+            ar[i][j] = std::atof(nextValue.c_str());
 #endif
-	    if ( ch != ',' ) 
-	      BoxLib::Error("Expected a ',' got something else");
         }
     }
 
EOF
    if [[ $? != 0 ]] ; then
        warn "darwin patch failed."
        return 1
    fi

    return 0;
}

function apply_boxlib_patch
{
    apply_nan_inf_patch
    if [[ $? != 0 ]]; then
        return 1
    fi

    apply_endian_patch
    if [[ $? != 0 ]]; then
        return 1
    fi

    if [[ "$OPSYS" == "Darwin" ]]; then
        apply_darwin_patch_1
        if [[ $? != 0 ]] ; then
            return 1
        fi
    fi

    return 0;
}

function build_boxlib
{
    #
    # Prepare build dir
    #
    prepare_build_dir $BOXLIB_BUILD_DIR $BOXLIB_FILE
    untarred_boxlib=$?
    if [[ $untarred_boxlib == -1 ]] ; then
        warn "Unable to prepare Boxlib Build Directory. Giving Up"
        return 1
    fi

    #
    # Apply patches
    #
    info "Patching Boxlib . . ."
    apply_boxlib_patch
    if [[ $? != 0 ]] ; then
        if [[ $untarred_boxlib == 1 ]] ; then
            warn "Giving up on Boxlib build because the patch failed."
            return 1
        else
            warn "Patch failed, but continuing.  I believe that this script\n" \
                 "tried to apply a patch to an existing directory that had\n" \
                 "already been patched ... that is, the patch is\n" \
                 "failing harmlessly on a second application."
        fi
    fi

    cd $BOXLIB_BUILD_DIR || error "Can't cd to BoxLib build dir."

    #
    # Build BoxLib
    #
    info "Building Boxlib. . . (~4 minutes)"

    set -x
    if [[ "$OPSYS" == "AIX" ]]; then
        $MAKE -f GNUmakefile CXX="$CXX_COMPILER" CC="$C_COMPILER" \
              CCFLAGS="$CFLAGS $C_OPT_FLAGS" CXXFLAGS="$CXXFLAGS $CXX_OPT_FLAGS" \
              DEBUG="FALSE" DIM=3 COMP="xlC" USE_MPI="FALSE" \
              BL_NO_FORT="TRUE" || error "Boxlib build failed. Giving up"

        $MAKE -f GNUmakefile CXX="$CXX_COMPILER" CC="$C_COMPILER" \
              CCFLAGS="$CFLAGS $C_OPT_FLAGS" CXXFLAGS="$CXXFLAGS $CXX_OPT_FLAGS" \
              DEBUG="FALSE" DIM=2 COMP="xlC" USE_MPI="FALSE" \
              BL_NO_FORT="TRUE" || error "Boxlib build failed. Giving up"
    elif [[ "$OPSYS" == "Darwin" ]]; then
        $MAKE -f GNUmakefile CXX="$CXX_COMPILER" CC="$C_COMPILER" \
              CCFLAGS="$CFLAGS $C_OPT_FLAGS" CXXFLAGS="$CXXFLAGS $CXX_OPT_FLAGS" \
              DEBUG="FALSE" DIM=3 USE_MPI="FALSE" BL_MANGLE_SYMBOLS_WITH_DIM="TRUE" \
              BL_NO_FORT="TRUE" || error "Boxlib build failed. Giving up"

        $MAKE -f GNUmakefile CXX="$CXX_COMPILER" CC="$C_COMPILER" \
              CCFLAGS="$CFLAGS $C_OPT_FLAGS" CXXFLAGS="$CXXFLAGS $CXX_OPT_FLAGS" \
              DEBUG="FALSE" DIM=2 USE_MPI="FALSE" BL_MANGLE_SYMBOLS_WITH_DIM="TRUE" \
              BL_NO_FORT="TRUE" || error "Boxlib build failed. Giving up"
    else
        $MAKE -f GNUmakefile CXX="$CXX_COMPILER" CC="$C_COMPILER" \
              CCFLAGS="$CFLAGS $C_OPT_FLAGS" CXXFLAGS="$CXXFLAGS $CXX_OPT_FLAGS" \
              DEBUG="FALSE" DIM=3 USE_MPI="FALSE" \
              BL_NO_FORT="TRUE" || error "Boxlib build failed. Giving up"

        $MAKE -f GNUmakefile CXX="$CXX_COMPILER" CC="$C_COMPILER" \
              CCFLAGS="$CFLAGS $C_OPT_FLAGS" CXXFLAGS="$CXXFLAGS $CXX_OPT_FLAGS" \
              DEBUG="FALSE" DIM=2 USE_MPI="FALSE" \
              BL_NO_FORT="TRUE" || error "Boxlib build failed. Giving up"
    fi
    set +x
    #
    # Create dynamic library for Darwin
    #
    if [[ "$DO_STATIC_BUILD" == "no" && "$OPSYS" == "Darwin" ]]; then
        INSTALLNAMEPATH="$VISITDIR/boxlib/$BOXLIB_VERSION/$VISITARCH/lib"

        $CXX_COMPILER -dynamiclib -o libbox3D.$SO_EXT o/3d.Darwin.*/*.o \
                      -lSystem -Wl,-headerpad_max_install_names \
                      -Wl,-install_name,$INSTALLNAMEPATH/libbox3D.$SO_EXT \
                      -Wl,-compatibility_version,$BOXLIB_COMPATIBILITY_VERSION \
                      -Wl,-current_version,$BOXLIB_VERSION || \
            error "Creation of dynamic 3D Boxlib library failed. Giving up!"
        $CXX_COMPILER -dynamiclib -o libbox2D.$SO_EXT o/2d.Darwin.*/*.o \
                      -lSystem -Wl,-headerpad_max_install_names \
                      -Wl,-install_name,$INSTALLNAMEPATH/libbox2D.$SO_EXT \
                      -Wl,-compatibility_version,$BOXLIB_COMPATIBILITY_VERSION \
                      -Wl,-current_version,$BOXLIB_VERSION || \
            error "Creation of dynamic 2D Boxlib library failed. Giving up!"
        boxlib_ext=$SO_EXT
    else
        mv libbox3d.*.a libbox3D.a
        mv libbox2d.*.a libbox2D.a
        boxlib_ext=a
    fi

    #
    # Install into the VisIt third party location.
    #
    info "Installing Boxlib . . ."

    mkdir "$VISITDIR/boxlib"
    mkdir "$VISITDIR/boxlib/$BOXLIB_VERSION"
    mkdir "$VISITDIR/boxlib/$BOXLIB_VERSION/$VISITARCH"
    mkdir "$VISITDIR/boxlib/$BOXLIB_VERSION/$VISITARCH/include"
    mkdir "$VISITDIR/boxlib/$BOXLIB_VERSION/$VISITARCH/lib"

    cp libbox3D.$boxlib_ext \
       "$VISITDIR/boxlib/$BOXLIB_VERSION/$VISITARCH/lib/" || \
        error "Boxlib install failed. Giving up!"

    cp libbox2D.$boxlib_ext \
       "$VISITDIR/boxlib/$BOXLIB_VERSION/$VISITARCH/lib/" || \
        error "Boxlib install failed. Giving up!"

    cp *.H "$VISITDIR/boxlib/$BOXLIB_VERSION/$VISITARCH/include" || \
        error "Boxlib install failed. Giving up!"

    if [[ "$DO_GROUP" == "yes" ]] ; then
        chmod -R ug+w,a+rX "$VISITDIR/boxlib"
        chgrp -R ${GROUP} "$VISITDIR/boxlib"
    fi

    cd "$START_DIR"
    info "Done with BoxLib"
    return 0
}

function bv_boxlib_is_enabled
{
    if [[ $DO_BOXLIB == "yes" ]]; then
        return 1    
    fi
    return 0
}

function bv_boxlib_is_installed
{
    check_if_installed "boxlib" $BOXLIB_VERSION
    if [[ $? == 0 ]] ; then
        return 1
    fi
    return 0
}

function bv_boxlib_build
{
    cd "$START_DIR"
    if [[ "$DO_BOXLIB" == "yes" ]] ; then
        check_if_installed "boxlib" $BOXLIB_VERSION
        if [[ $? == 0 ]] ; then
            info "Skipping Boxlib build.  Boxlib is already installed."
        else
            info "Building Boxlib (~2 minutes)"
            build_boxlib
            if [[ $? != 0 ]] ; then
                error "Unable to build or install Boxlib.  Bailing out."
            fi
            info "Done building Boxlib"
        fi
    fi
}
