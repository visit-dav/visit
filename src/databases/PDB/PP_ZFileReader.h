#ifndef PP_ZFILE_FORMAT_H
#define PP_ZFILE_FORMAT_H
#include <PDBReader.h>
#include <string>
#include <vector>
#include <void_ref_ptr.h>

class vtkDataArray;
class vtkDataSet;
class vtkMatrix4x4;
class avtDatabaseMetaData;

// ****************************************************************************
// Class: PP_ZFileReader
//
// Purpose:
//   This class reads a PP and Z format PDB files.
//
// Notes:      
//
// Programmer: Brad Whitlock
// Creation:   Tue Apr 29 13:51:09 PST 2003
//
// Modifications:
//   Brad Whitlock, Tue Aug 5 17:38:32 PST 2003
//   Added support for materials, a revolved mesh, and zonal variables.
//
//   Brad Whitlock, Tue Sep 16 13:54:46 PST 2003
//   I renamed the class and made a lot of methods be helper methods so
//   I can use them in derived classes without having name conflicts.
//
// ****************************************************************************

class PP_ZFileReader : public PDBReader
{
public:
    PP_ZFileReader(const char *filename);
    PP_ZFileReader(PDBFileObject *p);
    virtual ~PP_ZFileReader();

    // Methods that help implement the file format methods.
    void          PopulateDatabaseMetaData(avtDatabaseMetaData *);
    void          GetTimeVaryingInformation(int ts, avtDatabaseMetaData *);
    vtkDataSet   *GetMesh(int ts, const char *var);
    vtkDataArray *GetVar(int ts, const char *var);
    void         *GetAuxiliaryData(int ts,
                                   const char *var,
                                   const char *type,
                                   void *args,
                                   DestructorFunction &);
    int           GetNumTimeSteps();
    const int    *GetCycles();
    const double *GetTimes();
protected:
    virtual bool IdentifyFormat();

    // Internal helper functions.
    void Initialize();
    void InitializeVarStorage();
    void ReadVariable(const std::string &var);
    void CreateGhostZones(const int *, vtkDataSet *);
    bool VariableIsNodal(const std::string &var) const;
    void ReadMixvarAndCache(const std::string &var, int state);
    const int *GetIreg(int state);
    int  GetUnstructuredCellCount();
    bool PopulateMaterialNames();

    static void GetRotationMatrix(double angle, const double axis[3],
                                  vtkMatrix4x4 *mat);
    static vtkDataSet *RevolveDataSet(vtkDataSet *in_ds, const double *axis,
                                      double start_angle, double stop_angle,
                                      int nsteps);
    // Data members
    int                      kmax;
    int                      lmax;
    bool                     meshDimensionsKnown;
    int                      unstructuredCellCount;
    std::string              rtVar;
    std::string              ztVar;
    bool                     formatIdentified;
    bool                     initialized;
    int                     *cycles;
    int                      nCycles;
    double                  *times;
    int                      nTimes;
    std::vector<std::string> materialNames;
    bool                     assumeMixedMaterialsPresent;
    VariableDataMap          varStorage;
    bool                     varStorageInitialized;
    std::vector<std::string> nodalVars;

    static const int         revolutionSteps;
};

#endif
