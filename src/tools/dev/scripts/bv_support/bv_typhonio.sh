#initialize all the variables
function bv_typhonio_initialize
{
    export DO_TYPHONIO="no"
    export ON_TYPHONIO="off"
}

#enable the module for install
function bv_typhonio_enable
{ 
    DO_TYPHONIO="yes"
    ON_TYPHONIO="on"
}

#disable the module for install
function bv_typhonio_disable
{
    DO_TYPHONIO="no"
    ON_TYPHONIO="off"
}

#add any dependency with comma separation, both dependers and dependees
function bv_typhonio_depends_on
{
    echo "hdf5"
}

#add information about how to get library..
function bv_typhonio_info
{
    export TYPHONIO_VERSION=${TYPHONIO_VERSION:-"1.6"}
    export TYPHONIO_FILE=${TYPHONIO_FILE:-"typhonio-${TYPHONIO_VERSION}.tar.gz"}
    export TYPHONIO_COMPATIBILITY_VERSION=${TYPHONIO_COMPATIBILITY_VERSION:-"1.6"}
    export TYPHONIO_BUILD_DIR=${TYPHONIO_BUILD_DIR:-"typhonio-${TYPHONIO_VERSION}"}
    export TYPHONIO_URL=${TYPHONIO_URL:-https://github.com/UK-MAC/typhonio/archive}
    export TYPHONIO_MD5_CHECKSUM="204d2820190b079a6bb297af5aa42f8c"
    export TYPHONIO_SHA256_CHECKSUM="59b31cac7e0acc4f3001fff0d5c575f77752ed6b767247211d6ed3584d132ada"
}

#print variables used by this module
function bv_typhonio_print
{
    printf "%s%s\n" "TYPHONIO_FILE=" "${TYPHONIO_FILE}"
    printf "%s%s\n" "TYPHONIO_VERSION=" "${TYPHONIO_VERSION}"
    printf "%s%s\n" "TYPHONIO_COMPATIBILITY_VERSION=" "${TYPHONIO_COMPATIBILITY_VERSION}"
    printf "%s%s\n" "TYPHONIO_BUILD_DIR=" "${TYPHONIO_BUILD_DIR}"
}

#print how to install and uninstall module..
function bv_typhonio_print_usage
{
    printf "%-15s %s [%s]\n" "--typhonio"   "Build TyphonIO" "$DO_TYPHONIO"
}

#how to show information for graphical editor
function bv_typhonio_graphical
{
    local graphical_out="TyphonIO    $TYPHONIO_VERSION($TYPHONIO_FILE)    $ON_TYPHONIO"
    echo $graphical_out
}

#values to add to host profile, write to $HOSTCONF
function bv_typhonio_host_profile
{
    #Add code to write values to variable $HOSTCONF
    if [[ "$DO_TYPHONIO" == "yes" ]] ; then
        echo >> $HOSTCONF
        echo "##" >> $HOSTCONF
        echo "## TyphonIO" >> $HOSTCONF
        echo "##" >> $HOSTCONF
        echo \
        "VISIT_OPTION_DEFAULT(VISIT_TYPHONIO_DIR \${VISITHOME}/typhonio/$TYPHONIO_VERSION/\${VISITARCH})" \
        >> $HOSTCONF
        echo \
        "VISIT_OPTION_DEFAULT(VISIT_TYPHONIO_LIBDEP HDF5_LIBRARY_DIR hdf5 \${VISIT_HDF5_LIBDEP} TYPE STRING)" \
        >> $HOSTCONF
    fi
}

#prepare the module and check whether it is built or is ready to be built.
function bv_typhonio_ensure
{
    if [[ "$DO_TYPHONIO" == "yes" ]] ; then
        ensure_built_or_ready "typhonio" $TYPHONIO_VERSION \
            $TYPHONIO_BUILD_DIR $TYPHONIO_FILE $TYPHONIO_URL
        if [[ $? != 0 ]] ; then
            ANY_ERRORS="yes"
            DO_TYPHONIO="no"
            error "Unable to build TyphonIO.  ${TYPHONIO_FILE} not found."
        fi
    fi
}

#print what the module will do for building
function bv_typhonio_dry_run
{
    #print the build command..
    if [[ "$DO_TYPHONIO" == "yes" ]] ; then
        echo "Dry run option not set for typhonio."
    fi
}

function apply_typhonio_16_patch_1
{
    #
    # Need to change library extension and link against HDF5 on Darwin
    #
    patch -f -p0 <<\EOF
diff -c typhonio-1.6/src/Makefile.orig typhonio-1.6/src/Makefile
*** typhonio-1.6/src/Makefile.orig	2015-03-17 19:49:36.000000000 +0000
--- typhonio-1.6/src/Makefile	2015-03-17 19:50:16.000000000 +0000
***************
*** 24,29 ****
--- 24,30 ----
  
  BUILD			=	${HPC_COMPILER},${HPC_MPI}${EXTENSION}
  
+ EXT_LIB			:=	so
  
  # Version in Major, Minor, Revision format (M.N.R)
  VERSION_MNR		=	` echo $(VERSION) | 		 			\
***************
*** 273,279 ****
  			fi
  			@ if [ ${NOSHARED} == 0 ]; then 						\
  				cd $(SPEC_BUILD_DIR)/src;						\
! 				${MAKE} INCLUDEC=1 SPEC_BUILD_DIR=$(SPEC_BUILD_DIR) LIBNAME=lib$(NAME).so sharedlib includefiles;	\
  			else										\
  				echo "  ---  No C shared library";					\
  			fi
--- 274,280 ----
  			fi
  			@ if [ ${NOSHARED} == 0 ]; then 						\
  				cd $(SPEC_BUILD_DIR)/src;						\
! 				${MAKE} INCLUDEC=1 SPEC_BUILD_DIR=$(SPEC_BUILD_DIR) LIBNAME=lib$(NAME).$(EXT_LIB) sharedlib includefiles;	\
  			else										\
  				echo "  ---  No C shared library";					\
  			fi
***************
*** 285,291 ****
  
  sharedlib	:	$(OBJ)
  			@- rm $(SPEC_BUILD_DIR)/lib/$(LIBNAME)
! 			$(CC) -shared -o $(SPEC_BUILD_DIR)/lib/$(LIBNAME) $?
  			@ echo "  ---  Built $(TITLE) - shared";
  
  includefiles		:	
--- 286,292 ----
  
  sharedlib	:	$(OBJ)
  			@- rm $(SPEC_BUILD_DIR)/lib/$(LIBNAME)
! 			$(CC) -shared -o $(SPEC_BUILD_DIR)/lib/$(LIBNAME) -L $(HDF5_DIR)/lib -lhdf5 $?
  			@ echo "  ---  Built $(TITLE) - shared";
  
  includefiles		:	
EOF

    if [[ $? != 0 ]] ; then
        warn "Unable to apply patch 1 to TyphonIO 1.6"
        return 1
    else
        return 0
    fi
}

function apply_typhonio_16_patch_2
{
    #
    # Add clang to allow build on Darwin
    #
    patch -f -p0 <<\EOF
diff -c typhonio-1.6/src/Makefile.inc.orig typhonio-1.6/src/Makefile.inc
*** typhonio-1.6/src/Makefile.inc.orig	2015-03-20 10:08:25.000000000 +0000
--- typhonio-1.6/src/Makefile.inc	2015-03-20 10:21:46.000000000 +0000
***************
*** 245,250 ****
--- 245,292 ----
  
  endif
  
+ #
+ # Mac OS X clang -------------------------------------------
+ #
+ 
+ ifeq (clang, $(findstring clang,${HPC_COMPILER}))
+ 
+ VALID_COMPILER  =       1
+ 
+ CC_SER          =       clang -fpic -ansi
+ CC_PAR          =       mpicc -fpic -ansi
+ CC_VERSION      =       mpicc --version 2>&1
+ 
+ CFLAGS_OPT      =       -O2
+ CFLAGS_DBG      =       -g -ansi -pedantic -Wall -Wextra -Wno-long-long -Wno-unused-parameter
+ 
+ CINC            =       -I
+ COBJ            =       -c
+ COUT            =       -o
+ 
+ CPP_FLAGS       =
+ 
+ FC_SER          =       gfortran -fpic
+ FC_PAR          =       mpif90 -fpic
+ FC_VERSION      =       mpif90 --version 2>&1
+ 
+ FFLAGS_OPT      =   -O2
+ FFLAGS_DBG      =  -g  -pedantic -Wall -Wextra -Wno-long-long -Wno-unused-parameter
+ 
+ FINC            = -I
+ FOBJ            = -c
+ FOUT            = -o
+ 
+ FMODULES        = *.mod
+ 
+ FPP_FLAGS       = -cpp $(FPP_DEF)_TYPHIO_F2003_STDERR
+ FPP_DEF         = -D
+ FPP_UNDEF       = -U
+ 
+ F2C_DEF         = F2C_UNDERSCORE
+ 
+ 
+ endif
  
  # ----------------------------------------------------------
  
EOF

    if [[ $? != 0 ]] ; then
        warn "Unable to apply patch 2 to TyphonIO 1.6"
        return 1
    else
        return 0
    fi
}

function apply_typhonio_16_patch_3
{
    #
    # Need to add _C99_SOURCE to pick up snprintf etc. on Darwin
    #
    patch -f -p0 <<\EOF
diff -c typhonio-1.6/src/Makefile.inc.orig typhonio-1.6/src/Makefile.inc
typhonio-1.6/src/Makefile.Ccommon.orig typhonio-1.6/src/Makefile.Ccommon
*** typhonio-1.6/src/Makefile.Ccommon.orig	2015-03-23 10:21:33.449059805 +0000
--- typhonio-1.6/src/Makefile.Ccommon	2015-03-23 10:23:31.722059791 +0000
***************
*** 10,16 ****
  
  STANDARD_DEFS	=	-D_TYPHIO_HDF5 			\
  			-D_LARGE_FILES -D_LARGEFILE_SOURCE -D_LARGEFILE64_SOURCE -D_FILE_OFFSET_BITS=64	\
! 			-D_POSIX_SOURCE -D_BSD_SOURCE
  
  CPREPRO_SER	=	$(CPP_FLAGS) $(STANDARD_DEFS) -U_TYPHIO_PARALLEL
  
--- 10,16 ----
  
  STANDARD_DEFS	=	-D_TYPHIO_HDF5 			\
  			-D_LARGE_FILES -D_LARGEFILE_SOURCE -D_LARGEFILE64_SOURCE -D_FILE_OFFSET_BITS=64	\
! 			-D_POSIX_SOURCE -D_BSD_SOURCE -D_C99_SOURCE
  
  CPREPRO_SER	=	$(CPP_FLAGS) $(STANDARD_DEFS) -U_TYPHIO_PARALLEL
  
EOF

    if [[ $? != 0 ]] ; then
        warn "Unable to apply patch 3 to TyphonIO 1.6"
        return 1
    else
        return 0
    fi
}

function apply_typhonio_16_patch_4
{
    #
    # Ensure H5Fopen uses H5F_CLOSE_SEMI like other VisIt plugins
    # - required to allow file to be opened multiple times
    #   (see VisIt src/include/visit-hdf5.h for details)
    #
    patch -f -p0 <<\EOF
diff -c typhonio-1.6/src/tio_hdf5.c.orig typhonio-1.6/src/tio_hdf5.c
*** typhonio-1.6/src/tio_hdf5.c.orig    2017-01-06 13:04:59.603869268 +0000
--- typhonio-1.6/src/tio_hdf5.c 2017-01-06 13:05:41.732867829 +0000
***************
*** 109,114 ****
--- 109,116 ----
  {
    const char *subname = "iH5_ReadVersion()";
  
+   hid_t  access_plist;                      /* HDF5 ID for file access property list */
+ 
    hid_t  file_id, root_id;
    hid_t  dtype_id;
    hid_t  attr_id;
***************
*** 243,249 ****
      irc = iH5_Init();
      TIOassert(irc != 0, ERR_INT, "Failed to initialize HDF5", -8);
  
!     file_id  = H5Fopen(name, H5F_ACC_RDONLY, H5P_DEFAULT);
  
      root_id  = H5Gopen(file_id, "/", H5P_DEFAULT);
  
--- 245,261 ----
      irc = iH5_Init();
      TIOassert(irc != 0, ERR_INT, "Failed to initialize HDF5", -8);
  
!     /* -- Ensure H5F_CLOSE_SEMI to match other VisIt plugins */
!     /*    H5FOpen will fail if file already open otherwise */
!     access_plist = H5Pcreate(H5P_FILE_ACCESS);
!     TIOassert(access_plist < 0, ERR_HDF5, "Failed to create access_plist", -2);
!     H5Pset_fclose_degree(access_plist, H5F_CLOSE_SEMI);
! 
!     file_id  = H5Fopen(name, H5F_ACC_RDONLY, access_plist);
! 
!     /* -- Close file access property list */
!     hrc = H5Pclose(access_plist);
!     TIOassert(hrc < 0, ERR_HDF5, "Failed to close access_plist", -3);
  
      root_id  = H5Gopen(file_id, "/", H5P_DEFAULT);
  
***************
*** 1247,1252 ****
--- 1259,1267 ----
      break;
    }
  
+   /* -- Ensure H5F_CLOSE_SEMI to match other VisIt plugins */
+   /*    cannot open multiple times otherwise */
+   H5Pset_fclose_degree(access_plist, H5F_CLOSE_SEMI);
  
    /* -- Open the file - don't need any file open property list, just access, unlike create */
    hfile->file_id = H5Fopen(name, accessH5, access_plist);
EOF

    if [[ $? != 0 ]] ; then
        warn "Unable to apply patch 4 to TyphonIO 1.6"
        return 1
    else
        return 0
    fi
}

function apply_typhonio_patch
{
    info "Patching TyphonIO . . ."
    if [[ ${TYPHONIO_VERSION} == "1.6" ]] ; then
        apply_typhonio_16_patch_1
        if [[ $? != 0 ]] ; then
            return 1
        fi
        apply_typhonio_16_patch_2
        if [[ $? != 0 ]] ; then
            return 1
        fi
        apply_typhonio_16_patch_3
        if [[ $? != 0 ]] ; then
            return 1
        fi
        apply_typhonio_16_patch_4
        if [[ $? != 0 ]] ; then
            return 1
        fi
    else
        warn "Unsupported TyphonIO Version ${TYPHONIO_VERSION}"
        return 1
    fi

    return 0
}

# ***************************************************************************
# Function: build_typhonio
#
# Purpose: Build TyphonIO library
#
# Programmer: Paul Selby
# Date: February 6, 2015
#
# Modifications:
#   Paul Selby, Mon 23 Mar 10:46:17 GMT 2015
#   Patches Makefiles and passes additional information through to make
#     => can now build on Darwin or with non-gcc compiler
#   Build directory now includes $VISITARCH so can host multiple builds
#   Patches install path on Darwin for dynamic library
#   Removes install dir on failure so build_visit knows to try again
#
#   Paul Selby, Mon 23 Mar 16:03:00 GMT 2015
#   Added support for static build. Now builds using TyphonIO install target
#
#   Paul Selby, Wed 25 Mar 2015 08:55:55 GMT
#   Fixed Darwin build by adding LIB_EXT back to make
#
#   Paul Selby, Fri  6 Jan 13:12:14 GMT 2017
#   Added patch to ensure H5F_CLOSE_SEMI used for H5Fopen calls to match other
#   HDF5 plugins (see VisIt src/include/visit-hdf5.h)
#
# ***************************************************************************
function build_typhonio
{
    #
    # Prepare build dir
    #
    prepare_build_dir $TYPHONIO_BUILD_DIR $TYPHONIO_FILE SHA256 \
        $TYPHONIO_SHA256_CHECKSUM
    untarred_typhonio=$?
    # 0, already exists, 1 untarred src, 2 error

    if [[ $untarred_typhonio == -1 ]] ; then
       warn "Unable to prepare TyphonIO build directory. Giving Up!"
       return 1
    fi

    #
    # Patch TyphonIO
    #
    apply_typhonio_patch
    if [[ $? != 0 ]] ; then
        if [[ $untarred_typhonio == 1 ]] ; then
            warn "Giving up on TyphonIO build because the patch failed."
            return 1
         else
            warn "Patch failed, but continuing.  I believe that this script\n" \
                 "tried to apply a patch to an existing directory that had\n" \
                 "already been patched ... that is, that the patch is\n" \
                 "failing harmlessly on a second application."
        fi
    fi

    #
    # Choose static or dynamic build
    #
    local NOSTATIC=1
    local NOSHARED=0
    local BUILD_SUBDIR=$VISITARCH-visit
    if [[ "$DO_STATIC_BUILD" == "yes" ]] ; then
        NOSTATIC=0
        NOSHARED=1
        BUILD_SUBDIR=$VISITARCH-static-visit
    fi
    
    cd $TYPHONIO_BUILD_DIR/src || error "Can't cd to TyphonIO build dir."
    
    #
    # Build TyphonIO and install into the VisIt third party location
    # - fails with make -j so leave out $MAKE_OPT_FLAGS
    # Based on VISIT_STUFF in Makefile but BUILD instead of SPEC_BUILD_DIR
    # Not sure if NOH5CLOSE needed for static build
    #
    info "Making TyphonIO . . ."
    $MAKE EXT_LIB=$SO_EXT BUILD=$BUILD_SUBDIR \
          CPREPRO_EXTRA=-D_TYPHIO_NOH5CLOSE \
          HDF5_DIR=$VISITDIR/hdf5/$HDF5_VERSION/$VISITARCH \
          HDF5_VERSION=$HDF5_VERSION \
          NOSTATIC=$NOSTATIC NOF90=1 NOSHARED=$NOSHARED \
          HPC_COMPILER=$C_COMPILER HPC_MPI=serial install
    if [[ $? != 0 ]] ; then
        warn "TyphonIO build failed. Giving up"
        return 1
    fi
    
    #
    # Install into the VisIt third party location
    #
    info "Installing TyphonIO . . ."
    mkdir -p $VISITDIR/typhonio/$TYPHONIO_VERSION/$VISITARCH
    cp -R ../build/$BUILD_SUBDIR/include \
          ../build/$BUILD_SUBDIR/lib \
          $VISITDIR/typhonio/$TYPHONIO_VERSION/$VISITARCH
    if [[ $? != 0 ]] ; then
        #
        # Remove install dir otherwise build_visit thinks library exists
        #
        rm -Rf $VISITDIR/typhonio/$TYPHONIO_VERSION/$VISITARCH
        warn "TyphonIO install failed.  Giving up"
        return 1
    fi

    if [[ "$DO_STATIC_BUILD" == "no" && "$OPSYS" == "Darwin" ]]; then
        #
        # Make dynamic executable, need to patch up the install path and
        # version information.
        #
        info "Creating dynamic libraries for TyphonIO . . ."
        install_name_tool -id $VISITDIR/typhonio/$TYPHONIO_VERSION/$VISITARCH/lib/libtyphonio.$SO_EXT $VISITDIR/typhonio/$TYPHONIO_VERSION/$VISITARCH/lib/libtyphonio.$SO_EXT
    fi

    if [[ "$DO_GROUP" == "yes" ]] ; then
        chmod -R ug+w,a+rX "$VISITDIR/typhonio"
        chgrp ${GROUP} "$VISITDIR/typhonio"
    fi

    cd "$START_DIR"
    info "Done with TyphonIO"
    return 0
}

function bv_typhonio_is_enabled
{
    if [[ $DO_TYPHONIO == "yes" ]]; then
        return 1
    fi
    return 0
}

function bv_typhonio_is_installed
{
    check_if_installed "typhonio" $TYPHONIO_VERSION
    if [[ $? == 0 ]] ; then
        return 1
    fi
    return 0
}

#the build command..
function bv_typhonio_build
{

    if [[ "$DO_TYPHONIO" == "yes" ]] ; then
        check_if_installed "typhonio" $TYPHONIO_VERSION
        if [[ $? == 0 ]] ; then
            info "Skipping TyphonIO build.  TyphonIO is already installed."
        else
            info "Building TyphonIO (~1 minute)"

            #Build the Module 
            #(Please enter custom configure and install instructions here)
            build_typhonio
            if [[ $? != 0 ]] ; then
                error "Unable to build or install TyphonIO.  Bailing out."
            fi
            info "Done building TyphonIO"
       fi
    fi
}
