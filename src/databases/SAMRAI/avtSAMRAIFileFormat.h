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

using std::string;
using std::vector;
using std::map;

class avtMaterial;
class avtSpecies;

// ****************************************************************************
//  Class: avtSAMRAIFileFormat
//
//  Purpose:
//      A file format reader for single-timestep, multi-domain SAMRAI files.
//
//  Programmer:  Walter Herrera-Jimenez
//  Creation:    July 7, 2003
//
//  Modifications:
//
//    Mark C. Miller, Mon Dec  8 13:54:21 PST 2003
//    Added constants for expected version number, and enviornment variable
//    used to disable version checking. Added var_num_ghosts array. Added
//    prototype for RegisterVariableList
//
//    Kathleen Bonnell, Fri Dec 19 10:20:57 PST 2003 
//    Moved 'expected_version_number' to source file so that code will build 
//    on IRIX.
// 
// ****************************************************************************

class avtSAMRAIFileFormat : public avtSTMDFileFormat
{
  public:
                          avtSAMRAIFileFormat(const char *);
                         ~avtSAMRAIFileFormat();

    bool                  HasInvariantMetaData(void) const { return false; };
    bool                  HasInvariantSIL(void) const { return false; };
    bool                  CanCacheVariable(const char *);
    
    virtual const char   *GetType(void) { return "SAMRAI File Format"; };
    
    vtkDataSet           *GetMesh(int, const char *);
    vtkDataArray         *GetVar(int, const char *);
    vtkDataArray         *GetVectorVar(int, const char *);
    avtMaterial          *GetMaterial(int, const char *);
    avtSpecies           *GetSpecies(int, const char *);

    void                 *GetAuxiliaryData(const char *var, int,
                                           const char *type, void *args,
                                           DestructorFunction &);
    int                   GetCycle(void) { return time_step_number; };

    void                  PopulateDatabaseMetaData(avtDatabaseMetaData *);
    void                  RegisterVariableList(const char *,
                                               const vector<CharStrRef> &);

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
      int num_ghosts[3];
    } var_t; 

    typedef struct {
      int offset; 
      int number_children;
    } child_t;

    typedef struct {
      int offset; 
      int number_parents;
    } parent_t;

    typedef struct {
      unsigned char data_is_defined;
      int mat_comp_flag;
      int spec_comp_flag;
      double min;
      double max;
    } matinfo_t;

    vtkDataSet                 ***cached_patches;
    string                        file_name;
    string                        dir_name;

    string                        grid_type;

    int                           time_step_number;
    double                        time;
    string                        time_dump;

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
    int                          *var_num_ghosts;
    string                       *var_names;
    map<string, var_t>            var_names_num_components;

    bool                          has_mats;
    bool                          inferVoidMaterial;
    int                           num_mats;
    string                       *mat_names;
    int                          *mat_num_ghosts;
    int                           num_mat_vars;
    int                          *mat_var_num_components;
    string                       *mat_var_names;
    map<string, var_t>            mat_var_names_num_components;
    map<string, matinfo_t*>       mat_names_matinfo;
    map<string, map<string, matinfo_t*> > mat_var_names_matinfo;

    bool                          has_specs;
    int                           num_spec_vars;
    string                       *spec_var_names;
    int                          *nmatspec;
    map<string, string*>          mat_specs;
    map<string, map<string, matinfo_t*> > mat_specs_matinfo;

    var_extents_t               **var_extents; 
    patch_extents_t              *patch_extents;
    patch_map_t                  *patch_map;

    int                          *child_array;
    int                           child_array_length;
    child_t                      *child_pointer_array;
    int                          *parent_array;
    int                           parent_array_length;
    parent_t                     *parent_pointer_array;

    bool                          has_ghost;
    bool                          ghosting_is_consistent;

    string                        active_visit_var_name;
    string                        last_visit_var_name;
    int                           last_patch;

    static int           objcnt;
    static void          InitializeHDF5();
    static void          FinalizeHDF5();

    virtual vtkDataSet   *ReadMesh(int);
    vtkDataArray         *ReadVar(int patch, const char *visit_var_name);

    void            ReadAndCheckVDRVersion(hid_t &h5_file);

    void            ReadGridType(hid_t &h5_file);

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
    void            ReadVarNumGhosts(hid_t &h5_file);
    
    void            ReadVarExtents(hid_t &h5_file);
    void            ReadPatchExtents(hid_t &h5_file);
    void            ReadPatchMap(hid_t &h5_file);
    
    void            ReadChildArrayLength(hid_t &h5_file);
    void            ReadChildArray(hid_t &h5_file);
    void            ReadChildPointerArray(hid_t &h5_file);
    void            ReadParentArrayLength(hid_t &h5_file);
    void            ReadParentArray(hid_t &h5_file);
    void            ReadParentPointerArray(hid_t &h5_file);

    void            ReadMaterialInfo(hid_t &h5_file);
    float          *ReadMatSpecFractions(int, string, string="");
    void            ConvertVolumeFractionFields(vector<int> matIds, float **vfracs,
                        int ncells, int* &matfield, int &mixlen, int* &mix_mat,
                        int* &mix_next, int* &mix_zone, float* &mix_vf);
    char            DebugMixedMaterials(int ncells, int* &matfield, int* &mix_next,
                        int* &mix_mat, float* &mix_vf, int* &mix_zone);

    void            ReadSpeciesInfo(hid_t &h5_file);
    void            ConvertMassFractionFields(vector<int> matIds,
                        vector<float**> sfracs, int ncells, avtMaterial *mat,
                        int* &speclist, int &nspecies_mf, float* &species_mf,
                        int* &mixList);

    void            BuildDomainAuxiliaryInfo();

    void            ReadMetaDataFile();

    int             GetGhostCodeForVar(const char * visit_var_name);

    void            ReadDataset(hid_t &h5_file, const char *dspath, const char *typeName,
                        int ndims, int *dims, void **data, bool isOptional=true);
};


#define DEFAULT_SIZE    1024
#define DEFAULT_MINC    2.0

class UniqueSpeciesMF
{
  public:
      UniqueSpeciesMF(int initSize = DEFAULT_SIZE, float _minc = DEFAULT_MINC);
     ~UniqueSpeciesMF();

      int IndexOfMFVector(float *mf_vector, int nvals);

      float *GetData();
      int    GetSize();
      int    GetMaxSize();

  private:

      static   int DefaultSize;
      static float DefaultMinc;

      // this is solely to control default construction of mapped values
      struct Index {
          int idx;
          Index() : idx(-1) {}
      };

      map<unsigned int,Index> vmap;

      bool dataRetrieved;

      float minc;
      int max_nspecies_mf;

      int nspecies_mf;
      float *species_mf;
};

#endif
