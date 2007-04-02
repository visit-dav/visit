/*****************************************************************************
*
* Copyright (c) 2000 - 2006, The Regents of the University of California
* Produced at the Lawrence Livermore National Laboratory
* All rights reserved.
*
* This file is part of VisIt. For details, see http://www.llnl.gov/visit/. The
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
*    documentation and/or materials provided with the distribution.
*  - Neither the name of the UC/LLNL nor  the names of its contributors may be
*    used to  endorse or  promote products derived from  this software without
*    specific prior written permission.
*
* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT  HOLDERS AND CONTRIBUTORS "AS IS"
* AND ANY EXPRESS OR  IMPLIED WARRANTIES, INCLUDING,  BUT NOT  LIMITED TO, THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND  FITNESS FOR A PARTICULAR  PURPOSE
* ARE  DISCLAIMED.  IN  NO  EVENT  SHALL  THE  REGENTS  OF  THE  UNIVERSITY OF
* CALIFORNIA, THE U.S.  DEPARTMENT  OF  ENERGY OR CONTRIBUTORS BE  LIABLE  FOR
* ANY  DIRECT,  INDIRECT,  INCIDENTAL,  SPECIAL,  EXEMPLARY,  OR CONSEQUENTIAL
* DAMAGES (INCLUDING, BUT NOT  LIMITED TO, PROCUREMENT OF  SUBSTITUTE GOODS OR
* SERVICES; LOSS OF  USE, DATA, OR PROFITS; OR  BUSINESS INTERRUPTION) HOWEVER
* CAUSED  AND  ON  ANY  THEORY  OF  LIABILITY,  WHETHER  IN  CONTRACT,  STRICT
* LIABILITY, OR TORT  (INCLUDING NEGLIGENCE OR OTHERWISE)  ARISING IN ANY  WAY
* OUT OF THE  USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH
* DAMAGE.
*
*****************************************************************************/

// ************************************************************************* //
//                       avtStructuredDomainBoundaries.h                     //
// ************************************************************************* //

#ifndef AVT_STRUCTURED_DOMAIN_BOUNDARIES_H
#define AVT_STRUCTURED_DOMAIN_BOUNDARIES_H

#include <database_exports.h>

#include <avtDomainBoundaries.h>
#include <visitstream.h>

// Forward declaration.
class DATABASE_API avtStructuredDomainBoundaries;

//
//  Class:  Neighbor
//
struct Neighbor
{
    int domain;
    int match;
    int orient[3];
    int ndims[3];
    int zdims[3];
    int npts;
    int ncells;
    int nextents[6];
    int zextents[6];
    int type;
};

// ****************************************************************************
//
//  Class:  Boundary
//
//  Modifications:
//
//    Hank Childs, Tue Jul  5 14:05:09 PDT 2005
//    Removed method FindNeighborIndex which did not work if two domains
//    shared multiple boundaries.
//
// ****************************************************************************
struct Boundary
{
    enum BoundaryType
    {
        NONE = 0x00,
        IMIN = 0x01,
        IMAX = 0x02,
        JMIN = 0x04,
        JMAX = 0x08,
        KMIN = 0x10,
        KMAX = 0x20
    };

    int              domain;
    int              expand[6];
    vector<Neighbor> neighbors;
    // old extents
    int              oldnextents[6];
    int              oldzextents[6];
    int              oldndims[3];
    int              oldzdims[3];
    int              oldnpts;
    int              oldncells;
    // new extents
    int              newnextents[6];
    int              newzextents[6];
    int              newndims[3];
    int              newzdims[3];
    int              newnpts;
    int              newncells;
  public:
    // Creation methods
    void   SetExtents(int[6]);
    void   AddNeighbor(int,int,int[3],int[6]);
    void   DeleteNeighbor(int, vector<Boundary> &);
    void   Finish();
    // Utility methods
    bool   IsGhostZone(int,int);
    bool   IsGhostZone(int,int,int);
    int    OldPointIndex(int, int, int);
    int    OldCellIndex(int, int, int);
    int    NewPointIndex(int, int, int);
    int    NewCellIndex(int, int, int);
    int    ClosestExistingNewPointIndex(bool*, int, int, int);
    int    ClosestExistingNewCellIndex(bool*, int, int, int);
    int    TranslatedPointIndex(Neighbor*,Neighbor*,int,int,int);
    int    TranslatedCellIndex(Neighbor*,Neighbor*,int,int,int);
    int    NewPointIndexFromNeighbor(Neighbor*, int, int, int);
    int    NewCellIndexFromNeighbor(Neighbor*, int, int, int);
};

