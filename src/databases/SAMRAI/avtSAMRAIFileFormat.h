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
//  Programmer:  Jeremy Meredith, Walter Herrera-Jimenez
//  Creation:    July 7, 2003
//
// ****************************************************************************

class avtSAMRAIFileFormat : public avtSTMDFileFormat
{
  public:
                          avtSAMRAIFileFormat(const char *);
    virtual              ~avtSAMRAIFileFormat();
    
    virtual const char   *GetType(void) { return "SAMRAI File Format"; };
    
    virtual vtkDataSet   *ReadMesh(int, const char *);
    virtual vtkDataSet   *GetMesh(int, const char *);
    virtual vtkDataArray *GetVar(int, const char *);
    virtual vtkDataArray *GetVectorVar(int, const char *);
    virtual void         *GetAuxiliaryData(const char *var, int,
                                           const char *type, void *args,
                                           DestructorFunction &);
    virtual int           GetCycle(void) { return dump_number; };

    virtual void          PopulateDatabaseMetaData(avtDatabaseMetaData *);

  protected:
    typedef struct {
      int processor_number; 
      int file_cluster_number;
      int level_number;
      int patch_number;
    } patch_map_t;

    typedef struct {
      std::vector<int>   lower;
      std::vector<int>   upper;
      std::vector<float> xlo;
      std::vector<float> xup;
    } patch_extents_t;

    typedef struct {
      float min; 
      float max;
    } var_extents_t;

    typedef struct {
      int num_components; 
      int cell_centered;
    } var_t;

    vtkDataSet                  **cached_patches;
    std::string                   file_name;
    std::string                   dir_name;
    bool                          is_rectilinear;
    int                           dump_number;
    double                       *xlo;
    double                       *dx;
    int                          *num_patches_level;
    int                          *ratios_coarser_levels;
    int                          *var_cell_centered;
    int                          *var_num_components;
    std::string                  *var_names;
    std::map<std::string, var_t> var_names_num_components;
    var_extents_t               **var_extents; 
    patch_extents_t              *patch_extents;
    patch_map_t                  *patch_map;
    int                           num_clusters;
    int                           num_dim_problem;
    int                           num_levels;
    int                           num_patches;
    int                           num_procs;
    int                           num_vars;
    double                        time;

    void                ReadMetaDataFile(hid_t &file);
    unsigned int        GetPatchOffset(const int level, const int patch);
};


#endif

