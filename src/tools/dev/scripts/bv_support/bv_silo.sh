function bv_silo_initialize
{
    export DO_SILO="no"
    export DO_SILEX="no"
    add_extra_commandline_args "silo" "silex" 0 "Enable silex when building Silo"
}

function bv_silo_enable
{
    DO_SILO="yes"
}

function bv_silo_disable
{
    DO_SILO="no"
}

function bv_silo_silex
{
    info "Enabling silex in Silo build"
    DO_SILEX="yes"
    bv_silo_enable
}

function bv_silo_depends_on
{
    local depends_on=""

    if [[ "$DO_ZLIB" == "yes" ]] ; then
        depends_on="zlib"
    fi

    if [[ "$DO_HDF5" == "yes" ]] ; then
        depends_on="$depends_on hdf5"
    fi
    
    if [[ "$DO_SZIP" == "yes" ]] ; then
        depends_on="$depends_on szip"
    fi


    echo $depends_on
}

function bv_silo_info
{
    export SILO_VERSION=${SILO_VERSION:-"4.10.2"}
    export SILO_FILE=${SILO_FILE:-"silo-${SILO_VERSION}.tar.gz"}
    export SILO_COMPATIBILITY_VERSION=${SILO_COMPATIBILITY_VERSION:-"4.10.2"}
    export SILO_URL=${SILO_URL:-https://wci.llnl.gov/codes/silo/silo-${SILO_VERSION}}
    export SILO_BUILD_DIR=${SILO_BUILD_DIR:-"silo-${SILO_VERSION}"}
    export SILO_MD5_CHECKSUM="9ceac777a2f2469ac8cef40f4fab49c8"
    export SILO_SHA256_CHECKSUM="3af87e5f0608a69849c00eb7c73b11f8422fa36903dd14610584506e7f68e638"
}

function bv_silo_print
{
    printf "%s%s\n" "SILO_FILE=" "${SILO_FILE}"
    printf "%s%s\n" "SILO_VERSION=" "${SILO_VERSION}"
    printf "%s%s\n" "SILO_COMPATIBILITY_VERSION=" "${SILO_COMPATIBILITY_VERSION}"
    printf "%s%s\n" "SILO_BUILD_DIR=" "${SILO_BUILD_DIR}"
}

function bv_silo_print_usage
{
    printf "%-20s %s [%s]\n" "--silo" "Build Silo support" "$DO_SILO"
    printf "%-20s %s [%s]\n" "--silex" "Enable silex when building Silo" "$DO_SILEX"
}

function bv_silo_host_profile
{
    if [[ "$DO_SILO" == "yes" ]] ; then
        echo >> $HOSTCONF
        echo "##" >> $HOSTCONF
        echo "## Silo" >> $HOSTCONF
        echo "##" >> $HOSTCONF
        echo \
            "VISIT_OPTION_DEFAULT(VISIT_SILO_DIR \${VISITHOME}/silo/$SILO_VERSION/\${VISITARCH})" \
            >> $HOSTCONF

        libdep=""
        if [[ "$DO_HDF5" == "yes" ]] ; then
            libdep="HDF5_LIBRARY_DIR hdf5 \${VISIT_HDF5_LIBDEP}"
        fi
        libdep="$libdep ZLIB_LIBRARY_DIR z"
        if [[ -n "$libdep" ]]; then
            echo \
                "VISIT_OPTION_DEFAULT(VISIT_SILO_LIBDEP $libdep TYPE STRING)" \
                >> $HOSTCONF
        fi
    fi
}

function bv_silo_ensure
{
    if [[ "$DO_SILO" == "yes" ]] ; then
        ensure_built_or_ready "silo" $SILO_VERSION $SILO_BUILD_DIR $SILO_FILE $SILO_URL
        if [[ $? != 0 ]] ; then
            ANY_ERRORS="yes"
            DO_SILO="no"
            error "Unable to build Silo.  ${SILO_FILE} not found."
        fi
    fi
}

function apply_silo_4102_fpzip_patch
{
    info "Patching silo for fpzip DOMAIN and RANGE symbols"
    patch --verbose -p0 <<EOF
Index: src/fpzip/codec.h
===================================================================
--- src/fpzip/codec.h	(revision 809)
+++ src/fpzip/codec.h	(working copy)
@@ -16,13 +16,13 @@
 // identity map for integer arithmetic
 template <typename T, unsigned width>
 struct PCmap<T, width, T> {
-  typedef T DOMAIN;
-  typedef T RANGE;
+  typedef T FPZIP_Domain_t;
+  typedef T FPZIP_Range_t;
   static const unsigned bits = width;
   static const T        mask = ~T(0) >> (bitsizeof(T) - bits);
-  RANGE forward(DOMAIN d) const { return d & mask; }
-  DOMAIN inverse(RANGE r) const { return r & mask; }
-  DOMAIN identity(DOMAIN d) const { return d & mask; }
+  FPZIP_Range_t forward(FPZIP_Domain_t d) const { return d & mask; }
+  FPZIP_Domain_t inverse(FPZIP_Range_t r) const { return r & mask; }
+  FPZIP_Domain_t identity(FPZIP_Domain_t d) const { return d & mask; }
 };
 #endif
 
Index: src/fpzip/pcdecoder.inl
===================================================================
--- src/fpzip/pcdecoder.inl	(revision 809)
+++ src/fpzip/pcdecoder.inl	(working copy)
@@ -19,7 +19,7 @@
 T PCdecoder<T, M, false>::decode(T pred, unsigned context)
 {
   // map type T to unsigned integer type
-  typedef typename M::RANGE U;
+  typedef typename M::FPZIP_Range_t U;
   U p = map.forward(pred);
   // entropy decode d = r - p
   U r = p + rd->decode(rm[context]) - bias;
@@ -46,7 +46,7 @@
 template <typename T, class M>
 T PCdecoder<T, M, true>::decode(T pred, unsigned context)
 {
-  typedef typename M::RANGE U;
+  typedef typename M::FPZIP_Range_t U;
   unsigned s = rd->decode(rm[context]);
   if (s > bias) {      // underprediction
     unsigned k = s - bias - 1;
Index: src/fpzip/pcencoder.inl
===================================================================
--- src/fpzip/pcencoder.inl	(revision 809)
+++ src/fpzip/pcencoder.inl	(working copy)
@@ -18,7 +18,7 @@
 T PCencoder<T, M, false>::encode(T real, T pred, unsigned context)
 {
   // map type T to unsigned integer type
-  typedef typename M::RANGE U;
+  typedef typename M::FPZIP_Range_t U;
   U r = map.forward(real);
   U p = map.forward(pred);
   // entropy encode d = r - p
@@ -47,7 +47,7 @@
 T PCencoder<T, M, true>::encode(T real, T pred, unsigned context)
 {
   // map type T to unsigned integer type
-  typedef typename M::RANGE U;
+  typedef typename M::FPZIP_Range_t U;
   U r = map.forward(real);
   U p = map.forward(pred);
   // compute (-1)^s (2^k + m) = r - p, entropy code (s, k),
Index: src/fpzip/pcmap.h
===================================================================
--- src/fpzip/pcmap.h	(revision 809)
+++ src/fpzip/pcmap.h	(working copy)
@@ -14,53 +14,53 @@
 // specialized for integer-to-integer map
 template <typename T, unsigned width>
 struct PCmap<T, width, void> {
-  typedef T DOMAIN;
-  typedef T RANGE;
-  static const unsigned bits = width;                    // RANGE bits
-  static const unsigned shift = bitsizeof(RANGE) - bits; // DOMAIN\RANGE bits
-  RANGE forward(DOMAIN d) const { return d >> shift; }
-  DOMAIN inverse(RANGE r) const { return r << shift; }
-  DOMAIN identity(DOMAIN d) const { return inverse(forward(d)); }
+  typedef T FPZIP_Domain_t;
+  typedef T FPZIP_Range_t;
+  static const unsigned bits = width;                    // FPZIP_Range_t bits
+  static const unsigned shift = bitsizeof(FPZIP_Range_t) - bits; // FPZIP_Domain_t\FPZIP_Range_t bits
+  FPZIP_Range_t forward(FPZIP_Domain_t d) const { return d >> shift; }
+  FPZIP_Domain_t inverse(FPZIP_Range_t r) const { return r << shift; }
+  FPZIP_Domain_t identity(FPZIP_Domain_t d) const { return inverse(forward(d)); }
 };
 
 // specialized for float type
 template <unsigned width>
 struct PCmap<float, width, void> {
-  typedef float    DOMAIN;
-  typedef unsigned RANGE;
+  typedef float    FPZIP_Domain_t;
+  typedef unsigned FPZIP_Range_t;
   union UNION {
-    UNION(DOMAIN d) : d(d) {}
-    UNION(RANGE r) : r(r) {}
-    DOMAIN d;
-    RANGE r;
+    UNION(FPZIP_Domain_t d) : d(d) {}
+    UNION(FPZIP_Range_t r) : r(r) {}
+    FPZIP_Domain_t d;
+    FPZIP_Range_t r;
   };
-  static const unsigned bits = width;                    // RANGE bits
-  static const unsigned shift = bitsizeof(RANGE) - bits; // DOMAIN\RANGE bits
-  RANGE fcast(DOMAIN d) const;
-  DOMAIN icast(RANGE r) const;
-  RANGE forward(DOMAIN d) const;
-  DOMAIN inverse(RANGE r) const;
-  DOMAIN identity(DOMAIN d) const;
+  static const unsigned bits = width;                    // FPZIP_Range_t bits
+  static const unsigned shift = bitsizeof(FPZIP_Range_t) - bits; // FPZIP_Domain_t\FPZIP_Range_t bits
+  FPZIP_Range_t fcast(FPZIP_Domain_t d) const;
+  FPZIP_Domain_t icast(FPZIP_Range_t r) const;
+  FPZIP_Range_t forward(FPZIP_Domain_t d) const;
+  FPZIP_Domain_t inverse(FPZIP_Range_t r) const;
+  FPZIP_Domain_t identity(FPZIP_Domain_t d) const;
 };
 
 // specialized for double type
 template <unsigned width>
 struct PCmap<double, width, void> {
-  typedef double             DOMAIN;
-  typedef unsigned long long RANGE;
+  typedef double             FPZIP_Domain_t;
+  typedef unsigned long long FPZIP_Range_t;
   union UNION {
-    UNION(DOMAIN d) : d(d) {}
-    UNION(RANGE r) : r(r) {}
-    DOMAIN d;
-    RANGE r;
+    UNION(FPZIP_Domain_t d) : d(d) {}
+    UNION(FPZIP_Range_t r) : r(r) {}
+    FPZIP_Domain_t d;
+    FPZIP_Range_t r;
   };
-  static const unsigned bits = width;                    // RANGE bits
-  static const unsigned shift = bitsizeof(RANGE) - bits; // DOMAIN\RANGE bits
-  RANGE fcast(DOMAIN d) const;
-  DOMAIN icast(RANGE r) const;
-  RANGE forward(DOMAIN d) const;
-  DOMAIN inverse(RANGE r) const;
-  DOMAIN identity(DOMAIN d) const;
+  static const unsigned bits = width;                    // FPZIP_Range_t bits
+  static const unsigned shift = bitsizeof(FPZIP_Range_t) - bits; // FPZIP_Domain_t\FPZIP_Range_t bits
+  FPZIP_Range_t fcast(FPZIP_Domain_t d) const;
+  FPZIP_Domain_t icast(FPZIP_Range_t r) const;
+  FPZIP_Range_t forward(FPZIP_Domain_t d) const;
+  FPZIP_Domain_t inverse(FPZIP_Range_t r) const;
+  FPZIP_Domain_t identity(FPZIP_Domain_t d) const;
 };
 
 #include "pcmap.inl"
Index: src/fpzip/pcmap.inl
===================================================================
--- src/fpzip/pcmap.inl	(revision 809)
+++ src/fpzip/pcmap.inl	(working copy)
@@ -3,12 +3,12 @@
 PCmap<float, width, void>::fcast(float d) const
 {
 #ifdef WITH_REINTERPRET_CAST
-  return reinterpret_cast<const RANGE&>(d);
+  return reinterpret_cast<const FPZIP_Range_t&>(d);
 #elif defined WITH_UNION
   UNION shared(d);
   return shared.r;
 #else
-  RANGE r;
+  FPZIP_Range_t r;
   memcpy(&r, &d, sizeof(r));
   return r;
 #endif
@@ -19,12 +19,12 @@
 PCmap<float, width, void>::icast(unsigned r) const
 {
 #ifdef WITH_REINTERPRET_CAST
-  return reinterpret_cast<const DOMAIN&>(r);
+  return reinterpret_cast<const FPZIP_Domain_t&>(r);
 #elif defined WITH_UNION
   UNION shared(r);
   return shared.d;
 #else
-  DOMAIN d;
+  FPZIP_Domain_t d;
   memcpy(&d, &r, sizeof(d));
   return d;
 #endif
@@ -37,7 +37,7 @@
 unsigned
 PCmap<float, width, void>::forward(float d) const
 {
-  RANGE r = fcast(d);
+  FPZIP_Range_t r = fcast(d);
   r = ~r;
   r >>= shift;
   r ^= -(r >> (bits - 1)) >> (shift + 1);
@@ -61,7 +61,7 @@
 float
 PCmap<float, width, void>::identity(float d) const
 {
-  RANGE r = fcast(d);
+  FPZIP_Range_t r = fcast(d);
   r >>= shift;
   r <<= shift;
   return icast(r);
@@ -72,12 +72,12 @@
 PCmap<double, width, void>::fcast(double d) const
 {
 #ifdef WITH_REINTERPRET_CAST
-  return reinterpret_cast<const RANGE&>(d);
+  return reinterpret_cast<const FPZIP_Range_t&>(d);
 #elif defined WITH_UNION
   UNION shared(d);
   return shared.r;
 #else
-  RANGE r;
+  FPZIP_Range_t r;
   memcpy(&r, &d, sizeof(r));
   return r;
 #endif
@@ -88,12 +88,12 @@
 PCmap<double, width, void>::icast(unsigned long long r) const
 {
 #ifdef WITH_REINTERPRET_CAST
-  return reinterpret_cast<const DOMAIN&>(r);
+  return reinterpret_cast<const FPZIP_Domain_t&>(r);
 #elif defined WITH_UNION
   UNION shared(r);
   return shared.d;
 #else
-  DOMAIN d;
+  FPZIP_Domain_t d;
   memcpy(&d, &r, sizeof(d));
   return d;
 #endif
@@ -106,7 +106,7 @@
 unsigned long long
 PCmap<double, width, void>::forward(double d) const
 {
-  RANGE r = fcast(d);
+  FPZIP_Range_t r = fcast(d);
   r = ~r;
   r >>= shift;
   r ^= -(r >> (bits - 1)) >> (shift + 1);
@@ -130,7 +130,7 @@
 double
 PCmap<double, width, void>::identity(double d) const
 {
-  RANGE r = fcast(d);
+  FPZIP_Range_t r = fcast(d);
   r >>= shift;
   r <<= shift;
   return icast(r);
Index: src/fpzip/read.cpp
===================================================================
--- src/fpzip/read.cpp	(revision 809)
+++ src/fpzip/read.cpp	(working copy)
@@ -103,7 +103,7 @@
 {
   // initialize decompressor
   typedef PCmap<T, bits> TMAP;
-  typedef typename TMAP::RANGE U;
+  typedef typename TMAP::FPZIP_Range_t U;
   typedef PCmap<U, bits, U> UMAP;
   RCmodel* rm = new RCqsmodel(false, PCdecoder<U, UMAP>::symbols);
   PCdecoder<U, UMAP>* fd = new PCdecoder<U, UMAP>(rd, &rm);
Index: src/fpzip/write.cpp
===================================================================
--- src/fpzip/write.cpp	(revision 809)
+++ src/fpzip/write.cpp	(working copy)
@@ -103,7 +103,7 @@
 {
   // initialize compressor
   typedef PCmap<T, bits> TMAP;
-  typedef typename TMAP::RANGE U;
+  typedef typename TMAP::FPZIP_Range_t U;
   typedef PCmap<U, bits, U> UMAP;
   RCmodel* rm = new RCqsmodel(true, PCencoder<U, UMAP>::symbols);
   PCencoder<U, UMAP>* fe = new PCencoder<U, UMAP>(re, &rm);
EOF
    if [[ $? != 0 ]] ; then
        return 1
    fi
}

function apply_silo_patch
{
    info "Patching silo . . ."

    compare_version_strings $SILO_VERSION 4.10.3 -le
    if [[ $? -eq 0 ]]; then
        apply_silo_4102_fpzip_patch
        if [[ $? != 0 ]] ; then
            if [[ $untarred_silo == 1 ]] ; then
                warn "Giving up on Silo build because the patch failed."
                return 1
            else
                warn "Patch failed, but continuing.  I believe that this script\n" \
                     "tried to apply a patch to an existing directory that had\n" \
                     "already been patched ... that is, the patch is\n" \
                     "failing harmlessly on a second application."
            fi
        fi
    fi
    return 0
}

# *************************************************************************** #
#                            Function 8, build_silo
#
# Modfications:
#   Mark C. Miller, Wed Feb 18 22:57:25 PST 2009
#   Added logic to build silex and copy bins on Mac. Removed disablement of
#   browser.
#
#   Mark C. Miller Mon Jan  7 10:31:46 PST 2013
#   PDB/SCORE lite headers are now handled in Silo and require additional
#   configure option to ensure they are installed.
#
#   Brad Whitlock, Tue Apr  9 12:20:22 PDT 2013
#   Add support for custom zlib.
#
#   Kathleen Biagas, Tue Jun 10 08:21:33 MST 2014
#   Disable silex for static builds.
#
# *************************************************************************** #

function build_silo
{
    #
    # Prepare build dir
    #
    prepare_build_dir $SILO_BUILD_DIR $SILO_FILE
    untarred_silo=$?
    if [[ $untarred_silo == -1 ]] ; then
        warn "Unable to prepare Silo build directory. Giving Up!"
        return 1
    fi
    
    #
    # Call configure
    #
    info "Configuring Silo . . ."
    cd $SILO_BUILD_DIR || error "Can't cd to Silo build dir."
    apply_silo_patch || return 1
    info "Invoking command to configure Silo"
    if [[ "$DO_HDF5" == "yes" ]] ; then
        HDF5INCLUDE="$VISITDIR/hdf5/$HDF5_VERSION/$VISITARCH/include"
        HDF5LIB="$VISITDIR/hdf5/$HDF5_VERSION/$VISITARCH/lib"
        WITHHDF5ARG="--with-hdf5=$HDF5INCLUDE,$HDF5LIB"
    else
        WITHHDF5ARG="--without-hdf5"
    fi
    if [[ "$DO_SZIP" == "yes" ]] ; then
        SZIPDIR="$VISITDIR/szip/$SZIP_VERSION/$VISITARCH"
        WITHSZIPARG="--with-szlib=$SZIPDIR"
    else
        WITHSZIPARG="--without-szlib"
    fi
    if [[ "$DO_ZLIB" == "no" ]]; then
        WITH_HZIP_AND_FPZIP="--disable-hzip --disable-fpzip"
    else
        ZLIBARGS="--with-zlib=${VISITDIR}/zlib/${ZLIB_VERSION}/${VISITARCH}/include,${VISITDIR}/zlib/${ZLIB_VERSION}/${VISITARCH}/lib"
    fi
    WITHSHAREDARG="--enable-shared"
    if [[ "$DO_STATIC_BUILD" == "yes" ]] ; then
        WITHSHAREDARG="--disable-shared"
    fi
    WITHSILOQTARG='--disable-silex'

    if [[ "$FC_COMPILER" == "no" ]] ; then
        FORTRANARGS="--disable-fortran"
    else
        FORTRANARGS="FC=\"$FC_COMPILER\" F77=\"$FC_COMPILER\" FCFLAGS=\"$FCFLAGS\" FFLAGS=\"$FCFLAGS\""
    fi

    extra_ac_flags=""
    # detect coral systems, which older versions of autoconf don't detect
    if [[ "$(uname -m)" == "ppc64le" ]] ; then
         extra_ac_flags="ac_cv_build=powerpc64le-unknown-linux-gnu"
    fi 

    set -x
    # In order to ensure $FORTRANARGS is expanded to build the arguments to
    # configure, we wrap the invokation in 'sh -c "..."' syntax
    sh -c "./configure CXX=\"$CXX_COMPILER\" CC=\"$C_COMPILER\" \
        CFLAGS=\"$CFLAGS $C_OPT_FLAGS\" CXXFLAGS=\"$CXXFLAGS $CXX_OPT_FLAGS\" \
        $FORTRANARGS \
        --prefix=\"$VISITDIR/silo/$SILO_VERSION/$VISITARCH\" \
        $WITHHDF5ARG $WITHSZIPARG $WITHSILOQTARG $WITHSHAREDARG $WITH_HZIP_AND_FPZIP\
        --enable-install-lite-headers --without-readline \
        $ZLIBARGS $SILO_EXTRA_OPTIONS ${extra_ac_flags}"
    set +x

    if [[ $? != 0 ]] ; then
        warn "Silo configure failed.  Giving up"
        return 1
    fi

    #
    # Build Silo
    #
    info "Building Silo . . . (~2 minutes)"
    $MAKE $MAKE_OPT_FLAGS
    if [[ $? != 0 ]] ; then
        warn "Silo build failed.  Giving up"
        return 1
    fi
    #
    # Install into the VisIt third party location.
    #
    info "Installing Silo"

    $MAKE install
    if [[ $? != 0 ]] ; then
        warn "Silo install failed.  Giving up"
        return 1
    fi

    if [[ "$DO_GROUP" == "yes" ]] ; then
        chmod -R ug+w,a+rX "$VISITDIR/silo"
        chgrp -R ${GROUP} "$VISITDIR/silo"
    fi
    cd "$START_DIR"
    info "Done with Silo"
    return 0
}

function bv_silo_is_enabled
{
    if [[ $DO_SILO == "yes" ]]; then
        return 1    
    fi
    return 0
}

function bv_silo_is_installed
{
    check_if_installed "silo" $SILO_VERSION
    if [[ $? == 0 ]] ; then
        return 1
    fi
    return 0
}

function bv_silo_build
{
    cd "$START_DIR"
    if [[ "$DO_SILO" == "yes" ]] ; then
        check_if_installed "silo" $SILO_VERSION
        if [[ $? == 0 ]] ; then
            info "Skipping Silo build.  Silo is already installed."
        else
            info "Building Silo (~2 minutes)"
            build_silo
            if [[ $? != 0 ]] ; then
                error "Unable to build or install Silo.  Bailing out."
            fi
            info "Done building Silo"
        fi
    fi
}