// ****************************************************************************
//  Class: BoundaryHelperFunctions
//
//  Modifications:
//
//    Hank Childs, Mon Nov 10 15:26:00 PST 2003
//    Added routines for exchanging rectilinear components.
//
// ****************************************************************************
template<class T>
class BoundaryHelperFunctions
{
  private:
    avtStructuredDomainBoundaries *sdb;
  public:
    BoundaryHelperFunctions(avtStructuredDomainBoundaries *sdb_) : sdb(sdb_) { }

    T   ***InitializeBoundaryData();
    void   FillBoundaryData(int, const T*, T***, bool, int=1);
    void   FillMixedBoundaryData(int,avtMaterial*,const T*,T***,int***,int***,vector<int>&);
    void   FillRectilinearBoundaryData(int, const T*, const T*, const T*, T***);
    void   CommunicateBoundaryData(const vector<int>&, T***, bool, int=1);
    void   CommunicateMixedBoundaryData(const vector<int>&,T***,int***,int***,vector< vector<int> > &);
    void   CopyOldValues(int, const T*, T*, bool, int=1);
    void   CopyOldMixedValues(avtMaterial*,const T*, T*);
    void   CopyOldRectilinearValues(int, const T*, T*, int);
    void   SetNewBoundaryData(int, T***, T*, bool, int=1);
    void   SetNewMixedBoundaryData(int,avtMaterial*,const vector< vector<int> >&,int***,T***,int***,int***,int*,T*,int*,int*,int*,int&);
    void   SetNewRectilinearBoundaryData(int, T***, T*, T*, T*);
    void   FakeNonexistentBoundaryData(int, T*, bool, int=1);
    void   FreeBoundaryData(T***);
};

// ****************************************************************************
//  Class:  avtStructuredDomainBoundaries
//
//  Purpose:
//    Encapsulate domain boundary information for structured meshes.
//
//  Programmer:  Jeremy Meredith
//  Creation:    October 25, 2001
//
//  Modifications:
//    Jeremy Meredith, Wed Nov 21 14:12:18 PST 2001
//    Filled in most of the class.
//
//    Jeremy Meredith, Thu Dec 13 11:47:42 PST 2001
//    Added support for materials and mixed vars.
//    Templatized most of the helper functions so they support int's as well.
//
//    Hank Childs, Wed Mar 27 08:26:58 PST 2002
//    Added ConfirmMeshes.
//
//    Kathleen Bonnell, Wed Mar 27 15:47:14 PST 2002 
//    vtkScalars and vtkVectors have been deprecated in VTK 4.0, 
//    use vtkDataArray instead.
//
//    Jeremy Meredith, Brad Whitlock, Thu Apr 4 09:55:34 PDT 2002
//    Refactored so it uses instances of template classes instead of
//    relying on template function specialization. This works around a bug
//    in Microsoft's compiler on windows.
//
//    Kathleen Bonnell, Mon May 20 13:40:17 PDT 2002  
//    Made ExhangeVector into two methods to handle different underlying
//    data types (int, float).  
//    
//    Jeremy Meredith, Fri Nov  7 15:15:34 PST 2003
//    Added private methods to handle varying types of scalar exchanges.
//
//    Hank Childs, Mon Nov 10 14:57:10 PST 2003
//    Previously, StructuredDomainBoundaries was a concrete type.  I made it be
//    abstract and added derived types for curvilinear and rectilinear grids
//    (the only previous support was for curvilinear grids).  Also added
//    DeclareNumDomains and CreateGhostZones.
//
//    Mark C. Miller, Mon Jan 12 17:29:19 PST 2004
//    Re-structured to permit ANY structured mesh to compute neighbors from
//    knowledge of extents when that is possible. Used a boolean flag to
//    indicate this condition rather than sub-classing because the class
//    hierarchy is more involved than was worth it.
//
//    Hank Childs, Sat Aug 14 06:41:00 PDT 2004
//    Added CreateGhostNodes.
//
//    Kathleen Bonnell, Tue Feb  8 15:32:22 PST 2005 
//    Added GetExtents. 
//
//    Hank Childs, Sun Feb 27 12:14:32 PST 2005
//    Added RequiresCommunication.  Added new argument to CreateGhostNodes.
//
//    Hank Childs, Wed Jun 29 15:24:35 PDT 2005
//    Added ResetCachedMembers, as well as domain2proc member.
//
// ****************************************************************************

