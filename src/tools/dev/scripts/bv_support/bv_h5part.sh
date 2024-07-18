function bv_h5part_initialize
{
    export DO_H5PART="no"
}

function bv_h5part_enable
{
    DO_H5PART="yes"
    DO_HDF5="yes"
    DO_SZIP="yes"
}

function bv_h5part_disable
{
    DO_H5PART="no"
}

function bv_h5part_depends_on
{
    echo "szip hdf5"
}

function bv_h5part_info
{
    export H5PART_VERSION=${H5PART_VERSION:-"1.6.6"}
    export H5PART_FILE=${H5PART_FILE:-"H5Part-${H5PART_VERSION}.tar.gz"}
    export H5PART_COMPATIBILITY_VERSION=${H5PART_COMPATIBILITY_VERSION:-"1.6"}
    export H5PART_URL=${H5PART_URL:-"https://codeforge.lbl.gov/frs/download.php/387"}
    export H5PART_BUILD_DIR=${H5PART_BUILD_DIR:-"H5Part-${H5PART_VERSION}"}
    export H5PART_SHA256_CHECKSUM="10347e7535d1afbb08d51be5feb0ae008f73caf889df08e3f7dde717a99c7571"
}

function bv_h5part_print
{
    printf "%s%s\n" "H5PART_FILE=" "${H5PART_FILE}"
    printf "%s%s\n" "H5PART_VERSION=" "${H5PART_VERSION}"
    printf "%s%s\n" "H5PART_COMPATIBILITY_VERSION=" "${H5PART_COMPATIBILITY_VERSION}"
    printf "%s%s\n" "H5PART_BUILD_DIR=" "${H5PART_BUILD_DIR}"
}

function bv_h5part_print_usage
{
    printf "%-20s %s [%s]\n" "--h5part" "Build H5Part" "$DO_H5PART"
}

function bv_h5part_host_profile
{
    if [[ "$DO_H5PART" == "yes" ]] ; then
        echo >> $HOSTCONF
        echo "##" >> $HOSTCONF
        echo "## H5Part" >> $HOSTCONF
        echo "##" >> $HOSTCONF
        echo "SETUP_APP_VERSION(H5PART $H5PART_VERSION)" >> $HOSTCONF
        echo \
            "VISIT_OPTION_DEFAULT(VISIT_H5PART_DIR \${VISITHOME}/h5part/\${H5PART_VERSION}/\${VISITARCH})" \
            >> $HOSTCONF
        echo \
            "VISIT_OPTION_DEFAULT(VISIT_H5PART_LIBDEP HDF5_LIBRARY_DIR hdf5 \${VISIT_HDF5_LIBDEP} TYPE STRING)" \
            >> $HOSTCONF

    fi

}

function bv_h5part_ensure
{
    if [[ "$DO_H5PART" == "yes" ]] ; then
        ensure_built_or_ready "h5part" $H5PART_VERSION $H5PART_BUILD_DIR $H5PART_FILE $H5PART_URL
        if [[ $? != 0 ]] ; then
            ANY_ERRORS="yes"
            DO_H5PART="no"
            error "Unable to build H5Part.  ${H5PART_FILE} not found."
        fi
    fi
}

