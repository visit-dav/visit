/*****************************************************************************
*
* Copyright (c) 2000 - 2007, The Regents of the University of California
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
//                     avtUnstructuredDomainBoundaries.h                     //
// ************************************************************************* //

#ifndef AVT_UNSTRUCTURED_DOMAIN_BOUNDARIES_H
#define AVT_UNSTRUCTURED_DOMAIN_BOUNDARIES_H

#include <database_exports.h>
#include <avtDomainBoundaries.h>

#include <map>
#include <vector>
#include <utility>

using std::vector;
using std::pair;
using std::map;

class vtkDataSet;
class vtkDataArray;
class avtMixedVariable;
class avtMaterial;

// ****************************************************************************
//  Class:  avtUnstructuredDomainBoundaries
//
//  Purpose:
//    Encapsulate domain boundary information for unstructured datasets.
//
//  Notes:
//    For each domain that an instance on a processor is responsible for,
//    it needs to know all of the shared points for those domains, and
//    what points and cells are being given by those domains.
//
//  Programmer:  Akira Haddox
//  Creation:    August 11, 2003
//
//  Modifications:
//
//    Hank Childs, Mon Aug 16 08:47:27 PDT 2004
//    Added CreateGhostNodes.
//
//    Brad Whitlock, Thu Sep 16 12:52:56 PDT 2004
//    I added some workarounds so it builds using the MSVC6.0 compiler on
//    Windows. The code should be unaffected on other systems.
//
//    Hank Childs, Sun Feb 27 12:12:03 PST 2005
//    Added RequiresCommunication.  Also added argument to CreateGhostNodes.
//
//    Hank Childs, Thu Jan 26 10:04:34 PST 2006
//    Add virtual method "CreatesRobustGhostNodes".
//
// ****************************************************************************

class DATABASE_API avtUnstructuredDomainBoundaries : public avtDomainBoundaries
{
  public:
                 avtUnstructuredDomainBoundaries();
    virtual      ~avtUnstructuredDomainBoundaries();

    static void Destruct(void *);

    void        SetTotalNumberOfDomains(int nd) { nTotalDomains = nd; }
    
    void        SetSharedPoints(int d1, int d2, const vector<int> &d1pts,
                                                const vector<int> &d2pts);
    
    void        SetGivenCellsAndPoints(int fromDom, int toDom,
                                       const vector<int> &cells,
                                       const vector<int> &points,
                                       bool filterShared = false);
    
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
    
    virtual void                      CreateGhostNodes(vector<int>   domainNum,
                                               vector<vtkDataSet*> meshes,
                                               vector<int> &allDomains);
    virtual bool                      CreatesRobustGhostNodes(void) 
                                                             { return false; };

    virtual bool                      RequiresCommunication(avtGhostDataType);
    virtual bool                      ConfirmMesh(vector<int>      domainNum,
                                        vector<vtkDataSet*> meshes);
    
  protected:
    int                               GetGivenIndex(int from, int to);

    int                             nTotalDomains;
    
    vector<std::pair<int, int> >    giveIndex;
    vector<vector<int> >            givenCells;
    vector<vector<int> >            givenPoints;
    vector<map<int, int> >          sharedPointsMap;

    // [<send, recv>] = startingCell for recvDom
    map<std::pair<int, int>, int>   startingCell;
    map<std::pair<int, int>, int>   startingPoint;


    template <class T>
    vector<vtkDataArray*>        ExchangeData(vector<int> &domainNum,
                                              bool isPointData,
                                              vector<vtkDataArray*> &data
#if defined(_WIN32) && defined(USING_MSVC6)
    // Extra argument to help the compiler instantiate the right function.
                                              , T signature
#endif
                                             );
    // Communication methods
    vector<int>     CreateDomainToProcessorMap(const vector<int> &domainNum);

    void            CommunicateMeshInformation(const vector<int> &domain2proc,
                                               const vector<int> &domainNum,
                                               const vector<vtkDataSet *> &,
                                               float ***&gainedPoints,
                                               int ***&cellTypes,
                                               int ****&cellPoints,
                                               int ***&origPointIds,
                                               int **&nGainedPoints,
                                               int **&nGainedCells,
                                               int ***&nPointsPerCell);

#if !defined(USING_MSVC6)
    template <class T>
    void            CommunicateDataInformation(const vector<int> &domain2proc,
                                               const vector<int> &domainNum,
                                               const vector<vtkDataArray *>&,
                                               bool isPointData,
                                               T ***&gainedData,
                                               int **&nGainedTuples);
#endif
};

#endif
