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

#ifndef PP_ZFILE_FORMAT_H
#define PP_ZFILE_FORMAT_H
#include <PDBReader.h>
#include <string>
#include <vector>
#include <void_ref_ptr.h>
#include <vectortypes.h>

class vtkDataArray;
class vtkDataSet;
class vtkMatrix4x4;
class avtDatabaseMetaData;
class avtVariableCache;

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
//   Brad Whitlock, Fri Sep 26 10:22:05 PDT 2003
//   I added support for ray meshes.
//
//   Brad Whitlock, Wed Sep 1 23:53:01 PST 2004
//   I added a FreeUpResources method to clear out the stored data in the
//   variable cache and close the PDB file object.
//
//   Brad Whitlock, Mon Dec 6 16:48:18 PST 2004
//   Added a method to set the a variable cache pointer so the object has
//   access to the file format's variable cache object for storing auxiliary
//   data.
//
//   Hank Childs, Fri Nov 18 09:08:48 PST 2005
//   Add new arguments to revolve data sets.
//
//   Brad Whitlock, Thu Dec  4 11:33:09 PST 2008
//   I exposed ReadMaterialNames as a static, public method.
//
// ****************************************************************************

class PP_ZFileReader : public PDBReader
{
public:
    PP_ZFileReader(const char *filename);
    PP_ZFileReader(PDBFileObject *p);
    virtual ~PP_ZFileReader();

    // Methods that help implement the file format methods.
    void          PopulateDatabaseMetaData(int ts, avtDatabaseMetaData *);
    void          GetTimeVaryingInformation(avtDatabaseMetaData *);
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

    void          FreeUpResources();
    void          SetCache(avtVariableCache *);

    static bool ReadMaterialNames(PDBFileObject *, int nmats, 
                                  stringVector &matNames);
protected:
    virtual bool IdentifyFormat();

    // Internal helper functions.
    void Initialize();
    void InitializeVarStorage();
    void ReadVariable(const std::string &var);
    void CreateGhostZones(const int *, vtkDataSet *);
    bool VariableIsNodal(const std::string &var) const;
    void ReadMixvarAndCache(const std::string &var,
                            const std::string &realVar, int state);
    const int *GetIreg(int state);
    int  GetUnstructuredCellCount();
    bool PopulateMaterialNames();
    static bool ReadMaterialNamesHelper(PDBFileObject *, const char *, int , 
                                        stringVector &);
    void AddRayMetaData(avtDatabaseMetaData *);
    vtkDataSet *GetRayMesh(int state, const char *var);
    vtkDataSet *ConstructRayMesh(int state, bool is3d);
    vtkDataArray *GetRayVar(int state, const std::string &varStr);

    static vtkDataSet *RevolveDataSet(vtkDataSet *in_ds, const double *axis,
                                      double start_angle, double stop_angle,
                                      int nsteps, vtkMatrix4x4 **, 
                                      int *, bool extrude);

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
    avtVariableCache        *cache;

    static const int         revolutionSteps;
};

#endif
