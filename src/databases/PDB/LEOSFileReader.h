#ifndef LEOS_FILE_FORMAT_H
#define LEOS_FILE_FORMAT_H
#include <PDBReader.h>
#include <string>
#include <vector>
#include <void_ref_ptr.h>

class vtkDataArray;
class vtkDataSet;
class avtDatabaseMetaData;

using std::string;
using std::map;

// ****************************************************************************
// Class: LEOSFileReader
//
// Purpose:
//   This class reads a LEOS PDB files.
//
// Notes:      
//
// Programmer: Mark C. Miller
// Creation:   February 10, 2004 
//
// Modifications:
//   Mark C. Miller, Thu Jun 17 23:07:34 PDT 2004
//   Added matNameMap data member, GetCurve
//   
//   Mark C. Miller, Thu Jul 21 12:52:42 PDT 2005
//   Added xdim/ydim data members to var info
// ****************************************************************************

class LEOSFileReader : public PDBReader
{
public:
    LEOSFileReader(const char *filename);
    LEOSFileReader(PDBFileObject *p);
    virtual ~LEOSFileReader();

    // Methods that help implement the file format methods.
    void          PopulateDatabaseMetaData(avtDatabaseMetaData *);
    vtkDataSet   *GetMesh(int ts, const char *var);
    vtkDataArray *GetVar(int ts, const char *var);

protected:

    // struct to organize eos variable information 
    typedef struct _eosVarInfo {
        int ndims;
        int xdim;      // size in x (obtained directly from PDB variable)
        int ydim;      // size in y (obtained directly from PDB variable)

        string vName;  // name of PDB symbol containing variable data
        string vUnits; // units of variable data

        string xName;  // name of PDB symbol containing x-axis data
        string xUnits; // units of x-axis data
        string xSize;  // name of PDB symbol containing size in x

        string yName;  // name of PDB symbol containing y-axis data
        string yUnits; // units of y-axis data
        string ySize;  // name of PDB symbol containing size in y
    } eosVarInfo_t ;

    // useful method to populate eos variable info map
    static void   SetMapEntry(eosVarInfo_t &info, int ndims,
                      string vName, string vUnits,
                      string xName, string xUnits, string xSize,
                      string yName, string yUnits, string ySize);

    virtual bool  IdentifyFormat();

    void          BuildVarInfoMap(void);
    void          GetTopDirs(void);

    void          ThrowInvalidVariableException(bool ignorable,
                      const char *varClass, const char *assumedVal, 
                      const char *actualVal, const char *fullVarName);
    void          ValidateVariableInfoAssumptions(eosVarInfo_t &mapInfo,
                      eosVarInfo_t &fileInfo, const char *fullVarName);

    bool          ReadMaterialInfo(const char *matDirName, string &matName,
                      string &matForm);
    bool          ReadVariableInfo(const char *matDirName, const char *varDirName,
                      eosVarInfo_t &varInfo);
    bool          AddVariableAndMesh(avtDatabaseMetaData *md, const char *matDirName, 
                      const char *matName, const char *matForm, const char *varDirName);

    bool          ParseContentsAndPopulateMetaData(avtDatabaseMetaData *md,
                      const char *contents);
    void          ReadFileAndPopulateMetaData(avtDatabaseMetaData *md);

    vtkDataSet   *GetCurve(const string matDirName, const string varName,
                      const eosVarInfo_t& varInfo);

    // maps an LEOS mesh or variable name to its cooresponding PDB directory
    map<string, string> matDirMap;

    // maps LEOS material name to the named used in the plugin
    map<string, string> matNameMap;

    // stores meta-data knowledge of each equation-of-state (eos) variable
    map<string, eosVarInfo_t> eosVarInfoMap;

    // stores list of variables for which assumptions turned out to be wrong
    map<string, bool> invalidVarMap;

    // used to control how much PDB work we do during PopulateDatabaseMetaData 
    int           tryHardLevel;

    // returned results from a PD_ls at the top level 
    char        **topDirs;
    int           numTopDirs;

};

#endif
