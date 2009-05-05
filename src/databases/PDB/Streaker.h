/*****************************************************************************
*
* Copyright (c) 2000 - 2008, Lawrence Livermore National Security, LLC
* Produced at the Lawrence Livermore National Laboratory
* LLNL-CODE-400142
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
#ifndef STREAKER_H
#define STREAKER_H
#include <map>
#include <vectortypes.h>
#include <void_ref_ptr.h>

class avtDatabaseMetaData;
class PDBFileObject;
class vtkDataSet;
class vtkDataArray;
class vtkFloatArray;

// ****************************************************************************
// Class: Streaker
//
// Purpose:
//   This class provides support for Streak plots coming from PDB files. A
//   file is read that contains information needed to construct streak datasets.
//   If all of the needed information is available, this class provides methods
//   to add streak variables to metadata and to construct and return the 
//   streak datasets using data from a vector of PDB files.
//
// Notes:      
//
// Programmer: Brad Whitlock
// Creation:   Fri Nov  7 10:42:02 PST 2008
//
// Modifications:
//   Brad Whitlock, Tue Dec  2 15:53:13 PST 2008
//   Added GetAuxiliaryData so we can return materials.
//
//   Brad Whitlock, Wed Feb 25 16:07:32 PST 2009
//   I added x_scale and x_translate to the StreakInfo. I also added support
//   for different kinds of log scaling.
//
//   Brad Whitlock, Tue May  5 16:22:56 PDT 2009
//   I added support for setting the streak plot centering.
//
// ****************************************************************************

class Streaker
{
public:
    Streaker();
    ~Streaker();

    typedef std::vector<PDBFileObject *> PDBFileObjectVector;

    void ReadStreakFile(const std::string &, PDBFileObject *pdb);
    void PopulateDatabaseMetaData(avtDatabaseMetaData *md);

    vtkDataSet   *GetMesh(const std::string &mesh, const PDBFileObjectVector &pdb);
    vtkDataArray *GetVar(const std::string &mesh, const PDBFileObjectVector &pdb);
    void         *GetAuxiliaryData(const std::string &var, 
                                   const char *type, void *args, DestructorFunction &df, 
                                   const PDBFileObjectVector &pdb);
    void FreeUpResources();
private:
    typedef enum {LOGTYPE_NONE, LOGTYPE_LOG, LOGTYPE_LOG10} logtype;

    struct StreakInfo
    {
        StreakInfo();

        std::string xvar;
        std::string yvar;
        std::string zvar;

        bool        hasMaterial;
        bool        cellCentered;
        int         slice;
        int         sliceIndex;
        int         hsize;
        bool        integrate;
        logtype     log;
        float       x_scale;
        float       x_translate;
        float       y_scale;
        float       y_translate;

        vtkDataSet *dataset;
    };

    void AddStreak(const std::string &varname, StreakInfo &s, PDBFileObject *pdb);
    bool FindMaterial(PDBFileObject *pdb, int *zDimensions, int zDims);

#ifndef MDSERVER
    vtkDataSet *ConstructDataset(const std::string &, const StreakInfo &,
                                 const PDBFileObjectVector &pdb);
    vtkFloatArray *AssembleData(const std::string &var, int *sdims, int slice, 
                                int sliceIndex, const PDBFileObjectVector &pdb) const;
#endif

    std::map<std::string, StreakInfo> streaks;
    std::string                       matvar;
    intVector                         matnos;
    stringVector                      matNames;
    std::map<std::string,std::string> matToStreak;
};

#endif
