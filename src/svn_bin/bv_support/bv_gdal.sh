function bv_gdal_initialize
{
export DO_GDAL="no"
export ON_GDAL="off"
}

function bv_gdal_enable
{
DO_GDAL="yes"
ON_GDAL="on"
}

function bv_gdal_disable
{
DO_GDAL="no"
ON_GDAL="off"
}

function bv_gdal_depends_on
{
echo ""
}

function bv_gdal_info
{
export GDAL_FILE=${GDAL_FILE:-"gdal-1.7.1.tar.gz"}
export GDAL_VERSION=${GDAL_VERSION:-"1.7.1"}
export GDAL_COMPATIBILITY_VERSION=${GDAL_COMPATIBILITY_VERSION:-"1.7"}
export GDAL_BUILD_DIR=${GDAL_BUILD_DIR:-"gdal-1.7.1"}
export GDAL_MD5_CHECKSUM="5a90be385613730cc12fcf3f59b89de6"
export GDAL_SHA256_CHECKSUM=""
}

function bv_gdal_print
{
  printf "%s%s\n" "GDAL_FILE=" "${GDAL_FILE}"
  printf "%s%s\n" "GDAL_VERSION=" "${GDAL_VERSION}"
  printf "%s%s\n" "GDAL_COMPATIBILITY_VERSION=" "${GDAL_COMPATIBILITY_VERSION}"
  printf "%s%s\n" "GDAL_BUILD_DIR=" "${GDAL_BUILD_DIR}"
}

function bv_gdal_print_usage
{
 printf "%-15s %s [%s]\n" "--gdal" "Build GDAL" "$DO_GDAL"
}

function bv_gdal_graphical
{
local graphical_out="GDAL     $GDAL_VERSION($GDAL_FILE)      $ON_GDAL"
echo $graphical_out
}

function bv_gdal_host_profile
{
    if [[ "$DO_GDAL" == "yes" ]] ; then
        echo >> $HOSTCONF
        echo "##" >> $HOSTCONF
        echo "## GDAL" >> $HOSTCONF
        echo "##" >> $HOSTCONF
        echo \
        "VISIT_OPTION_DEFAULT(VISIT_GDAL_DIR \${VISITHOME}/gdal/$GDAL_VERSION/\${VISITARCH})" \
        >> $HOSTCONF
    fi

}

function bv_gdal_ensure
{
    if [[ "$DO_GDAL" == "yes" ]] ; then
        ensure_built_or_ready "gdal" $GDAL_VERSION $GDAL_BUILD_DIR $GDAL_FILE http://dl.maptools.org/dl/gdal
        if [[ $? != 0 ]] ; then
            ANY_ERRORS="yes"
            DO_GDAL="no"
            error "Unable to build GDAL.  ${GDAL_FILE} not found."
        fi
    fi
}

function bv_gdal_dry_run
{
  if [[ "$DO_GDAL" == "yes" ]] ; then
    echo "Dry run option not set for gdal."
  fi
}

# *************************************************************************** #
#                         Function 8.6, build_gdal                            #
# *************************************************************************** #

function apply_gdal_linux_x86_64_patch
{
    mv configure configure.old
    sed "s/expat_prefix\/lib -lexpat/expat_prefix\/lib64 -lexpat/g" configure.old > configure
    chmod 700 configure
}

function apply_gdal_mac6_patch
{
    cat frmts/gtiff/libtiff/GNUmakefile | \
       sed 's/tif_zip.o/tif_zip.o lfind.o/' > tmp.make
    mv frmts/gtiff/libtiff/GNUmakefile \
       frmts/gtiff/libtiff/GNUmakefile.orig
    mv tmp.make frmts/gtiff/libtiff/GNUmakefile

    echo > frmts/gtiff/libtiff/lfind.c
    cat >> frmts/gtiff/libtiff/lfind.c << EOF
#include <sys/types.h>
#include <string.h>
#include <unistd.h>

static char *linear_base();

char *
lfind(key, base, nelp, width, compar)
      char *key, *base;
      u_int *nelp, width;
      int (*compar)();
{
      return(linear_base(key, base, nelp, width, compar, 0));
}

static char *
linear_base(key, base, nelp, width, compar, add_flag)
      char *key, *base;
      u_int *nelp, width;
      int (*compar)(), add_flag;
{
      register char *element, *end;

      end = base + *nelp * width;
      for (element = base; element < end; element += width)
              if (!compar(element, key))              /* key found */
                      return(element);

      if (!add_flag)                                  /* key not found */
              return(NULL);

      ++*nelp;
      bcopy(key, end, (int)width);
      return(end);
}
EOF
}