function apply_h5part_1_6_6_patch
{
    info "Patching H5Part"
    patch -p0 << \EOF
diff -rcN H5Part-1.6.6/src/H5PartTypes-orig.h  H5Part-1.6.6/src/H5PartTypes.h 
*** H5Part-1.6.6/src/H5PartTypes-orig.h	2016-12-14 14:04:41.000000000 -0700
--- H5Part-1.6.6/src/H5PartTypes.h	2016-12-14 14:00:57.000000000 -0700
***************
*** 19,28 ****
  #endif
   ;
  
- #ifndef PARALLEL_IO
- typedef unsigned long		MPI_Comm;
- #endif
- 
  #define H5PART_STEPNAME_LEN	64
  #define H5PART_DATANAME_LEN	64
  
--- 19,24 ----
***************
*** 86,93 ****
  	/**
  	   MPI communicator
  	*/
  	MPI_Comm comm;
! 
  	int throttle;
  
  	struct H5BlockStruct *block;
--- 82,90 ----
  	/**
  	   MPI communicator
  	*/
+ #ifdef PARALLEL_IO
  	MPI_Comm comm;
! #endif
  	int throttle;
  
  	struct H5BlockStruct *block;

diff -rcN H5Part-1.6.6/src/H5Part-orig.h  H5Part-1.6.6/src/H5Part.h
*** H5Part-1.6.6/src/H5Part-orig.h	2016-12-14 14:04:41.000000000 -0700
--- H5Part-1.6.6/src/H5Part.h	        2016-12-14 14:00:57.000000000 -0700
***************
*** 160,165 ****
--- 160,193 ----
  	const h5part_int32_t *array
  	);
  
+ h5part_int64_t
+ H5PartAppendDataFloat64 (
+ 	H5PartFile *f,
+ 	const char *name,
+ 	const h5part_float64_t *array
+ 	);
+ 
+ h5part_int64_t
+ H5PartAppendDataFloat32 (
+ 	H5PartFile *f,
+ 	const char *name,
+ 	const h5part_float32_t *array
+ 	);
+ 
+ h5part_int64_t
+ H5PartAppendDataInt64 (
+ 	H5PartFile *f,
+ 	const char *name,
+ 	const h5part_int64_t *array
+ 	);
+ 
+ h5part_int64_t
+ H5PartAppendDataInt32 (
+ 	H5PartFile *f,
+ 	const char *name,
+ 	const h5part_int32_t *array
+ 	);
+ 
  /*================== File Reading Routines =================*/
  h5part_int64_t
  H5PartSetStep (

diff -rcN H5Part-1.6.6/src/H5Part-orig.c  H5Part-1.6.6/src/H5Part.c
*** H5Part-1.6.6/src/H5Part-orig.c	2016-12-14 14:04:41.000000000 -0700
--- H5Part-1.6.6/src/H5Part.c	        2016-12-14 14:00:57.000000000 -0700
***************
*** 140,146 ****
--- 140,148 ----
  _H5Part_open_file (
  	const char *filename,	/*!< [in] The name of the data file to open. */
  	const char flags,	/*!< [in] The access mode for the file. */
+ #ifdef PARALLEL_IO
  	MPI_Comm comm,		/*!< [in] MPI communicator */
+ #endif	
  	int f_parallel,		/*!< [in] 0 for serial io otherwise parallel */
  	h5part_int64_t align	/*!< [in] Number of bytes for setting alignment,
  					  metadata block size, etc.
***************
*** 166,171 ****
--- 168,175 ----
  	f->xfer_prop = f->dcreate_prop = f->fcreate_prop = H5P_DEFAULT;
  
  	f->access_prop = H5Pcreate (H5P_FILE_ACCESS);
+         H5Pset_fclose_degree(f->access_prop, H5F_CLOSE_SEMI);
+ 	
  	if (f->access_prop < 0) {
  		HANDLE_H5P_CREATE_ERR;
  		goto error_cleanup;
***************
*** 282,288 ****
--- 286,294 ----
  #endif // PARALLEL_IO
  	} else {
  		_is_root_proc = 1;
+ #ifdef PARALLEL_IO
  		f->comm = 0;
+ #endif		
  		f->nprocs = 1;
  		f->myproc = 0;
  		f->pnparticles = 
***************
*** 481,491 ****
  	INIT
  	SET_FNAME ( "H5PartOpenFile" );
  
! 	MPI_Comm comm = 0;	/* dummy */
  	int f_parallel = 0;	/* serial open */
  	int align = 0;		/* no tuning parameters */
  
! 	return _H5Part_open_file ( filename, flags, comm, f_parallel, align );
  }
  
  /*!
--- 487,497 ----
  	INIT
  	SET_FNAME ( "H5PartOpenFile" );
  
! 	/* MPI_Comm comm = 0;	/\* dummy *\/ */
  	int f_parallel = 0;	/* serial open */
  	int align = 0;		/* no tuning parameters */
  
! 	return _H5Part_open_file ( filename, flags, /*comm,*/ f_parallel, align );
  }
  
  /*!
***************
*** 519,528 ****
  	INIT
  	SET_FNAME ( "H5PartOpenFileAlign" );
  
! 	MPI_Comm comm = 0;	/* dummy */
  	int f_parallel = 0;	/* serial open */
  
