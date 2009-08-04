/*****************************************************************************
*
* Copyright (c) 2000 - 2009, Lawrence Livermore National Security, LLC
* Produced at the Lawrence Livermore National Laboratory
* LLNL-CODE-400124
* All rights reserved.
*
* This file is  part of VisIt. For  details, see https://visit.llnl.gov/.  The
* full copyright notice is contained in the file COPYRIGHT located at the root
* of the VisIt distribution or at http://www.llnl.gov/visit/copyright.html.
*
* Redistribution  and  use  in  source  and  binary  forms,  with  or  without
* modification, are permitted provided that the following conditions are met:
*
*  - Redistributions of  source code must  retain the above  copyright notice,
*    this list of conditions and the disclaimer below.
*  - Redistributions in binary form must reproduce the above copyright notice,
*    this  list of  conditions  and  the  disclaimer (as noted below)  in  the
*    documentation and/or other materials provided with the distribution.
*  - Neither the name of  the LLNS/LLNL nor the names of  its contributors may
*    be used to endorse or promote products derived from this software without
*    specific prior written permission.
*
* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT  HOLDERS AND CONTRIBUTORS "AS IS"
* AND ANY EXPRESS OR  IMPLIED WARRANTIES, INCLUDING,  BUT NOT  LIMITED TO, THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND  FITNESS FOR A PARTICULAR  PURPOSE
* ARE  DISCLAIMED. IN  NO EVENT  SHALL LAWRENCE  LIVERMORE NATIONAL  SECURITY,
* LLC, THE  U.S.  DEPARTMENT OF  ENERGY  OR  CONTRIBUTORS BE  LIABLE  FOR  ANY
* DIRECT,  INDIRECT,   INCIDENTAL,   SPECIAL,   EXEMPLARY,  OR   CONSEQUENTIAL
* DAMAGES (INCLUDING, BUT NOT  LIMITED TO, PROCUREMENT OF  SUBSTITUTE GOODS OR
* SERVICES; LOSS OF  USE, DATA, OR PROFITS; OR  BUSINESS INTERRUPTION) HOWEVER
* CAUSED  AND  ON  ANY  THEORY  OF  LIABILITY,  WHETHER  IN  CONTRACT,  STRICT
* LIABILITY, OR TORT  (INCLUDING NEGLIGENCE OR OTHERWISE)  ARISING IN ANY  WAY
* OUT OF THE  USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH
* DAMAGE.
*
*****************************************************************************/

#ifndef LEOS_FILE_FORMAT_H
#define LEOS_FILE_FORMAT_H
#include <PDBReader.h>
#include <string>
#include <vector>
#include <void_ref_ptr.h>

class vtkDataArray;
class vtkDataSet;
class avtDatabaseMetaData;
class DBOptionsAttributes;

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

    void          ProcessReadOptions(const DBOptionsAttributes *rdopts);
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
