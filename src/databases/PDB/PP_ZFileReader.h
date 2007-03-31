#ifndef PP_ZFILE_READER_H
#define PP_ZFILE_READER_H
#include <PDBReader.h>

class vtkDataSet;

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
//
// ****************************************************************************

class PP_ZFileReader : public PDBReader
{
public:
    PP_ZFileReader(PDBfile *pdb);
    virtual ~PP_ZFileReader();

    virtual bool Identify();
    virtual void GetTimeVaryingInformation(int, avtDatabaseMetaData *);
    virtual void PopulateDatabaseMetaData(avtDatabaseMetaData *);

    virtual void          GetCycles(std::vector<int> &cycles);
    virtual int           GetNTimesteps();
    virtual vtkDataSet   *GetMesh(int, const char *);
    virtual vtkDataArray *GetVar(int, const char *);
    virtual vtkDataArray *GetVectorVar(int, const char *);

protected:
    void Initialize();
    void ReadVariable(const std::string &);
    void CreateGhostZones(const int *, vtkDataSet *);

    int              kmax;
    int              lmax;
    bool             initialized;
    int             *cycles;
    int              nCycles;
    double          *times;
    int              nTimes;
    VariableDataMap  varStorage;
};

#endif
