// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                       avtStructuredDomainBoundaries.h                     //
// ************************************************************************* //

#ifndef AVT_STRUCTURED_DOMAIN_BOUNDARIES_H
#define AVT_STRUCTURED_DOMAIN_BOUNDARIES_H

#include <database_exports.h>

#include <avtDomainBoundaries.h>
#include <visitstream.h>

#include <vector>

// Forward declaration.
class avtStructuredDomainBoundaries;

typedef enum
{
    SAME_REFINEMENT_LEVEL = 0,
    MORE_FINE,
    MORE_COARSE
} RefinementRelationship;

typedef enum
{
    SYMMETRIC_NEIGHBOR = 0, // Normal case ... we give ghost data to each other
    DONOR_NEIGHBOR,
    RECIPIENT_NEIGHBOR
} NeighborRelationship;

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

    // A neighbor associates some implied domain with the domain stored in the data
    // member this->domain.  ("Implied" meaning the domain in Boundary->domain
    // which in turn has a vector of Neighbors.)
    // A neighbor relationship of DONOR means that this->domain
    // is a donor to the implied domain.  
    // A refinement relationship of MORE_FINE means that this->domain is more fine
    // than the implied domain.
    RefinementRelationship refinement_rel;
    std::vector<int>       refinement_ratio;
    NeighborRelationship   neighbor_rel;
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
//    Hank Childs, Tue Jan  4 11:33:47 PST 2011
//    Add support for ghost data across refinement levels.
//
//    Gunther H. Weber, Wed Jul 18 15:38:36 PDT 2012
//    Support anisotropic refinement.
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
    std::vector<Neighbor> neighbors;
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
    void   AddNeighbor(int,int,int[3],int[6],
                       RefinementRelationship = SAME_REFINEMENT_LEVEL,
                       const std::vector<int>& ref_ratio = std::vector<int>(3, 1),
                       NeighborRelationship = SYMMETRIC_NEIGHBOR);
    void   DeleteNeighbor(int, std::vector<Boundary> &);
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
//    Jeremy Meredith, Thu Aug 14 10:24:12 EDT 2014
//    Added ability to fill/communicate/receive values from the 'mixnext'
//    array for mixed boundary data.  We can't reliably use any other
//    information (like a change in zone ID) to determine when a segment
//    of mix data has ended.
//
//    Cyrus Harrison, Wed Dec 23 14:31:34 PST 2015
//    Added FindMatchIndex, which allows us to find the proper "match" values.
//
// ****************************************************************************
template<class T>
class BoundaryHelperFunctions
{
  private:
    avtStructuredDomainBoundaries *sdb;
  public:
    typedef T Storage;

    BoundaryHelperFunctions(avtStructuredDomainBoundaries *sdb_) : sdb(sdb_) { }

