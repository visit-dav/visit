#ifndef PDB_READER_H
#define PDB_READER_H
#include <pdb.h>
#include <vector>
#include <map>
#include <string>

// Forward declarations
class avtDatabaseMetaData;
class vtkDataArray;
class vtkDataSet;

//
// Enum for array type definitions.
//
typedef enum {NO_TYPE, CHAR_TYPE, INTEGER_TYPE, FLOAT_TYPE, DOUBLE_TYPE,
              LONG_TYPE, CHARARRAY_TYPE, INTEGERARRAY_TYPE, FLOATARRAY_TYPE,
              DOUBLEARRAY_TYPE, LONGARRAY_TYPE} TypeEnum;

// ****************************************************************************
// Class: PDBReader
//
// Purpose:
//   Abstract base class for PDB file readers.
//
// Notes:      
//
// Programmer: Brad Whitlock
// Creation:   Thu Oct 10 08:55:05 PDT 2002
//
// Modifications:
//   Brad Whitlock, Tue Apr 29 13:22:55 PST 2003
//   I added time arguments to GetMesh, GetVar, and GetVectorVar. I also
//   added the GetCycles method. I also added GetDoubleArray, GetIntegerArray,
//   and GetString.
//
// ****************************************************************************

class PDBReader
{
public:
    PDBReader(PDBfile *pdb);
    virtual ~PDBReader();

    virtual bool Identify() = 0;
    virtual void GetTimeVaryingInformation(int ts, avtDatabaseMetaData *) = 0;
    virtual void PopulateDatabaseMetaData(avtDatabaseMetaData *) = 0;

    virtual void          GetCycles(std::vector<int> &cycles);
    virtual int           GetNTimesteps();

    virtual vtkDataSet   *GetMesh(int, const char *) = 0;
    virtual vtkDataArray *GetVar(int, const char *) = 0;
    virtual vtkDataArray *GetVectorVar(int, const char *) = 0;

    void *ReadValues(const char *, TypeEnum *, int *, int **, int*);

protected:
    class VariableData
    {
    public:
        VariableData(const std::string &name);
        ~VariableData();
        bool ReadValues(PDBReader *reader);

        std::string  varName;
        void        *data;
        TypeEnum     dataType;
        int         *dims;
        int          nDims;
        int          nTotalElements;
    };

    typedef std::map<std::string, VariableData *> VariableDataMap;

    bool GetDouble(const char *name, double *val);
    bool GetDoubleArray(const char *name, double **val, int *nvals);
    bool GetInteger(const char *name, int *val);
    bool GetIntegerArray(const char *name, int **val, int *nvals);
    bool GetString(const char *name, char **str);
    bool SymbolExists(const char *name);
protected:
    PDBfile *pdb;
};


//
// Functions to free memory.
//

template <class T>
void free_mem(T *);

void free_void_mem(void *);

#endif
