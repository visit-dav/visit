// ************************************************************************* //
//                             avtSAMRAIFileFormat.h                           //
// ************************************************************************* //

#ifndef AVT_SAMRAI_FILE_FORMAT_H
#define AVT_SAMRAI_FILE_FORMAT_H

#include <avtSTMDFileFormat.h>

#include <vector>
#include <string>
#include <map>
#include <fstream.h>
#include <hdf5.h>

// ****************************************************************************
//  Class: avtSAMRAIFileFormat
//
//  Purpose:
//      A file format reader for single-timestep, multi-domain SAMRAI files.
//
//  Programmer:  Walter Herrera-Jimenez
//  Creation:    July 7, 2003
//
// ****************************************************************************

class avtSAMRAIFileFormat : public avtSTMDFileFormat
{
  public:
                          avtSAMRAIFileFormat(const char *);
    virtual              ~avtSAMRAIFileFormat();
    
    virtual const char   *GetType(void) { return "SAMRAI File Format"; };
    
    virtual vtkDataSet   *GetMesh(int, const char *);
    virtual vtkDataArray *GetVar(int, const char *);
    virtual vtkDataArray *GetVectorVar(int, const char *);
    virtual void         *GetAuxiliaryData(const char *var, int,
                                           const char *type, void *args,
                                           DestructorFunction &);
    virtual int           GetCycle(void) { return time_step_number; };

    virtual void          PopulateDatabaseMetaData(avtDatabaseMetaData *);

  protected:
    typedef struct {
      int processor_number; 
      int file_cluster_number;
      int level_number;
      int patch_number;
    } patch_map_t;

    typedef struct {
      int    lower[3];
      int    upper[3];
      double xlo[3];
      double xup[3];
    } patch_extents_t;

    typedef struct {
      char  data_is_defined;
      double min; 
      double max;
    } var_extents_t;

    typedef struct {
      int num_components; 
      int cell_centered;
    } var_t; 

    typedef struct {
      int offset; 
      int number_children;
    } child_t;

    typedef struct {
      int offset; 
      int number_parents;
    } parent_t;

    vtkDataSet                  **cached_patches;
    std::string                   file_name;
    std::string                   dir_name;

    std::string                   grid_type;
    std::string                   data_type;

    int                           time_step_number;
    double                        time;
    std::string                   time_dump;

    double                       *xlo;
    double                       *dx;

    int                           num_dim_problem;

    int                           num_levels;
    int                           num_patches;
    int                          *num_patches_level;
    int                          *ratios_coarser_levels;

    int                           num_clusters;
    int                           num_procs;

    int                           num_vars;
    int                          *var_cell_centered;
    int                          *var_num_components;
    std::string                  *var_names;
    std::map<std::string, var_t> var_names_num_components;

    var_extents_t               **var_extents; 
    patch_extents_t              *patch_extents;
    patch_map_t                  *patch_map;

    int                          *child_array;
    int                           child_array_length;
    child_t                      *child_pointer_array;
    int                          *parent_array;
    int                           parent_array_length;
    parent_t                     *parent_pointer_array;


    virtual vtkDataSet   *ReadMesh(int);

    void            ReadGridType(hid_t &h5_file);
    void            ReadDataType(hid_t &h5_file);

    void            ReadTime(hid_t &h5_file);
    void            ReadTimeStepNumber(hid_t &h5_file);
    void            ReadTimeOfDump(hid_t &h5_file);
    
    void            ReadNumDimensions(hid_t &h5_file);
    
    void            ReadXLO(hid_t &h5_file);
    void            ReadDX(hid_t &h5_file);
    
    void            ReadNumLevels(hid_t &h5_file);
    void            ReadNumPatches(hid_t &h5_file);
    void            ReadNumPatchesLevel(hid_t &h5_file);
    void            ReadRatiosCoarserLevels(hid_t &h5_file);
    
    void            ReadNumClusters(hid_t &h5_file);
    void            ReadNumProcessors(hid_t &h5_file);
    
    void            ReadNumVariables(hid_t &h5_file);
    void            ReadVarCellCentered(hid_t &h5_file);
    void            ReadVarNames(hid_t &h5_file);
    void            ReadVarNumComponents(hid_t &h5_file);
    
    void            ReadVarExtents(hid_t &h5_file);
    void            ReadPatchExtents(hid_t &h5_file);
    void            ReadPatchMap(hid_t &h5_file);
    
    void            ReadChildArrayLength(hid_t &h5_file);
    void            ReadChildArray(hid_t &h5_file);
    void            ReadChildPointerArray(hid_t &h5_file);
    void            ReadParentArrayLength(hid_t &h5_file);
    void            ReadParentArray(hid_t &h5_file);
    void            ReadParentPointerArray(hid_t &h5_file);

    void            ReadMetaDataFile();
    unsigned int    GetPatchOffset(const int level, const int patch);
};


#endif

