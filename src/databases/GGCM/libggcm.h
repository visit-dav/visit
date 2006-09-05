#ifndef GGCM_LIB_H
#define GGCM_LIB_H

#include <stdio.h>

#if __GNUC__ >= 3
# define __deprecated   __attribute__ ((deprecated))
#else
# define __deprecated   /* no deprecated */
#endif

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

typedef struct MHDdata_st {
    struct    MHDdata_st *next;    
    char        field_name[128];  /* name of the field */
    int        subsample[3];       /* amount of subsampling to do when re-writing */
    int        subsamp_dim[3];    /* the dimentions of the data after subsampling */
    /** flag of whether data is a regular or rectilinear structure */
    int        regular;
    /** flag of whether this struct should be written to file (vs. part of the
     * grid or a vector) */
    int        write_data;
    int        num_covecs;       /* the number of co-vector components */
    /** XYZ grid values stored in other MHD structures */
    struct    MHDdata_st    *grid[3];
    /* co-components with this MHD data to create a vector */
    struct    MHDdata_st    *co_vector[3];

    char        filename[256];    /* name of the file containing the MHD data */
    FILE        *mhd_fp;          /* pointer to input file structure */
    long        file_position;    /* position in the file where this MHD field begins.
                             * NOTE, was declars as "fpos_t" but that generates
                            * lots of compiler warnings */

    /** These fields are parsed from lines 1-4 of the MHD header */
    int        dim_rank;        /* number of dimensions */
    int        dim_size[3];     /* size of up to 3 dimensions */
    int        value_rank;         /* scalar(1) vs. vector(3), etc. */
    char        line3_info[128]; /* name of the file with grid information */
    int        line4_value;     /* unknown -- perhaps related to time */

    /** The "wrn2" fields are the values extracted from line 5 of the MHD
     * header. (ie. this is the required information for decoding the data[]
     * array) */
    char        encoding[128];      /* how the data is encoded -- always "WRN2" */
    int        number;          /* number of data values */
    float        wrn2_z1;         /* minimum data value in log scale */
    float        wrn2_z2;         /* maximum data value in log scale */
    float        wrn2_rid;        /* deprecated */
    int        wrn2_it;         /* deprecated -- used to be a time code*/
    char        wrn2_cid[128];   /* dimensionality & rank coding -- redundant */

    /** there is some associated time data in the data file.  I'm not exactly
     * sure if the order of the fields I have is correct, I just guessed on what
     * seemed logical. */
    double   time_since_model_start; /* that is associated w/ this data */
    double   some_time;              /* I don't know */
    unsigned int year;
    unsigned int month;
    unsigned int day;
    unsigned int hour;
    unsigned int minute;
    double second;
    
    /* The data */
    float        *data;           /* pointer to the data array */
} MHDdata;

/** initialize an MHDdata record */
void ggcm_init_mhd(MHDdata *data);
/** initialize an MHDdata record */
__deprecated void initMHD(MHDdata *data);

/** ggcm_read_mhd reads all the wrn ASCII-encoded MHD data requested
 * by the "field_list[]" character array from the file specified by the
 * "file_name" string (as long as they are in the file of course).
 * A linked list of MHDdata structures is created, and a pointer to the
 * first structure of the list is returned.  The order of the list is
 * the order the data fields were found in the file -- not necessarily
 * the order they were specified in the "field_list" array. */
MHDdata *ggcm_read_mhd(const char *file_name, const char *field_list[]);
/** readMHDFile() reads all the wrn ASCII-encoded MHD data requested
 * by the "field_list[]" character array from the file specified by the
 * "file_name" string (as long as they are in the file of course).
 * A linked list of MHDdata structures is created, and a pointer to the
 * first structure of the list is returned.  The order of the list is
 * the order the data fields were found in the file -- not necessarily
 * the order they were specified in the "field_list" array. */
__deprecated MHDdata *readMHDFile(const char *file_name,
                                  const char *field_list[], long file_start);

/** creates a list which contains all of the metadata for a given GGCM file,
 * such as the fields contained within it, data points / field, time code
 * information, etc. */
MHDdata *ggcm_read_metadata(const char *filename);

/** traverses the MHDdata structure, deleting 'data' as it goes. stop when it
 * encounters a NULL pointer. Finally, free()s the pointer to the structure. */
void ggcm_free_data(MHDdata *);
/** traverses the MHDdata structure, deleting 'data' as it goes. stop when it
 * encounters a NULL pointer. Finally, free()s the pointer to the structure. */
__deprecated void freeMHDData(MHDdata *md);

/** matrix [de]allocation routines */

__deprecated float ***matrixAllocate(int l, int m, int n);
__deprecated void matrixFree(float ***A, int l, int m, int n);

/** allocates a matrix with the specified number of dimensions */
float ***m_alloc(unsigned int x, unsigned int y, unsigned int z);
/** frees a matrix allocated via m_alloc. */
void m_free(float ***M);

/** create a GGCM grid array */
float *ggcm_grid(MHDdata *data, int *ndim, char * const grid_name);
/** create a GGCM grid array */
__deprecated float *ggcmGrid(MHDdata *data, int *ndim, char * const grid_name);

/** Our fields are stored in an MHDdata record, in no particular order
 * (actually it depends on the input order, which we don't rely on).  This
 * scans through and copies the data for the desired field out of the
 * structure. */
float *ggcm_field(MHDdata * data, int nx, int ny, int nz,
                  char * const field_name);

/** Create a 3D GGCM MHD field matrix */
float ***ggcm_field_matrix(MHDdata * const data, int nx, int ny, int nz,
                           char field_name[128]);
/** Create a 3D GGCM MHD field matrix */
__deprecated float ***ggcmFieldMatrix(const MHDdata *data,
                                      int nx, int ny, int nz,
                                      char field_name[128]);

/** Convert 1D grid arrays from MHD to GSE coordinates */
void ggcm_grid_gse(float *xg, float *yg, float *zg, 
                   float *x_gse, float *y_gse, float *z_gse,
                   const int nx, const int ny, const int nz);
/** Convert 1D grid arrays from MHD to GSE coordinates */
__deprecated void ggcmGridGSE(float *xg, float *yg, float *zg, 
                              float *x_gse, float *y_gse, float *z_gse,
                              const int nx, const int ny, const int nz);

/** Convert 3D GGCM field matrices from MHD to GSE coordinates */
void ggcm_mhd_gse(float ***fg, float ***fg_gse, int nx, int ny, int nz,
                  char field_name[128]);  
/** Convert 3D GGCM field matrices from MHD to GSE coordinates */
__deprecated void ggcmMHDGSE(float ***fg, float ***fg_gse,
                             int nx, int ny, int nz,
                             char field_name[128]);

/** converts to GSE 'in place': from a flattened 3d array.  Returns a
 * dynamically allocated array of size nx*ny*nz */
float *ggcm_mhd_gse_inplace(float * const data, int nx, int ny, int nz);

#ifdef __cplusplus
}
#endif /* __cplusplus */
#endif /* GGCM_LIB_H */
