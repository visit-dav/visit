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
//                           avtNekDomainBoundaries.h                        //
// ************************************************************************* //

#ifndef AVT_NEK_DOMAIN_BOUNDARIES_H
#define AVT_NEK_DOMAIN_BOUNDARIES_H

#include <database_exports.h>

#include <avtDomainBoundaries.h>
#include <avtGhostData.h>

#include <vector>
using std::vector;

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
// ****************************************************************************

class DATABASE_API avtNekDomainBoundaries : public avtDomainBoundaries
{
  public:
                 avtNekDomainBoundaries();
    virtual     ~avtNekDomainBoundaries();

    static void Destruct(void *);

    // These are required to be implemented, but an exception is thrown if used.
    virtual vector<vtkDataSet*>       ExchangeMesh(vector<int>       domainNum,
                                               vector<vtkDataSet*>   meshes);

    virtual vector<vtkDataArray*>     ExchangeScalar(vector<int>     domainNum,
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

    // These are inherited and have real implementations.
    virtual void                      CreateGhostNodes(vector<int>   domainNum,
                                               vector<vtkDataSet*>   meshes,
                                               vector<int> &allDomains);
    virtual bool                      CanOnlyCreateGhostNodes(void)
                                                              { return true; };
    virtual bool                      RequiresCommunication(avtGhostDataType);
    virtual bool                      ConfirmMesh(vector<int>      domainNum,
                                                  vector<vtkDataSet*> meshes);
    virtual void                      ResetCachedMembers(void) {;};

    // These are unique to this class
    virtual void                      SetDomainInfo(int num_domains, 
                                                    const int dims[3]);
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

    void       CreateNeighborList(const vector<int>         &domainNum,
                                  const vector<vtkDataSet*> &meshes);
    int        ExtractMatchingFaces(Face *faces, int nFaces, 
                                    vector<int> &aMatchedFaces, 
                                    bool bCompressFaces);


    //There are 6 entries for each domain in the data, holding the 
    //number of the domain adjacent to each face, or -1 if it is an
    //exterior face.
    int *aNeighborDomains;
    bool bFullDomainInfo; //true if data in aNeighborDomains covers all domains
    bool bSaveDomainInfo; //true if this class should save aNeighborDomains to
                          //use in future calls to CreateGhostNodes

    int  nDomains;
    int  iBlockSize[3];
    int  aCornerOffsets[8];
};

#endif


