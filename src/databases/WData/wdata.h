/**
 * W-SLDA Toolkit
 * This file is taken from repository
 * https://gitlab.fizyka.pw.edu.pl/wtools/wdata
 * which provides C implementation of WData format under GNU GPLv3 license. 
 * */

/*****************************************************************************
// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.
*****************************************************************************/

#ifndef __W_DATA_LIB__
#define __W_DATA_LIB__

#include <stdio.h>

#define MD_CHAR_LGTH      256
#define MD_VARNAME_LGTH    64
#define WDATA_MAX_NVARS   128
#define WDATA_MAX_NLINKS  128
#define WDATA_MAX_NCONSTS 128
#define WDATA_MAX_NTXT    128

// Version: 0.3.0
#define VERSION_MAJOR 0
#define VERSION_MINOR 3
#define VERSION_PATCH 0

#define WDATA_OK 0

// Errors
#define WDATA_ERROR_WTXTFILE_INCOMPLETE 30000
#define WDATA_ERROR_WTXTFILE_INCOMPLETE_DIM1_N 30001       // did not provide nx (default 0), when datadim is 1
#define WDATA_ERROR_WTXTFILE_INCOMPLETE_DIM2_N 30002       // did not provide nx, ny (default 0), when datadim is 2
#define WDATA_ERROR_WTXTFILE_INCOMPLETE_DIM3_N 30003       // did not provide nx, ny, nz (default 0), when datadim is 3
#define WDATA_ERROR_WTXTFILE_INCOMPLETE_DIM1_D 30004       // did not provide dx (default 0.0), when datadim is 1
#define WDATA_ERROR_WTXTFILE_INCOMPLETE_DIM2_D 30005       // did not provide dx, dy (default 0.0), when datadim is 2
#define WDATA_ERROR_WTXTFILE_INCOMPLETE_DIM3_D 30006       // did not provide dx, dy, dz (default 0.0), when datadim is 3
#define WDATA_ERROR_WTXTFILE_INCOMPLETE_DATADIM1_N 30007   // number of provided dimensions does not match with datadim == 1
#define WDATA_ERROR_WTXTFILE_INCOMPLETE_DATADIM2_N 30008   // number of provided dimensions does not match with datadim == 2
#define WDATA_ERROR_WTXTFILE_INCOMPLETE_DATADIM3_N 30009   // number of provided dimensions does not match with datadim == 3
#define WDATA_ERROR_WTXTFILE_INCOMPLETE_DATADIM1_D 30010   // number of provided lattice spacings does not match with datadim == 1
#define WDATA_ERROR_WTXTFILE_INCOMPLETE_DATADIM2_D 30011   // number of provided lattice spacings does not match with datadim == 2
#define WDATA_ERROR_WTXTFILE_INCOMPLETE_DATADIM3_D 30012   // number of provided lattice spacings does not match with datadim == 3
#define WDATA_ERROR_WTXTFILE_INCOMPLETE_EMPTY_N 30013      // did not provide nx, ny, nz
#define WDATA_ERROR_WTXTFILE_INCOMPLETE_EMPTY_D 30014      // did not provide dx, dy, dz
#define WDATA_ERROR_WTXTFILE_INCOMPLETE_PREFIX 30015       // did not provide prefix (default '\0')
#define WDATA_ERROR_WTXTFILE_INCOMPLETE_CONST_ITAG2 300016 // did not provide enough number of tags (2) in `consts`
#define WDATA_ERROR_WTXTFILE_INCOMPLETE_VAR_ITAG2 30017    // did not provide enough number of tags (2) in `vars`
#define WDATA_ERROR_WTXTFILE_NOTSUPPORTED_VAR_FORMAT 30018 // did not provide correct data formats
#define WDATA_ERROR_WDATA_ADD_DATABLOCK_WCM 30019          // error with `wdata_add_datablock()` function in `write_cycle_many()`
#define WDATA_ERROR_WDATA_GET_VARIABLE_WCM 30020           // error with `wdata_get_variable()` function in `write_cycle_many()`
#define WDATA_ERROR_WDATA_GET_TIME 30021                   // error with `wdata_add_time()` implemented only when `dt` is varying (set when negative)
#define WDATA_ERROR_OUT_OF_DOMAIN 30022                   // error with `wdata_get_x()` , `wdata_get_y()`, `wdata_get_z()` or `wdata_get_time()`
#define WDATA_ERROR_CANNOT_OPEN__T 30023                   // cannot open prefix__t.wdat file
#define WDATA_ERROR_CANNOT_READ__T 30024                   // cannot read prefix__t.wdat file
#define WDATA_ERROR_CANNOT_WRIT__T 30025                   // cannot write prefix__t.wdat file
#define WDATA_ERROR_CANNOT_OPEN__X 30026                   // cannot open prefix__x.wdat file
#define WDATA_ERROR_CANNOT_READ__X 30027                   // cannot read prefix__x.wdat file
#define WDATA_ERROR_CANNOT_WRIT__X 30028                   // cannot write prefix__x.wdat file
#define WDATA_ERROR_CANNOT_OPEN__Y 30029                   // cannot open prefix__y.wdat file
#define WDATA_ERROR_CANNOT_READ__Y 30030                   // cannot read prefix__y.wdat file
#define WDATA_ERROR_CANNOT_WRIT__Y 30031                   // cannot write prefix__y.wdat file
#define WDATA_ERROR_CANNOT_OPEN__Z 30032                   // cannot open prefix__z.wdat file
#define WDATA_ERROR_CANNOT_READ__Z 30033                   // cannot read prefix__z.wdat file
#define WDATA_ERROR_CANNOT_WRIT__Z 30034                   // cannot write prefix__z.wdat file
#define WDATA_ERROR_FORMAT_WRITE_NOT_SUPPORTED 30035       // functionality not supported for the format
#define WDATA_ERROR_FORMAT_READ_NOT_SUPPORTED 30035       // functionality not supported for the format
#define WDATA_ERROR_CANNOT_OPEN_WDAT 30036                 // cannot open wdat file
#define WDATA_ERROR_CANNOT_READ_WDAT 30037                 // cannot read wdat file
#define WDATA_ERROR_CANNOT_WRIT_WDAT 30038                 // cannot write to wdat file
#define WDATA_ERROR_CANNOT_OPEN_NPY 30039                 // cannot open npy file
#define WDATA_ERROR_CANNOT_READ_NPY 30040                 // cannot read npy file
#define WDATA_ERROR_CANNOT_WRIT_NPY 30041                 // cannot write to npy file
#define WDATA_ERROR_CANNOT_GET_TXT_FULLNAME 30042         // wdata_get_txt_fullname returns the fullname of a nonexisting file
#define WDATA_ERROR_VECTOR_TYPE 30043                     // incorrect format of vector type
#define WDATA_ERROR_NOT_VECTOR 30044                     // variable if not of vector type