    T   ***InitializeBoundaryData();
    void   FillBoundaryData(int, const T*, T***, bool, int=1);
    void   FillMixedBoundaryData(int,avtMaterial*,const T*,T***,int***,int***,int***,std::vector<int>&);
    void   FillRectilinearBoundaryData(int, const T*, const T*, const T*, T***);
    void   CommunicateBoundaryData(const std::vector<int>&, T***, bool, int=1);
    void   CommunicateMixedBoundaryData(const std::vector<int>&,T***,int***,int***,int***,std::vector< std::vector<int> > &);
    void   CopyOldValues(int, const T*, T*, bool, int=1);
    void   CopyOldMixedValues(avtMaterial*,const T*, T*);
    void   CopyOldRectilinearValues(int, const T*, T*, int);
    void   SetNewBoundaryData(int, T***, T*, bool, int=1);
    void   SetNewMixedBoundaryData(int,avtMaterial*,const std::vector< std::vector<int> >&,int***,T***,int***,int***,int***,int*,T*,int*,int*,int*,int&);
    void   SetNewRectilinearBoundaryData(int, T***, T*, T*, T*);
    void   FakeNonexistentBoundaryData(int, T*, bool, int=1);
    void   FreeBoundaryData(T***);
    int    FindMatchIndex(int src_domain, int nei_domain);
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
//    Hank Childs, Tue Jul 31 15:41:17 PDT 2007
//    Add support for getting boundary presence for individual domains.
//
//    Hank Childs, Fri Nov 14 10:49:08 PST 2008
//    Add data members for creating domain boundaries for AMR data sets more
//    efficiently.
//
//    Hank Childs, Tue Jan  4 11:33:47 PST 2011
//    Add support for ghost data across refinement levels.
//
//    Hank Childs, Thu Sep 29 14:49:20 PDT 2011
//    Add extra data members to CreateGhostZones.  This allows 
//    REFINED_ZONE_IN_AMR_GRID to be properly represented in zones that are 
//    also DUPLICATED_ZONE_INTERNAL_TO_PROBLEM.
//
//    Brad Whitlock, Sun Apr 22 09:59:55 PDT 2012
//    Support for double.
//
//    Gunther H. Weber, Thu Jun 14 17:31:00 PDT 2012
//    Add method to select new ghost zone generation method for AMRStichCell
//    operator.
//
//    Gunther H. Weber, Wed Jul 18 15:38:36 PDT 2012
//    Support anisotropic refinement.
//
//    Gunther H. Weber, Thu Aug 21 14:16:29 PDT 2014
//    Make new ghost zone generation method default.
//
//    Kevin Griffin, Tue Apr 21 17:41:51 PDT 2015
//    Added the ExchangeVector method so that the Exchange*Vector methods can
//    be called based on the underlying data type.
//
// ****************************************************************************

class DATABASE_API avtStructuredDomainBoundaries :  public avtDomainBoundaries
{
  public:
             avtStructuredDomainBoundaries(bool _canComputeNeighborsFromExtents = false);
    virtual ~avtStructuredDomainBoundaries();

    static void Destruct(void *);
    static void SetCreateGhostsForTIntersections(bool createGhosts = false)
    {
        createGhostsForTIntersections = createGhosts;
    }

    void     SetNumDomains(int nd);
    void     SetExtents(int domain, int e[6]);
    void     AddNeighbor(int domain, int d,int mi, int o[3], int e[6],
                         RefinementRelationship = SAME_REFINEMENT_LEVEL,
                         const std::vector<int>& ref_ratio = std::vector<int>(3, 1),
                         NeighborRelationship = SYMMETRIC_NEIGHBOR);
    void     Finish(int domain);

    void     GetExtents(int domain, int e[6]);

    //  methods for cases where neighbors can be computed
    void  SetIndicesForRectGrid(int domain, int e[6]);
    void  SetIndicesForAMRPatch(int domain, int level, int e[6]);
    void  SetRefinementRatios(const std::vector<int> &r); // Isotropic refinement ratios
    void  SetRefinementRatios(const std::vector< std::vector<int> > &r) { ref_ratios = r; }; // Anisotropic refinement ratios
    void  CalculateBoundaries(void);
    void  GetNeighborPresence(int domain, bool *hasNeighbor, 
                              std::vector<int> &);

    std::vector<Neighbor>                  GetNeighbors(int domain);

    virtual std::vector<vtkDataArray*>     ExchangeScalar(std::vector<int>   domainNum,
                                             bool                  isPointData,
                                             std::vector<vtkDataArray*> scalars);

    virtual std::vector<vtkDataArray*>     ExchangeVector(std::vector<int> domainNum,
                                                               bool                   isPointData,
                                                               std::vector<vtkDataArray*>  vectors);

    virtual std::vector<avtMaterial*>      ExchangeMaterial(std::vector<int>   domainNum,
                                              std::vector<avtMaterial*>   mats);

    virtual std::vector<avtMixedVariable*> ExchangeMixVar(std::vector<int> domainNum,
                                              const std::vector<avtMaterial*>   mats,
                                              std::vector<avtMixedVariable*>    mixvars);

    virtual void                           CreateGhostNodes(std::vector<int>   domainNum,
                                              std::vector<vtkDataSet*> meshes,
                                              std::vector<int> &);

    virtual bool                           RequiresCommunication(avtGhostDataType);
    virtual bool                           ConfirmMesh(std::vector<int>      domainNum,
                                               std::vector<vtkDataSet*> meshes);
    virtual void                           ResetCachedMembers();

