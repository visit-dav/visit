// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                           avtNekDomainBoundaries.h                        //
// ************************************************************************* //

#ifndef AVT_NEK_DOMAIN_BOUNDARIES_H
#define AVT_NEK_DOMAIN_BOUNDARIES_H

#include <database_exports.h>

#include <avtDomainBoundaries.h>
#include <avtGhostData.h>

#include <vector>

class vtkDataSet;
class vtkDataArray;
class avtMixedVariable;
class avtMaterial;

// ****************************************************************************
//  Class:  avtNekDomainBoundaries
//
//  Purpose:
//    Encapsulate domain boundary information specific to Nek data.
//    This assumes the following while determining face matching:
//    - all domains are topologically rectilinear, and have the same
//      dimensions
//    - domains touch at the corners
//    - only bit-wise identical corners are counted as matches
//
//  Programmer:  Dave Bremer
//  Creation:    Tue Jan  8 16:19:08 PST 2008
//
//  Modifications:
//    Dave Bremer, Thu Jan 24 14:53:27 PST 2008
//    Rewrote the matching algorithm to scale better.  Changed face matching
//    to match 3 points instead of 4.  Added a flag to enable/disable caching
//    the adjacency structure.
//
//    Hank Childs, Thu Feb 14 17:04:56 PST 2008
//    Inherit from avtDomainBoundaries.  Also add method 
//    CanOnlyCreateGhostNodes.
//   
//    Hank Childs, Mon Feb 28 10:02:55 PST 2011
//    Add data member for multiple blocks.  This is for the case where multiple
//    domains have been put in the same VTK data set.
//
//    Brad Whitlock, Sun Apr 22 10:29:43 PDT 2012
//    Double support.
//
//    Kevin Griffin, Tue Apr 21 17:41:51 PDT 2015
//    Added the ExchangeVector method and removed the Exchange*Vector methods.
//    (* = Float, Double, etc)
//
//    Eric Brugger, Tue Dec 11 09:46:02 PST 2018
//    Corrected a bug generating ghost nodes for Nek5000 files where all the
//    nodes were marked as ghost when the mesh was 2D.
//
// ****************************************************************************

class DATABASE_API avtNekDomainBoundaries : public avtDomainBoundaries
{
  public:
                 avtNekDomainBoundaries();
    virtual     ~avtNekDomainBoundaries();

    static void Destruct(void *);

    // These are required to be implemented, but an exception is thrown if used.
    virtual std::vector<vtkDataSet*>       ExchangeMesh(std::vector<int>       domainNum,
                                               std::vector<vtkDataSet*>   meshes);

    virtual std::vector<vtkDataArray*>     ExchangeScalar(std::vector<int>     domainNum,
                                               bool                  isPointData,
                                               std::vector<vtkDataArray*> scalars);
    
    virtual std::vector<vtkDataArray*>     ExchangeVector(std::vector<int> domainNum,
                                                       bool isPointData,
                                                       std::vector<vtkDataArray*> vectors);

    virtual std::vector<avtMaterial*>      ExchangeMaterial(std::vector<int>   domainNum,
                                              std::vector<avtMaterial*>   mats);

    virtual std::vector<avtMixedVariable*> ExchangeMixVar(std::vector<int>     domainNum,
                                        const std::vector<avtMaterial*>   mats,
                                        std::vector<avtMixedVariable*>    mixvars);

    // These are inherited and have real implementations.
    virtual void                      CreateGhostNodes(std::vector<int>   domainNum,
                                               std::vector<vtkDataSet*>   meshes,
                                               std::vector<int> &allDomains);
    virtual bool                      CanOnlyCreateGhostNodes(void)
                                                              { return true; };
    virtual bool                      RequiresCommunication(avtGhostDataType);
    virtual bool                      ConfirmMesh(std::vector<int>      domainNum,
                                                  std::vector<vtkDataSet*> meshes);
    virtual void                      ResetCachedMembers(void) {;};

    // These are unique to this class
    virtual void                      SetDomainInfo(int num_domains, 
                                                    int num_dims,
                                                    const int dims[3],
                                                    bool multipleBlocks = false);
    virtual void                      SetCacheDomainAdjacency(bool bCache) 
                                          {bSaveDomainInfo = bCache;}

  protected:
    struct Face
    {
        void  Set(const float *points);
        void  Sort();

        float pts[9];
        int   domain;
        int   side;
        int   proc;
    };
    static int CompareFaces(const void *f0, const void *f1);
    static int CompareFaceProcs(const void *f0, const void *f1);

    void       CreateNeighborList(const std::vector<int>         &domainNum,
                                  const std::vector<vtkDataSet*> &meshes);
    int        ExtractMatchingFaces(Face *faces, int nFaces, 
                                    std::vector<int> &aMatchedFaces, 
                                    bool bCompressFaces);


    //There are 6 entries for each domain in the data, holding the 
    //number of the domain adjacent to each face, or -1 if it is an
    //exterior face.
    int *aNeighborDomains;
    bool bFullDomainInfo; //true if data in aNeighborDomains covers all domains
    bool bSaveDomainInfo; //true if this class should save aNeighborDomains to
                          //use in future calls to CreateGhostNodes

    int  nDomains;
    int  nDims;
    int  iBlockSize[3];
    int  ptsPerDomain;
    bool multipleBlocks;
    int  aCornerOffsets[8];
};

#endif


