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

// ************************************************************************* //
//                            avtCCMFileFormat.h                             //
// ************************************************************************* //

#ifndef AVT_CCM_FILE_FORMAT_H
#define AVT_CCM_FILE_FORMAT_H

#include <avtSTMDFileFormat.h>
#include <ccmio.h>
#include <vectortypes.h>
#include <map>

using std::string;
using std::vector;

class vtkCellArray;
class vtkIdList;
class vtkPoints;
class vtkUnstructuredGrid;

// ****************************************************************************
//  Class: avtCCMFileFormat
//
//  Purpose:
//      Reads in CCM files as a plugin to VisIt.
//
//  Programmer: Brad Whitlock, Thu Aug 2 15:11:25 PST 2007
//  Creation:   Thu Aug 2 15:01:17 PST 2007
//
//  Modifications:
//    Kathleen Bonnell, Thu Feb 28 16:42:02 PST 2008
//    Added methods CanCacheVariable, RegisterVariableList, GetCellMapData, and
//    vars activeVisItVar, varsOnSubmesh, and ccmProblem.
//
//    Dave Bremer, Fri Apr  4 16:29:49 PDT 2008
//    Added a class for mapping IDs to indices.  If the IDs are sequential, 
//    just do a simple mapping, otherwise sort the values and use a binary 
//    search.
// ****************************************************************************

class avtCCMFileFormat : public avtSTMDFileFormat
{
public:
                       avtCCMFileFormat(const char *);
    virtual           ~avtCCMFileFormat();

    //
    // This is used to return unconvention data -- ranging from material
    // information to information about block connectivity.
    //
    // virtual void      *GetAuxiliaryData(const char *var, int domain,
    //                                     const char *type, void *args, 
    //                                     DestructorFunction &);
    //

    //
    // If you know the cycle number, overload this function.
    // Otherwise, VisIt will make up a reasonable one for you.
    //
    // virtual int         GetCyle(void);
    //

    virtual const char    *GetType(void)   { return "CCM"; };
    virtual void           FreeUpResources(void); 

    virtual vtkDataSet    *GetMesh(int, const char *);
    virtual vtkDataArray  *GetVar(int, const char *);
    virtual vtkDataArray  *GetVectorVar(int, const char *);
    virtual bool           CanCacheVariable(const char *);
    virtual void           RegisterVariableList(const char *,
                                                const vector<CharStrRef> &);


protected:
    class FaceInfo
    {
    public:
        FaceInfo();
        FaceInfo(const FaceInfo &);
        virtual ~FaceInfo();
        void operator = (const FaceInfo &);
        //int id;
        int cells[2];
        intVector nodes;
    };
    typedef std::vector<FaceInfo> FaceInfoVector;
    class CellInfo
    {
    public:
        CellInfo();
        CellInfo(const CellInfo &);
        virtual ~CellInfo();
        void operator = (const CellInfo &);
        void CellCenter(double *, vtkPoints *) const;
        void UseNodes(bool *) const;

        int id;
        // Important for knowing how face nodes are ordered. 
        // 0 = boundary
        // 1 = internal, first cell
        // 2 = internal, second cell 
        intVector faceTypes; 
        FaceInfoVector faces; 
    };

    class IDMap
    {
    public:
        IDMap();

        void SetIDs(const intVector &v);
        int  IDtoIndex(int id) const;

        static int compare(const void *, const void *);

        intVector  ids;
        bool  bSequential;
        bool  bReverseMap;
        int   iFirstElem;
        int   numIDs;
    };


    typedef std::vector<CellInfo> CellInfoVector;
    typedef std::map<std::string, CCMIOID> VarFieldMap;
    typedef std::vector<vtkDataArray*> DataArrayVector;

    CCMIOID          &GetRoot();
    CCMIOID          &GetState();
    bool              GetIDsForDomain(int dom, 
                                      CCMIOID &processor,
                                      CCMIOID &vertices,
                                      CCMIOID &topology,
                                      CCMIOID &solution,
                                      bool &hasSolution);
    void              GetFaces(CCMIOID faceID, CCMIOEntity faceType,
                               unsigned int nFaces, 
                               const IDMap &cellIDMap, const IDMap &vertexIDMap, 
                               int &minSize, int &maxSize, CellInfoVector &ci);
    void              ReadScalar(CCMIOID field, intVector &mapData, 
                                 floatVector &data, bool readingVector = false);
    void              BuildHex(const CellInfo &ci, 
                               vtkCellArray *cellArray, 
                               intVector &cellTypes);
    void              BuildTet(const CellInfo &ci, 
                               vtkCellArray *cellArray, 
                               intVector &cellTypes);
    void              BuildWedge(const CellInfo &ci, 
                                 vtkCellArray *cellArray, 
                                 intVector &cellTypes);
    void              BuildPyramid(const CellInfo &ci, 
                                   vtkCellArray *cellArray, 
                                   intVector &cellTypes);
    void              TesselateCell(const int, const CellInfoVector &civ, 
                                    vtkPoints *points, vtkUnstructuredGrid *ugrid);
    void              TesselateCells2D(const int, const CellInfoVector &civ, 
                                       vtkPoints *points, vtkUnstructuredGrid *ugrid);
    void              GetCellMapData(const int, const string &, intVector &);


    void              ReadCellInfo(int dom, const char *meshname,
                                   CellInfoVector &cellInfo, 
                                   int &minFaceSize, int &maxFaceSize);
    vtkPoints        *ReadPoints(int dom, const char *meshname);
    void              SelectCellsForThisProcessor(CellInfoVector &cellInfo, vtkPoints *);

    bool              subdividingSingleMesh;

    bool              ccmOpened;
    bool              ccmStateFound;
    CCMIOID           ccmRoot;
    CCMIOID           ccmState;
    CCMIOID           ccmProblem;
    CCMIOError        ccmErr;
    VarFieldMap       varsToFields;
    stringVector      varsOnSubmesh;
    string            activeVisItVar;

    DataArrayVector   originalCells; 

    virtual void      PopulateDatabaseMetaData(avtDatabaseMetaData *);
};


#endif