  private:
    virtual std::vector<vtkDataArray*>     ExchangeFloatScalar(std::vector<int> domainNum,
                                             bool                  isPointData,
                                             std::vector<vtkDataArray*> scalars);

    virtual std::vector<vtkDataArray*>     ExchangeDoubleScalar(std::vector<int> domainNum,
                                             bool                  isPointData,
                                             std::vector<vtkDataArray*> scalars);

    virtual std::vector<vtkDataArray*>     ExchangeIntScalar(std::vector<int>  domainNum,
                                             bool                  isPointData,
                                             std::vector<vtkDataArray*> scalars);

    virtual std::vector<vtkDataArray*>     ExchangeUCharScalar(std::vector<int> domainNum,
                                             bool                  isPointData,
                                             std::vector<vtkDataArray*> scalars);
    
    virtual std::vector<vtkDataArray*>     ExchangeFloatVector(std::vector<int> domainNum,
                                                               bool                   isPointData,
                                                               std::vector<vtkDataArray*>  vectors);
    
    virtual std::vector<vtkDataArray*>     ExchangeDoubleVector(std::vector<int> domainNum,
                                                                bool                   isPointData,
                                                                std::vector<vtkDataArray*>  vectors);
    
    virtual std::vector<vtkDataArray*>     ExchangeIntVector(std::vector<int>  domainNum,
                                                             bool                  isPointData,
                                                             std::vector<vtkDataArray*> vectors);


    static bool                       createGhostsForTIntersections;

  protected:
    // data
    std::vector<Boundary>                   wholeBoundary;
    std::vector<Boundary>                   boundary;

    // data for cases where neighbors can be computed
    bool                                    shouldComputeNeighborsFromExtents;
    std::vector<int>                        extents;
    std::vector<int>                        levels;
    std::vector<int>                        domain2proc;

    int                                     maxAMRLevel;
    std::vector<std::vector <int> >         ref_ratios;
    bool                                    haveCalculatedBoundaries;

    friend class BoundaryHelperFunctions<int>;
    friend class BoundaryHelperFunctions<float>;
    friend class BoundaryHelperFunctions<double>;
    friend class BoundaryHelperFunctions<unsigned char>;

    BoundaryHelperFunctions<int>           *bhf_int;
    BoundaryHelperFunctions<float>         *bhf_float;
    BoundaryHelperFunctions<double>        *bhf_double;
    BoundaryHelperFunctions<unsigned char> *bhf_uchar;

    // helper methods
    std::vector<int> CreateDomainToProcessorMap(const std::vector<int>&);
    void        CreateCurrentDomainBoundaryInformation(const std::vector<int>&);
    bool       *SetExistence(int, bool);
    void        CreateGhostZones(vtkDataSet *, vtkDataSet *, Boundary *,
                                 bool = false, int = -1, unsigned char *** = NULL);

    friend ostream &operator<<(ostream&, Boundary&);
};


// the communication methods are optimized for each class of mesh
class DATABASE_API avtCurvilinearDomainBoundaries
    : public avtStructuredDomainBoundaries
{
  public:
    avtCurvilinearDomainBoundaries(bool _canComputeNeighborsFromExtents = false) :
        avtStructuredDomainBoundaries(_canComputeNeighborsFromExtents) {;};
   
    virtual std::vector<vtkDataSet*>    ExchangeMesh(std::vector<int> domainNum,
                                                std::vector<vtkDataSet*> meshes);
  protected:
    template <typename Helper>
    void ExchangeMesh(Helper *bhf, int vtktype,
                      std::vector<int> domainNum, 
                      std::vector<vtkDataSet*> meshes,
                      std::vector<vtkDataSet*> &out);
};


// the communication methods are optimized for each class of mesh
class DATABASE_API avtRectilinearDomainBoundaries
    : public avtStructuredDomainBoundaries
{
  public:
    avtRectilinearDomainBoundaries(bool _canComputeNeighborsFromExtents = false) :
        avtStructuredDomainBoundaries(_canComputeNeighborsFromExtents) {;};

    virtual std::vector<vtkDataSet*>    ExchangeMesh(std::vector<int> domainNum,
                                                std::vector<vtkDataSet*> meshes);
};


#endif
