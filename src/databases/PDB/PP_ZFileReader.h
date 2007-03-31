#ifndef PP_ZFILE_READER_H
#define PP_ZFILE_READER_H
#include <PDBReader.h>
#include <string>
#include <vector>

class vtkDataSet;
class vtkMatrix4x4;

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
// ****************************************************************************

class PP_ZFileReader : public PDBReader
{
public:
    PP_ZFileReader(PDBfile *pdb, avtVariableCache *c);
    virtual ~PP_ZFileReader();

    virtual bool Identify();
    virtual void GetTimeVaryingInformation(int, avtDatabaseMetaData *);
    virtual void PopulateDatabaseMetaData(avtDatabaseMetaData *);

    virtual void          GetCycles(std::vector<int> &cycles);
    virtual int           GetNTimesteps();
    virtual vtkDataSet   *GetMesh(int, const char *);
    virtual vtkDataArray *GetVar(int, const char *);
    virtual vtkDataArray *GetVectorVar(int, const char *);

    virtual void         *GetAuxiliaryData(const char *var,
                                           int timeState,
                                           const char *type,
                                           void *args,
                                           DestructorFunction &);
protected:
    void Initialize();
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

    int                      kmax;
    int                      lmax;
    bool                     meshDimensionsKnown;
    int                      unstructuredCellCount;
    std::string              rtVar;
    std::string              ztVar;
    bool                     initialized;
    int                     *cycles;
    int                      nCycles;
    double                  *times;
    int                      nTimes;
    std::vector<std::string> materialNames;
    bool                     assumeMixedMaterialsPresent;
    VariableDataMap          varStorage;
    std::vector<std::string> nodalVars;

    static const int         revolutionSteps;
};

#endif