class DATABASE_API avtStructuredDomainBoundaries :  public avtDomainBoundaries
{
  public:
             avtStructuredDomainBoundaries(bool _canComputeNeighborsFromExtents = false);
    virtual ~avtStructuredDomainBoundaries();

    static void Destruct(void *);

    void     SetNumDomains(int nd);
    void     SetExtents(int domain, int e[6]);
    void     AddNeighbor(int domain, int d,int mi, int o[3], int e[6]);
    void     Finish(int domain);

    void     GetExtents(int domain, int e[6]);

    //  methods for cases where neighbors can be computed
    void  SetIndicesForRectGrid(int domain, int e[6]);
    void  SetIndicesForAMRPatch(int domain, int level, int e[6]);
    void  CalculateBoundaries(void);

    virtual vector<vtkDataArray*>     ExchangeScalar(vector<int>   domainNum,
                                             bool                  isPointData,
                                             vector<vtkDataArray*> scalars);

    virtual vector<vtkDataArray*>     ExchangeFloatVector(vector<int> domainNum,
                                            bool                   isPointData,
                                            vector<vtkDataArray*>  vectors);

    virtual vector<vtkDataArray*>     ExchangeIntVector(vector<int>  domainNum,
                                             bool                  isPointData,
                                             vector<vtkDataArray*> vectors);

    virtual vector<avtMaterial*>      ExchangeMaterial(vector<int>   domainNum,
                                              vector<avtMaterial*>   mats);

    virtual vector<avtMixedVariable*> ExchangeMixVar(vector<int>     domainNum,
                                        const vector<avtMaterial*>   mats,
                                        vector<avtMixedVariable*>    mixvars);

    virtual void                      CreateGhostNodes(vector<int>   domainNum,
                                                    vector<vtkDataSet*> meshes,
                                                    vector<int> &);

    virtual bool                      RequiresCommunication(avtGhostDataType);
    virtual bool                      ConfirmMesh(vector<int>      domainNum,
                                               vector<vtkDataSet*> meshes);
    virtual void                      ResetCachedMembers();

  private:
    virtual vector<vtkDataArray*>     ExchangeFloatScalar(vector<int> domainNum,
                                             bool                  isPointData,
                                             vector<vtkDataArray*> scalars);

    virtual vector<vtkDataArray*>     ExchangeIntScalar(vector<int>  domainNum,
                                             bool                  isPointData,
                                             vector<vtkDataArray*> scalars);

    virtual vector<vtkDataArray*>     ExchangeUCharScalar(vector<int> domainNum,
                                             bool                  isPointData,
                                             vector<vtkDataArray*> scalars);


  protected:
    // data
    vector<Boundary> wholeBoundary;
    vector<Boundary> boundary;

    // data for cases where neighbors can be computed
    bool shouldComputeNeighborsFromExtents;
    vector<int>   extents;
    vector<int>   levels;
    vector<int>   domain2proc;

    friend class BoundaryHelperFunctions<int>;
    friend class BoundaryHelperFunctions<float>;
    friend class BoundaryHelperFunctions<unsigned char>;
    BoundaryHelperFunctions<int>           *bhf_int;
    BoundaryHelperFunctions<float>         *bhf_float;
    BoundaryHelperFunctions<unsigned char> *bhf_uchar;

    // helper methods
    vector<int> CreateDomainToProcessorMap(const vector<int>&);
    void        CreateCurrentDomainBoundaryInformation(const vector<int>&);
    bool       *SetExistence(int, bool);
    void        CreateGhostZones(vtkDataSet *, vtkDataSet *, Boundary *);

    friend ostream &operator<<(ostream&, Boundary&);
};


// the communication methods are optimized for each class of mesh
class DATABASE_API avtCurvilinearDomainBoundaries
    : public avtStructuredDomainBoundaries
{
  public:
    avtCurvilinearDomainBoundaries(bool _canComputeNeighborsFromExtents = false) :
        avtStructuredDomainBoundaries(_canComputeNeighborsFromExtents) {;};
   
    virtual vector<vtkDataSet*>    ExchangeMesh(vector<int>         domainNum,
                                                vector<vtkDataSet*> meshes);
};


// the communication methods are optimized for each class of mesh
class DATABASE_API avtRectilinearDomainBoundaries
    : public avtStructuredDomainBoundaries
{
  public:
    avtRectilinearDomainBoundaries(bool _canComputeNeighborsFromExtents = false) :
        avtStructuredDomainBoundaries(_canComputeNeighborsFromExtents) {;};

    virtual vector<vtkDataSet*>    ExchangeMesh(vector<int>         domainNum,
                                                vector<vtkDataSet*> meshes);
};


#endif