#ifdef __cplusplus
extern "C"
{
#endif

  typedef struct
  {
    char name[MD_VARNAME_LGTH];
    char type[MD_VARNAME_LGTH];
    char unit[MD_VARNAME_LGTH];
    char format[8]; // one of wdat, npy, dpca
  } wdata_variable;

  typedef struct
  {
    char *names[WDATA_MAX_NVARS];
    char *types[WDATA_MAX_NVARS];
    char *units[WDATA_MAX_NVARS];
    char *formats[WDATA_MAX_NVARS]; // one of wdat, npy, dpca
  } wdata_variable_many;

  typedef struct
  {
    char name[MD_VARNAME_LGTH];
    char linkto[MD_VARNAME_LGTH];
  } wdata_link;

  typedef struct
  {
    char name[MD_VARNAME_LGTH];
    double value;
    char unit[MD_VARNAME_LGTH];
  } wdata_const;

  typedef struct
  {
    char filename[MD_CHAR_LGTH];
  } wdata_txt;

  typedef struct
  {
    int nx;
    int ny;
    int nz;
    double dx;
    double dy;
    double dz;
    double x0;
    double y0;
    double z0;
    int datadim;               /// dimension of block size 1=nx, 2=nx*ny, 3=nx*ny*nz
    char prefix[MD_CHAR_LGTH]; /// prefix for files belonging to this data set, binary files have names prefix_variable.wdat
    int cycles;                /// number of cycles (measurements)
    double t0;                 /// time value for the first mesurments
    double dt;                 /// time interval between cycles
    int nvar;                  /// number of variables single
    int nlink;                 /// number of links
    int nconsts;               /// number of constants
    int ntxt;                  /// number of txt files

    // variables
    wdata_variable var[WDATA_MAX_NVARS];
    wdata_variable_many vars[WDATA_MAX_NVARS];

    // links
    wdata_link link[WDATA_MAX_NLINKS];

    // constants
    wdata_const consts[WDATA_MAX_NCONSTS];

    // txts
    wdata_txt txt[WDATA_MAX_NTXT];

    // auxliary vars
    int issetwrkdir;
    char wrkdir[MD_CHAR_LGTH]; // working directory

  } wdata_metadata;

  /**
   * Function resets metadata.
   * It is recommended to use it after creating a metadata object, to avoid uncontrolled values of the structure fields.
   * */
  void wdata_reset_metadata(wdata_metadata *md);

  /**
 * Basic functions for extracting the lattice
 * */
  int wdata_getnx(wdata_metadata *md);
  int wdata_getny(wdata_metadata *md);
  int wdata_getnz(wdata_metadata *md);
  double wdata_getdx(wdata_metadata *md);
  double wdata_getdy(wdata_metadata *md);
  double wdata_getdz(wdata_metadata *md);
  /**
 * Basic functions for setting the lattice
 * */
  void wdata_setnx(wdata_metadata *md, int nx);
  void wdata_setny(wdata_metadata *md, int ny);
  void wdata_setnz(wdata_metadata *md, int nz);
  void wdata_setdx(wdata_metadata *md, double dx);
  void wdata_setdy(wdata_metadata *md, double dy);
  void wdata_setdz(wdata_metadata *md, double dz);
  void wdata_setx0(wdata_metadata *md, double x0);
  void wdata_sety0(wdata_metadata *md, double y0);
  void wdata_setz0(wdata_metadata *md, double z0);
  void wdata_setprefix(wdata_metadata *md, const char *prefix);
  /**
 * Basic functions for getting the coordinate
 * */
  int wdata_get_x(const wdata_metadata *md, const int ix, double *x);
  int wdata_get_y(const wdata_metadata *md, const int iy, double *y);
  int wdata_get_z(const wdata_metadata *md, const int iz, double *z);

  /**
 * Function sets lattice size in struct wdata_metadata
 * @return 0: ok, 1: error,
 * */
  int wdata_set_lattice(wdata_metadata *md, int nx, int ny, int nz, double dx, double dy, double dz);

  /**
 * Function adds time to file prefix__t.wdat. Use it with negative dt.
 * @return error code
 * */
  int wdata_add_time(const wdata_metadata *md, const int icycle, double *time);

  /**
 * Function adds coordinate to file prefix__x.wdat. Use it with negative dx.
 * @return error code
 * */
  int wdata_add_x(const wdata_metadata *md, double *x);

  /**
 * Function adds coordinate to file prefix__y.wdat. Use it with negative dy.
 * @return error code
 * */
  int wdata_add_y(const wdata_metadata *md, double *y);

  /**
 * Function adds coordinate to file prefix__z.wdat. Use it with negative dz.
 * @return error code
 * */
  int wdata_add_z(const wdata_metadata *md, double *z);

  /**
 * Function reads the time for the given cycle
 * @param md metadata structure
 * @param icycle cycle id
 * @param current_time value of time (OUTPUT)
 * @return error code
 * */
  int wdata_get_time(const wdata_metadata *md, const int icycle, double *current_time);

  /**
 * Function reads metadata
 * and puts values into struct wdata_metadata
 * @return 0: ok, 1: Cannot open metadata file,
 * */
  int wdata_parse_metadata_file(const char *file_name, wdata_metadata *md);

  void wdata_print_metadata(wdata_metadata *md, FILE *out);
  void wdata_print_variable(wdata_variable *md, FILE *out);
  void wdata_print_link(wdata_link *md, FILE *out);
  void wdata_print_const(wdata_const *md, FILE *out);
  void wdata_print_txt(wdata_txt *md, FILE *out);

  void wdata_add_variable(wdata_metadata *md, wdata_variable *var);
  void wdata_add_variable_many(wdata_metadata *md, wdata_variable_many *var_many);
  void wdata_add_link(wdata_metadata *md, wdata_link *link);
  void wdata_add_const(wdata_metadata *md, wdata_const *_const);
  void wdata_add_txt(wdata_metadata *md, wdata_txt *txt);

  /**
 * @return blocklength = nx       for datadim=1
 *                     = nx*ny    for datadim=2
 *                     = nx*ny*nz for datadim=3
 * */
  int wdata_get_blocklength(wdata_metadata *md);

  /**
 * @return blocklength_full = blocklength for real
 *                          = blocklength*2 for complex (re,im)
 *                          = blocklength*d for vector(d)
 * */
  int wdata_get_blocklength_full(wdata_metadata *md, wdata_variable *var);

  /**
 * @return size of block in bytes for given variable
 * */
  size_t wdata_get_blocksize(wdata_metadata *md, wdata_variable *var);

  /**
 * Function decomposes a vector variable into precision type and dimensionality.
 * Example: for var.type="vector8(3)" it will return var="vector8" and dim=3.
 * @return error code
 * */
  int wdata_vectorvar_decompose(wdata_variable *var, char *type, int *dim);

  /**
 * @return precision of the  variable: d - double, f - float
 * */
  char wdata_get_var_precision(wdata_variable *var);

  /**
 * Low level function. It adds block of data to binary file.
 * @param md metadata for data set
 * @param var variable to be added to file with name `prefix`_`varname`.wdat
 * @param data pointer to binary data (INPUT)
 * @return 0: ok; 1: cannot open binary file; 2: cannot add datablock to file
 * */
  int wdata_add_datablock(wdata_metadata *md, wdata_variable *var, void *data);
  int wdata_add_datablock_wdat(wdata_metadata *md, wdata_variable *var, void *data);
  int wdata_add_datablock_dpca(wdata_metadata *md, wdata_variable *var, void *data);
  int wdata_add_datablock_npy(wdata_metadata *md, wdata_variable *var, void *data);

  int wdata_load_datablock(wdata_metadata *md, wdata_variable *var, int cycle, void *data);
  int wdata_load_datablock_wdat(wdata_metadata *md, wdata_variable *var, int cycle, void *data);
  int wdata_load_datablock_dpca(wdata_metadata *md, wdata_variable *var, int cycle, void *data);
  int wdata_load_datablock_npy(wdata_metadata *md, wdata_variable *var, int cycle, void *data);

  /**
 * Function adds new block to data file.
 * @param md metadata for data set
 * @param varname name of variable, can be from list of vars or links
 * @param data pointer to binary data (INPUT)
 * @return 0: ok; 1: cannot open binary file; 2: cannot add datablock to file, 11: variable is not defined
 * */
  int wdata_write_cycle(wdata_metadata *md, const char *varname, void *data);
  int wdata_write_cycle_d(wdata_metadata *md, const char *varname, double *data);
  int wdata_write_cycle_f(wdata_metadata *md, const char *varname, float *data);

  /**
 * Function adds new block to data file.
 * @param md metadata for data set
 * @param var_many wdata_variable_many struct
 * @param data pointer to lise of binary data (INPUT)
 * @return 0: ok; 1: cannot open binary file; 2: cannot add datablock to file, 11: variable is not defined
 * */
  int wdata_write_cycle_many(wdata_metadata *md, const wdata_variable_many *var_many, void *data[]);
  int wdata_write_cycle_many_d(wdata_metadata *md, const wdata_variable_many *var_many, double *data[]);
  int wdata_write_cycle_many_f(wdata_metadata *md, const wdata_variable_many *var_many, float *data[]);

  /**
 * Function reads block of data from file
 * @param md metadata for data set
 * @param varname name of variable, can be from list of vars or links
 * @param data pointer to binary data (OUTPUT)
 * @return 0: ok; 1: cannot open binary file; 2: cannot read data block from file; 3: cannot shift pointer;  11: variable is not defined
 * */
  int wdata_read_cycle(wdata_metadata *md, const char *varname, int cycle, void *data);
  int wdata_read_cycle_d(wdata_metadata *md, const char *varname, int cycle, double *data);
  int wdata_read_cycle_f(wdata_metadata *md, const char *varname, int cycle, float *data);

  void wdata_get_filename(wdata_metadata *md, wdata_variable *var, char *file_name);

  /**
 * Functions extracts variable corresponding to given name
 * @param md metadata for data set
 * @param varname name of variable, can be from list of vars or links
 * @param var pointer to variable from md structure (OUTPUT)
 * @return 0: ok; 1: cannot find variable
 * */
  int wdata_get_variable(wdata_metadata *md, const char *varname, wdata_variable *var);

  int wdata_get_const(wdata_metadata *md, const char *constname, wdata_const *_const);

  /**
 * @return value of constant from metadata structure, if const is not found zero is returned
 * */
  double wdata_getconst_value(wdata_metadata *md, const char *constname);

  /**
 * Functions sets value of constant. If constant was not added before it adds it and sets its value.
 * */
  void wdata_setconst(wdata_metadata *md, const char *constname, double constvalue);

  /**
 * Functions sets value of constant and corresponding unit. If constant was not added before it adds it and sets its value.
 * */
  void wdata_setconst_unit(wdata_metadata *md, const char *constname, double constvalue, const char *unit);

  /**
 * Function checks if binary file exists for variable.
 * @return 1 if binary file for this variable exists, otherwise 0
 */
  int wdata_file_exists(wdata_metadata *md, const char *varname);

  /**
 * Function removes binary file associated with variable varname.
 * */
  void wdata_clear_file(wdata_metadata *md, const char *varname);

  /**
 * Removes all binary data files, except metadata file, and sets cycles to zero.
 * */
  void wdata_clear_database(wdata_metadata *md);

  /**
 * Functions increases number of cycles.
 * It is equivalent to: md->cycles++;
 * */
  int wdata_add_cycle(wdata_metadata *md);

  /**
 * Function writes metadata to file.
 * @param md metadata to be written
 * @param filename name of file to write md, if filename is empty string then default name of file will be used `prefix`.wtxt
 * @return 0: ok, 1: cannot create file
 * */
  int wdata_write_metadata_to_file(wdata_metadata *md, const char *filename);

  /**
 * Function sets working dir for given metadata.
 * Working dir defines directory where all binary files will be stored.
 * Default is working dir of code that executes wdata functions.
 * */
  void wdata_set_working_dir(wdata_metadata *md, const char *wkrdir);

  /**
 * Function adds at the end of file new entry for variable
 * @return 0: ok, 1: cannot add entry to file
 * */
  int wdata_add_var_to_metadata_file(const char *file_name, wdata_variable *var);

  /**
 * Function adds at the end of file new entry for link
 * @return 0: ok, 1: cannot add entry to file
 * */
  int wdata_add_link_to_metadata_file(const char *file_name, wdata_link *link);

  /**
 * Function adds at the end of file new entry for link
 * @return 0: ok, 1: cannot add entry to file
 * */
  int wdata_add_const_to_metadata_file(const char *file_name, wdata_const *_const);

  int wdata_add_txt_to_metadata_file(const char *file_name, wdata_txt *txt);

  /**
 * Function adds at the end of file new comment
 * @return 0: ok, 1: cannot add entry to file
 * */
  int wdata_add_comment_to_metadata_file(const char *file_name, const char *comment);

  /**
 * Function checks if dataset has variable of name varname
 * @return 1: dataset has this variable, 0: no variable in given dataset
 * */
  int wdata_has_variable(wdata_metadata *md, const char *varname);

  /**
 * Function checks if dataset has link of name linkname
 * @return 1: dataset has this link, 0: no link in given dataset
 * */
  int wdata_has_link(wdata_metadata *md, const char *linkname);

  /**
 * Function checks if dataset has constant of name constname
 * @return 1: dataset has this constant, 0: no constant in given dataset
 * */
  int wdata_has_const(wdata_metadata *md, const char *constname);

  /**
 * Function checks if dataset has txt of name filename
 * @return 1: dataset has this txt, 0: no txt in given dataset
 * */
  int wdata_has_txt(wdata_metadata *md, const char *filename);

  /**
 * Function generates full name of txt file.
 * fullname = prefix_filename
 * @return error code
 * */
  int wdata_get_txt_fullname(wdata_metadata *md, const char *filename, char *fullname);

#ifdef __cplusplus
} // closing brace for extern "C"
#endif

#endif
