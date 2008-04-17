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

// ************************************************************************* //
//                            avtCGNSFileFormat.h                            //
// ************************************************************************* //

#ifndef AVT_CGNS_FILE_FORMAT_H
#define AVT_CGNS_FILE_FORMAT_H

#include <avtMTMDFileFormat.h>

#include <vector>
#include <vectortypes.h>
#include <map>

// ****************************************************************************
//  Class: avtCGNSFileFormat
//
//  Purpose:
//      Reads in CGNS files as a plugin to VisIt.
//
//  Programmer: Brad Whitlock
//  Creation:   Tue Aug 30 16:08:44 PST 2005
//
//  Modifications:
//    Brad Whitlock, Tue Apr 15 10:20:10 PDT 2008
//    Added methods that help us inspect the file contents.
//
// ****************************************************************************

class avtCGNSFileFormat : public avtMTMDFileFormat
{
public:
                       avtCGNSFileFormat(const char *);
    virtual           ~avtCGNSFileFormat();

    //
    // This is used to return unconvention data -- ranging from material
    // information to information about block connectivity.
    //
    // virtual void      *GetAuxiliaryData(const char *var, const char *type,
    //                                     int timestep, int domain,void *args, 
    //                                     DestructorFunction &);
    //

    //
    // If you know the times and cycle numbers, overload this function.
    // Otherwise, VisIt will make up some reasonable ones for you.
    //
    virtual void           GetCycles(std::vector<int> &);
    virtual void           GetTimes(std::vector<double> &);
    
    virtual int            GetNTimesteps(void);

    virtual const char    *GetType(void)   { return "CGNS"; };
    virtual void           FreeUpResources(void); 

    virtual vtkDataSet    *GetMesh(int, int, const char *);
    virtual vtkDataArray  *GetVar(int, int, const char *);
    virtual vtkDataArray  *GetVectorVar(int, int, const char *);

protected:
    struct VarInfo
    {
        intVector   zoneList; // The zones that contain the variable
        int         cellCentering;
        int         nodeCentering;
        int         badCentering;
        bool        hasUnits;
        std::string units;
    };
    typedef std::map<std::string, VarInfo> StringVarInfoMap;

    struct BaseInformation
    {
        std::string      name;
        int              cellDim;
        int              physicalDim;
        int              meshType;
        stringVector     zoneNames;
        StringVarInfoMap vars;
    };
    typedef std::vector<BaseInformation> BaseInformationVector;

    struct BaseAndZoneList
    {
        int       base;
        intVector zones;
    };

    int                    GetFileHandle();
    void                   ReadTimes();
    bool                   GetCoords(int base, int zone, const int *zsize,
                                     bool structured, float **coords,
                                     int *ncoords);
    void                   AddReferenceStateExpressions(avtDatabaseMetaData *md,
                                     int base, int nBases, const std::string &baseName,
                                     const std::string &meshName);
    void                   AddVectorExpressions(avtDatabaseMetaData *md, 
                               bool *haveVelocity, bool *haveMomentum, int nBases,
                               const std::string &baseName);
    void                   AddVectorExpression(avtDatabaseMetaData *md, 
                               bool *haveComponent, int nBases, 
                               const std::string &baseName, 
                               const std::string &vecName);
    bool                   GetVariablesForBase(int base, BaseInformation &baseInfo);
    bool                   BaseContainsUnits(int base);

    vtkDataSet *           GetCurvilinearMesh(int, int, const char *,
                                              const int *);
    vtkDataSet *           GetUnstructuredMesh(int, int, const char *,
                                               const int *);

    void PrintVarInfo(ostream &out, const VarInfo &var, const char *indent);
    void PrintStringVarInfoMap(ostream &out, const StringVarInfoMap &vars, const char *indent);
    void PrintBaseInformation(ostream &out, const BaseInformation &baseInfo);

    char                                  *cgnsFileName;
    int                                    fn;
    bool                                   timesRead;
    bool                                   cgnsCyclesAccurate;
    bool                                   cgnsTimesAccurate;
    doubleVector                           times;
    intVector                              cycles;
    std::map<std::string, BaseAndZoneList> MeshDomainMapping;
    std::map<std::string, int>             BaseNameToIndices;
    std::map<std::string, std::string>     VisItNameToCGNSName;

    virtual void           PopulateDatabaseMetaData(avtDatabaseMetaData *, int);
};


#endif