function build_gdal
{
    #
    # Prepare build dir
    #
    prepare_build_dir $GDAL_BUILD_DIR $GDAL_FILE
    untarred_gdal=$?
    if [[ $untarred_gdal == -1 ]] ; then
       warn "Unable to prepare GDAL Build Directory. Giving Up"
       return 1
    fi

    #
    info "Configuring GDAL . . ."
    cd $GDAL_BUILD_DIR || error "Can't cd to GDAL build dir."
    info "Invoking command to configure GDAL"
    if [[ "$OPSYS" == "Darwin" ]]; then
       if [[ "$DO_STATIC_BUILD" == "no" ]]; then
           EXTRA_FLAGS="F77=\"\" --enable-shared --disable-static --without-libtool --without-expat"
       else
           EXTRA_FLAGS="F77=\"\" --enable-static --without-ld-shared  --without-libtool --without-expat"
       fi
    else
       EXTRA_FLAGS="--enable-static --disable-shared --with-hide-internal-symbols"
    fi

    if [[ "$OPSYS" == "Darwin" ]]; then
        # Check for version 6.x.x (MacOS 10.2, Jaguar)
        VER=$(uname -r)
        if (( ${VER%%.*} < 7 )) ; then
             apply_gdal_mac6_patch
        fi
    fi
    if [[ "$OPSYS" == "Linux" ]] ; then
        if [[ "$(uname -m)" == "x86_64" ]] ; then
             apply_gdal_linux_x86_64_patch
        fi
    fi

    ./configure CXX="$CXX_COMPILER" CC="$C_COMPILER" $EXTRA_FLAGS \
       CFLAGS="$CFLAGS $C_OPT_FLAGS -DH5_USE_16_API" \
       CXXFLAGS="$CXXFLAGS $CXX_OPT_FLAGS -DH5_USE_16_API" \
       --prefix="$VISITDIR/gdal/$GDAL_VERSION/$VISITARCH" \
       --with-libtiff=internal --with-gif=internal \
       --with-png=internal --with-jpeg=internal \
       --with-libz=internal --with-netcdf=no \
       --with-pg=no --with-curl=no \
       --without-jasper --without-python \
       --without-sqlite3
    if [[ $? != 0 ]] ; then
       warn "GDAL configure failed.  Giving up"
       return 1
    fi

    #
    # Build GDAL
    #
    info "Building GDAL . . . (~7 minutes)"

    $MAKE $MAKE_OPT_FLAGS
    if [[ $? != 0 ]] ; then
       warn "GDAL build failed.  Giving up"
       return 1
    fi
    #
    # Install into the VisIt third party location.
    #
    info "Installing GDAL . . ."

    $MAKE install
    if [[ $? != 0 ]] ; then
       warn "GDAL install failed.  Giving up"
       return 1
    fi

    if [[ "$DO_STATIC_BUILD" == "no" && "$OPSYS" == "Darwin" ]]; then
        #
        # Make dynamic executable
        #
        info "Fixing install_name of dynamic libraries for GDAL . . ."

        cp .libs/libgdal.1.10.0.${SO_EXT} libgdal.${SO_EXT}
        INSTALLNAMEPATH="$VISITDIR/gdal/${GDAL_VERSION}/$VISITARCH/lib"

        install_name_tool -id \
           $INSTALLNAMEPATH/libgdal.${SO_EXT} \
           libgdal.${SO_EXT}
        rm "$VISITDIR/gdal/$GDAL_VERSION/$VISITARCH/lib/libgdal.${SO_EXT}"
        cp libgdal.${SO_EXT} \
        "$VISITDIR/gdal/$GDAL_VERSION/$VISITARCH/lib/libgdal.${SO_EXT}"
    fi

    if [[ "$DO_GROUP" == "yes" ]] ; then
       chmod -R ug+w,a+rX "$VISITDIR/gdal"
       chgrp -R ${GROUP} "$VISITDIR/gdal"
    fi
    cd "$START_DIR"
    info "Done with GDAL"
    return 0
}

function bv_gdal_is_enabled
{
    if [[ $DO_GDAL == "yes" ]]; then
        return 1    
    fi
    return 0
}

function bv_gdal_is_installed
{
    check_if_installed "gdal" $GDAL_VERSION
    if [[ $? == 0 ]] ; then
        return 1
    fi
    return 0
}

function bv_gdal_build
{
cd "$START_DIR"
if [[ "$DO_GDAL" == "yes" ]] ; then
    if [[ "$OPSYS" == "AIX" ]]; then
        info "Skipping GDAL build.  AIX build is not supported."
        DO_GDAL="no"
    else
        check_if_installed "gdal" $GDAL_VERSION
        if [[ $? == 0 ]] ; then
            info "Skipping GDAL build.  GDAL is already installed."
        else
            info "Building GDAL (~2 minutes)"
            build_gdal
            if [[ $? != 0 ]] ; then
                error "Unable to build or install GDAL.  Bailing out."
            fi
            info "Done building GDAL"
        fi
    fi
fi
}
