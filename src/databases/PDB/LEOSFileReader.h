// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#ifndef LEOS_FILE_FORMAT_H
#define LEOS_FILE_FORMAT_H
#include <PDBReader.h>
#include <map>
#include <string>
#include <void_ref_ptr.h>

class vtkDataArray;
class vtkDataSet;
class avtDatabaseMetaData;
class DBOptionsAttributes;

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
    LEOSFileReader(const char *filename, const DBOptionsAttributes *rdopts);
    LEOSFileReader(PDBFileObject *p, const DBOptionsAttributes *rdopts);
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

        std::string vName;  // name of PDB symbol containing variable data
        std::string vUnits; // units of variable data

        std::string xName;  // name of PDB symbol containing x-axis data
        std::string xUnits; // units of x-axis data
        std::string xSize;  // name of PDB symbol containing size in x

        std::string yName;  // name of PDB symbol containing y-axis data
        std::string yUnits; // units of y-axis data
        std::string ySize;  // name of PDB symbol containing size in y
    } eosVarInfo_t ;

    // useful method to populate eos variable info map
    static void   SetMapEntry(eosVarInfo_t &info, int ndims,
                      std::string vName, std::string vUnits,
                      std::string xName, std::string xUnits, std::string xSize,
                      std::string yName, std::string yUnits, std::string ySize);

    virtual bool  IdentifyFormat();

    void          ProcessReadOptions(const DBOptionsAttributes *rdopts);
    void          BuildVarInfoMap(void);
    void          GetTopDirs(void);

    void          ThrowInvalidVariableException(bool ignorable,
                      const char *varClass, const char *assumedVal, 
                      const char *actualVal, const char *fullVarName);
    void          ValidateVariableInfoAssumptions(eosVarInfo_t &mapInfo,
                      eosVarInfo_t &fileInfo, const char *fullVarName);

    bool          ReadMaterialInfo(const char *matDirName, std::string &matName,
                      std::string &matForm);
    bool          ReadVariableInfo(const char *matDirName, const char *varDirName,
                      eosVarInfo_t &varInfo);
    bool          AddVariableAndMesh(avtDatabaseMetaData *md, const char *matDirName, 
                      const char *matName, const char *matForm, const char *varDirName);

    bool          ParseContentsAndPopulateMetaData(avtDatabaseMetaData *md,
                      const char *contents);
    void          ReadFileAndPopulateMetaData(avtDatabaseMetaData *md);

    vtkDataSet   *GetCurve(const std::string matDirName, const std::string varName,
                      const eosVarInfo_t& varInfo);

    // maps an LEOS mesh or variable name to its cooresponding PDB directory
    std::map<std::string, std::string> matDirMap;

    // maps LEOS material name to the named used in the plugin
    std::map<std::string, std::string> matNameMap;

    // stores meta-data knowledge of each equation-of-state (eos) variable
    std::map<std::string, eosVarInfo_t> eosVarInfoMap;

    // stores list of variables for which assumptions turned out to be wrong
    std::map<std::string, bool> invalidVarMap;

    // Stores material name to Periodic table formula map
    std::map<std::string, std::string> matFormulaMap;

    // used to control how much PDB work we do during PopulateDatabaseMetaData 
    int           tryHardLevel;

    // returned results from a PD_ls at the top level 
    char        **topDirs;
    int           numTopDirs;

};

#endif
