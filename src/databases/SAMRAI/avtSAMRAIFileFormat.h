// ************************************************************************* //
//                             avtSAMRAIFileFormat.h                           //
// ************************************************************************* //

#ifndef AVT_SAMRAI_FILE_FORMAT_H
#define AVT_SAMRAI_FILE_FORMAT_H

#include <avtMTMDFileFormat.h>

#include <vector>
#include <string>
#include <map>
#include <fstream.h>
#include <hdf5.h>

// ****************************************************************************
//  Class: avtSAMRAIFileFormat
//
//  Purpose:
//      A file format reader for multi-timestep, multi-domain SAMRAI files.
//
//  Programmer:  Jeremy Meredith, Walter Herrera-Jimenez
//  Creation:    July 7, 2003
//
// ****************************************************************************

class avtSAMRAIFileFormat : public avtMTMDFileFormat
{
  public:
                          avtSAMRAIFileFormat(const char *);
    virtual              ~avtSAMRAIFileFormat();
    
    virtual const char   *GetType(void) { return "SAMRAI File Format"; };
    
    virtual void          GetCycles(std::vector<int> &);
    virtual int           GetNTimesteps(void);
 
    virtual vtkDataSet   *ReadMesh(int, int, const char *);
    virtual vtkDataSet   *GetMesh(int, int, const char *);
    virtual vtkDataArray *GetVar(int, int, const char *);
    virtual vtkDataArray *GetVectorVar(int, int, const char *);

    virtual void          PopulateDatabaseMetaData(avtDatabaseMetaData *);

  protected:
    typedef struct {
      int proc; 
      int cluster;
      int level;
      int patch_num;
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
    int                           num_timesteps;
    std::string                   file_name;
    std::string                   dir_name;
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