! 	return _H5Part_open_file ( filename, flags, comm, f_parallel, align );
  }
  
  /*!
--- 525,534 ----
  	INIT
  	SET_FNAME ( "H5PartOpenFileAlign" );
  
! 	/* MPI_Comm comm = 0;	/\* dummy *\/ */
  	int f_parallel = 0;	/* serial open */
  
! 	return _H5Part_open_file ( filename, flags, /*comm,*/ f_parallel, align );
  }
  
  /*!
***************
*** 1277,1282 ****
--- 1283,1487 ----
  	return H5PART_SUCCESS;
  }
  
+ static h5part_int64_t
+ _append_data (
+         H5PartFile *f,          /*!< IN: Handle to open file */
+         const char *name,       /*!< IN: Name to associate array with */
+         const void *array,      /*!< IN: Array to commit to disk */
+         const hid_t type        /*!< IN: Type of data */
+         ) {
+ 
+         herr_t herr;
+         hid_t dataset_id;
+ 
+         char name2[H5PART_DATANAME_LEN];
+         _normalize_dataset_name ( name, name2 );
+ 
+         _H5Part_print_debug (
+                      "Create a dataset[%s] mounted on "
+                      "timestep %lld",
+                      name2, (long long)f->timestep );
+ 
+         if ( f->shape == H5S_ALL ) {
+                 _H5Part_print_warn (
+                      "The view is unset or invalid: please "
+                      "set the view or specify a number of particles." );
+                 return HANDLE_H5PART_BAD_VIEW_ERR ( f->viewstart, f->viewend );
+           return -1;
+         }
+ 
+         H5E_BEGIN_TRY
+         dataset_id = H5Dopen ( f->timegroup, name2
+ #ifndef H5_USE_16_API
+                 , H5P_DEFAULT
+ #endif
+                 );
+         H5E_END_TRY
+ 
+         hid_t dataspace_id, filespace_id;
+ 
+         if ( dataset_id > 0 ) {
+ 
+           hsize_t dims[1] = {f->nparticles}; // dataset dimensions
+           hsize_t dims_in[1] = {0};          // incoming dimensions
+           hsize_t dims_out[1] = {0};         // outgoing dimensions
+ 
+           // Get the original dataspace.
+           dataspace_id = H5Dget_space(dataset_id);
+           if ( dataspace_id < 0 )
+             return HANDLE_H5D_CREATE_ERR ( name2, f->timestep );
+           herr = H5Sget_simple_extent_dims(dataspace_id, dims_in, NULL);
+ 
+           // Extend the dataset
+           dims_out[0] = dims_in[0] + dims[0];
+           herr = H5Dset_extent(dataset_id, dims_out);
+ 
+           // Create the hyperslab
+           filespace_id = H5Dget_space(dataset_id);
+           if ( filespace_id < 0 )
+             return HANDLE_H5D_CREATE_ERR ( name2, f->timestep );
+           herr = H5Sselect_hyperslab(filespace_id, H5S_SELECT_SET,
+                                      dims_in, NULL, dims, NULL);
+ 
+           // Define the memory space
+           dataspace_id = H5Screate_simple(1, dims, NULL);
+           if ( dataspace_id < 0 )
+             return HANDLE_H5D_CREATE_ERR ( name2, f->timestep );
+           
+           if ( herr < 0 )
+             return HANDLE_H5D_CREATE_ERR ( name2, f->timestep );
+         } else {          
+                 dataset_id = H5Dcreate (
+                         f->timegroup,
+                         name2,
+                         type,
+ 			// Use the memshape because it has unlimited bounds
+                         f->memshape, //f->shape,
+ #ifndef H5_USE_16_API
+                         H5P_DEFAULT,
+                         f->dcreate_prop,
+                         H5P_DEFAULT
+ #else
+                         f->dcreate_prop
+ #endif
+                );
+                 if ( dataset_id < 0 )
+                         return HANDLE_H5D_CREATE_ERR ( name2, f->timestep );
+ 
+                 // Create the hyperslab
+                 dataspace_id = f->memshape;
+                 filespace_id = f->diskshape;
+         }
+ 
+ #ifdef PARALLEL_IO
+         herr = _H5Part_start_throttle ( f );
+         if ( herr < 0 ) return herr;
+ #endif
+ 
+         herr = H5Dwrite(dataset_id,
+                         type,
+                         dataspace_id,   // f->memshape,
+                         filespace_id,   // f->diskshape,
+                         f->xfer_prop,
+                         array);
+ 
+ #ifdef PARALLEL_IO
+         herr = _H5Part_end_throttle ( f );
+         if ( herr < 0 ) return herr;
+ #endif
+ 
+         if ( herr < 0 ) return HANDLE_H5D_WRITE_ERR ( name2, f->timestep );
+ 
+         herr = H5Dclose ( dataset_id );
+         if ( herr < 0 ) return HANDLE_H5D_CLOSE_ERR;
+ 
+         f->empty = 0;
+ 
+         return H5PART_SUCCESS;
+ }
+ 
+ h5part_int64_t
+ H5PartAppendDataFloat32 (
+         H5PartFile *f,          /*!< [in] Handle to open file */
+         const char *name,       /*!< [in] Name to associate array with */
+         const h5part_float32_t *array   /*!< [in] Array to commit to disk */
+         ) {
+ 
+         SET_FNAME ( "H5PartWriteDataFloat64" );
+         h5part_int64_t herr;
+ 
+         CHECK_FILEHANDLE ( f );
+         CHECK_WRITABLE_MODE( f );
+         CHECK_TIMEGROUP( f );
+ 
+         herr = _append_data ( f, name, (void*)array, H5T_NATIVE_FLOAT );
+         if ( herr < 0 ) return herr;
+ 
+         return H5PART_SUCCESS;
+ }
+ 
+ h5part_int64_t
+ H5PartAppendDataFloat64 (
+         H5PartFile *f,          /*!< [in] Handle to open file */
+         const char *name,       /*!< [in] Name to associate array with */
+         const h5part_float64_t *array   /*!< [in] Array to commit to disk */
+         ) {
+ 
+         SET_FNAME ( "H5PartWriteDataFloat64" );
+         h5part_int64_t herr;
+ 
+         CHECK_FILEHANDLE ( f );
+         CHECK_WRITABLE_MODE( f );
+         CHECK_TIMEGROUP( f );
+ 
+         herr = _append_data ( f, name, (void*)array, H5T_NATIVE_DOUBLE );
+         if ( herr < 0 ) return herr;
+ 
+         return H5PART_SUCCESS;
+ }
+ 
+ 
+ h5part_int64_t
+ H5PartAppendDataInt32 (
+         H5PartFile *f,          /*!< [in] Handle to open file */
+         const char *name,       /*!< [in] Name to associate array with */
+         const h5part_int32_t *array   /*!< [in] Array to commit to disk */
+         ) {
+ 
+         SET_FNAME ( "H5PartWriteDataInt64" );
+         h5part_int64_t herr;
+ 
+         CHECK_FILEHANDLE ( f );
+         CHECK_WRITABLE_MODE( f );
+         CHECK_TIMEGROUP( f );
+ 
+         herr = _append_data ( f, name, (void*)array, H5T_NATIVE_INT32 );
+         if ( herr < 0 ) return herr;
+ 
+         return H5PART_SUCCESS;
+ }
+ 
+ h5part_int64_t
+ H5PartAppendDataInt64 (
+         H5PartFile *f,          /*!< [in] Handle to open file */
+         const char *name,       /*!< [in] Name to associate array with */
+         const h5part_int64_t *array   /*!< [in] Array to commit to disk */
+         ) {
+ 
+         SET_FNAME ( "H5PartWriteDataInt64" );
+         h5part_int64_t herr;
+ 
+         CHECK_FILEHANDLE ( f );
+         CHECK_WRITABLE_MODE( f );
+         CHECK_TIMEGROUP( f );
+ 
+         herr = _append_data ( f, name, (void*)array, H5T_NATIVE_INT64 );
+         if ( herr < 0 ) return herr;
+ 
+         return H5PART_SUCCESS;
+ }
+ 
+ 
  /********************** reading and writing attribute ************************/
  
  /********************** private functions to handle attributes ***************/

EOF

}

function apply_h5part_patch
{
    if [[ ${H5PART_VERSION} == 1.6.6 ]] ; then
        apply_h5part_1_6_6_patch
        if [[ $? != 0 ]] ; then
            return 1
        fi
    fi

    return 0
}

# ***************************************************************************
#                         Function 8.10, build_h5part
#
# Modifications:
#
#  Mark C. Miller, Tue Oct 28 11:10:36 PDT 2008
#  Added -DH5_USE_16_API to CFLAGS for configuring H5Part. This should be
#  harmless when building H5Part against versions of HDF5 before 1.8 and
#  necessary when building against versions of HDF5 1.8 or later. It tells
#  HDF5 which version of the HDF5 API H5Part was implemented with.
#
#  Gunther H. Weber, Wed Jul 27 14:48:12 PDT 2011
#  Adapted to H5Part 1.6.3 which can correctly build shared libraries, does
#  not require -DH5_USE_16_API in CFLAGS and has a new way to pass path to
#  HDF5.
#
# ***************************************************************************

function build_h5part
{
    #
    # Prepare build dir
    #
    prepare_build_dir $H5PART_BUILD_DIR $H5PART_FILE
    untarred_h5part=$?
    if [[ $untarred_h5part == -1 ]] ; then
        warn "Unable to prepare H5Part Build Directory. Giving Up"
        return 1
    fi

    #
    # Apply patches
    #
    apply_h5part_patch
    if [[ $? != 0 ]] ; then
        if [[ $untarred_h5part == 1 ]] ; then
            warn "Giving up on H5part build because the patch failed."
            return 1
        else
            warn "Patch failed, but continuing.  I believe that this script\n" \
                 "tried to apply a patch to an existing directory that had\n" \
                 "already been patched ... that is, the patch is\n" \
                 "failing harmlessly on a second application."
        fi
    fi

    #
    # Apply configure
    #
    info "Configuring H5Part . . ."
    cd $H5PART_BUILD_DIR || error "Can't cd to h5part build dir."
    if [[ "$DO_HDF5" == "yes" ]] ; then
        export HDF5ROOT="$VISITDIR/hdf5/$HDF5_VERSION/$VISITARCH"
        export SZIPROOT="$VISITDIR/szip/$SZIP_VERSION/$VISITARCH"
        WITHHDF5ARG="--with-hdf5=$HDF5ROOT"
        HDF5DYLIB="-L$HDF5ROOT/lib -L$SZIPROOT/lib -lhdf5 -lsz -lz"
    else
        WITHHDF5ARG="--with-hdf5"
        HDF5DYLIB=""
    fi

    if [[ "$OPSYS" == "Darwin" ]]; then
        export DYLD_LIBRARY_PATH="$VISITDIR/hdf5/$HDF5_VERSION/$VISITARCH/lib":\
               "$VISITDIR/szip/$SZIP_VERSION/$VISITARCH/lib":\
               $DYLD_LIBRARY_PATH
        SOARG="--enable-shared"
    else
        export LD_LIBRARY_PATH="$VISITDIR/hdf5/$HDF5_VERSION/$VISITARCH/lib":\
               "$VISITDIR/szip/$SZIP_VERSION/$VISITARCH/lib":\
               $LD_LIBRARY_PATH
        SOARG=""
    fi
    if [[ "$FC_COMPILER" == "no" ]] ; then
        FORTRANARGS=""
    else
        FORTRANARGS="FC=\"$FC_COMPILER\" F77=\"$FC_COMPILER\" FCFLAGS=\"$FCFLAGS\" FFLAGS=\"$FCFLAGS\" --enable-fortran"
    fi

    EXTRAARGS=""
    # detect coral and NVIDIA Grace CPU (ARM) systems, which older versions of 
    # autoconf don't detect
    if [[ "$(uname -m)" == "ppc64le" ]] ; then
         EXTRAARGS="ac_cv_build=powerpc64le-unknown-linux-gnu"
    elif [[ "$(uname -m)" == "aarch64" ]] ; then
         EXTRAARGS="ac_cv_build=aarch64-unknown-linux-gnu"
    fi

    info "Invoking command to configure H5Part"
    # In order to ensure $FORTRANARGS is expanded to build the arguments to
    # configure, we wrap the invokation in 'sh -c "..."' syntax
    set -x
    sh -c "./configure ${WITHHDF5ARG} ${OPTIONAL} CXX=\"$CXX_COMPILER\" \
       CC=\"$C_COMPILER\" CFLAGS=\"$CFLAGS $C_OPT_FLAGS\" CXXFLAGS=\"$CXXFLAGS $CXX_OPT_FLAGS\" \
       $FORTRANARGS $EXTRAARGS \
       --prefix=\"$VISITDIR/h5part/$H5PART_VERSION/$VISITARCH\""
    set +x
    if [[ $? != 0 ]] ; then
        warn "H5Part configure failed.  Giving up"
        return 1
    fi

    #
    # Build H5Part
    #
    info "Building H5Part . . . (~1 minutes)"

    $MAKE $MAKE_OPT_FLAGS
    if [[ $? != 0 ]] ; then
        warn "H5Part build failed.  Giving up"
        return 1
    fi
    info "Installing H5Part . . ."

    $MAKE install
    if [[ $? != 0 ]] ; then
        warn "H5Part build (make install) failed.  Giving up"
        return 1
    fi

    if [[ "$DO_STATIC_BUILD" == "no" && "$OPSYS" == "Darwin" ]]; then
        #
        # Make dynamic executable, need to patch up the install path and
        # version information.
        #
        info "Creating dynamic libraries for H5Part . . ."
    fi

    if [[ "$DO_GROUP" == "yes" ]] ; then
        chmod -R ug+w,a+rX "$VISITDIR/h5part"
        chgrp -R ${GROUP} "$VISITDIR/h5part"
    fi
    cd "$START_DIR"
    info "Done with H5Part"
    return 0
}

function bv_h5part_is_enabled
{
    if [[ $DO_H5PART == "yes" ]]; then
        return 1    
    fi
    return 0
}

function bv_h5part_is_installed
{
    check_if_installed "h5part" $H5PART_VERSION
    if [[ $? == 0 ]] ; then
        return 1
    fi
    return 0
}

function bv_h5part_build
{
    cd "$START_DIR"
    if [[ "$DO_H5PART" == "yes" ]] ; then
        check_if_installed "h5part" $H5PART_VERSION
        if [[ $? == 0 ]] ; then
            info "Skipping H5Part build.  H5Part is already installed."
        else
            info "Building H5Part (~1 minutes)"
            build_h5part
            if [[ $? != 0 ]] ; then
                error "Unable to build or install H5Part.  Bailing out."
            fi
            info "Done building H5Part"
        fi
    fi

}
