/*****************************************************************************
*
* Copyright (c) 2000 - 2017, Lawrence Livermore National Security, LLC
* Produced at the Lawrence Livermore National Laboratory
* LLNL-CODE-442911
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
//                     avtUnstructuredDomainBoundaries.h                     //
// ************************************************************************* //

#ifndef AVT_UNSTRUCTURED_DOMAIN_BOUNDARIES_H
#define AVT_UNSTRUCTURED_DOMAIN_BOUNDARIES_H

#include <database_exports.h>
#include <avtDomainBoundaries.h>

#include <map>
#include <vector>
#include <utility>

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
//    Hank Childs, Fri Mar  2 07:43:47 PST 2007
//    Add support for exchanging mixed materials.
//
//    Kevin Griffin, Tue Apr 21 17:41:51 PDT 2015
//    Added the ExchangeVector method to call the correct Exchange*Vector method
//    based on the underlying data. (* = Float, Double, etc)
//
// ****************************************************************************

class DATABASE_API avtUnstructuredDomainBoundaries : public avtDomainBoundaries
{
  public:
                 avtUnstructuredDomainBoundaries();
    virtual      ~avtUnstructuredDomainBoundaries();

    static void Destruct(void *);

    void        SetTotalNumberOfDomains(int nd) { nTotalDomains = nd; }
    
    void        SetSharedPoints(int d1, int d2, const std::vector<int> &d1pts,
                                                const std::vector<int> &d2pts);
    
    void        SetGivenCellsAndPoints(int fromDom, int toDom,
                                       const std::vector<int> &cells,
                                       const std::vector<int> &points,
                                       bool filterShared = false);
    
    virtual std::vector<vtkDataSet*>    ExchangeMesh(
                                         std::vector<int> domainNum,
                                         std::vector<vtkDataSet*>   meshes);
    
    virtual std::vector<vtkDataArray*>  ExchangeScalar(
                                         std::vector<int> domainNum,
                                         bool isPointData,
                                         std::vector<vtkDataArray*> scalars);
    
    virtual std::vector<vtkDataArray*>  ExchangeVector(std::vector<int> domainNum,
                                                       bool isPointData,
                                                       std::vector<vtkDataArray*> vectors);

    virtual std::vector<avtMaterial*>   ExchangeMaterial(
                                         std::vector<int> domainNum,
                                         std::vector<avtMaterial*>   mats);
    virtual std::vector<avtMaterial*>   ExchangeMixedMaterials(
                                         std::vector<int> domainNum,
                                         std::vector<avtMaterial*>   mats);
    virtual std::vector<avtMaterial*>   ExchangeCleanMaterials(
                                         std::vector<int> domainNum,
                                         std::vector<avtMaterial*>   mats);

    virtual std::vector<avtMixedVariable*> ExchangeMixVar(
                                      std::vector<int> domainNum,
                                      const std::vector<avtMaterial*>  mats,
                                      std::vector<avtMixedVariable*> mixvars);
    
    virtual void                CreateGhostNodes(std::vector<int>   domainNum,
                                               std::vector<vtkDataSet*> meshes,
                                               std::vector<int> &allDomains);
    virtual bool                CreatesRobustGhostNodes(void) 
                                                             { return false; };

    virtual bool                RequiresCommunication(avtGhostDataType);
    virtual bool                ConfirmMesh(std::vector<int> domainNum,
                                            std::vector<vtkDataSet*> meshes);
    
  protected:

    int                                  nTotalDomains;
    
    std::vector<std::pair<int, int> >    giveIndex;
    std::vector<std::vector<int> >       givenCells;
    std::vector<std::vector<int> >       givenPoints;
    std::vector<std::map<int, int> >     sharedPointsMap;

    // [<send, recv>] = startingCell for recvDom
    std::map<std::pair<int, int>, int>   startingCell;
    std::map<std::pair<int, int>, int>   startingPoint;

    int                         GetGivenIndex(int from, int to);

    template <class T>
    std::vector<vtkDataArray*>  ExchangeData(
                                    std::vector<int> &domainNum,
                                    bool isPointData,
                                    std::vector<vtkDataArray*> &data);

    template <class T>
    std::vector<vtkDataSet*>  ExchangeMeshT(
                               std::vector<int> domainNum,
                               std::vector<vtkDataSet*> meshes);

    // Communication methods
    std::vector<int>     CreateDomainToProcessorMap(
                               const std::vector<int> &domainNum);

    template <class T>
    void            CommunicateMeshInformation(
                               const std::vector<int> &domain2proc,
                               const std::vector<int> &domainNum,
                               const std::vector<vtkDataSet *> &,
                               T ***&gainedPoints,
                               int ***&cellTypes,
                               int ****&cellPoints,
                               int ***&origPointIds,
                               int **&nGainedPoints,
                               int **&nGainedCells,
                               int ***&nPointsPerCell);

    void            CommunicateMaterialInformation(
                               const std::vector<int> &domain2proc,
                               const std::vector<int> &domainNum,
                               const std::vector<avtMaterial*> &,
                               int **&, int **&, int ***&,
                               int ***&, float ***&);

    void            CommunicateMixvarInformation(
                               const std::vector<int> &domain2proc,
                               const std::vector<int> &domainNum,
                               const std::vector<avtMaterial*> &,
                               const std::vector<avtMixedVariable *> &,
                               int **&, float ***&);
                      

    template <class T>
    void            CommunicateDataInformation(
                               const std::vector<int> &domain2proc,
                               const std::vector<int> &domainNum,
                               const std::vector<vtkDataArray *>&,
                               bool isPointData,
                               T ***&gainedData,
                               int **&nGainedTuples);
    
private:
    
    virtual std::vector<vtkDataArray*>  ExchangeFloatVector(
                                                            std::vector<int> domainNum,
                                                            bool isPointData,
                                                            std::vector<vtkDataArray*> vectors);
    
    virtual std::vector<vtkDataArray*>  ExchangeDoubleVector(
                                                             std::vector<int> domainNum,
                                                             bool isPointData,
                                                             std::vector<vtkDataArray*> vectors);
    
    virtual std::vector<vtkDataArray*>  ExchangeIntVector(
                                                          std::vector<int> domainNum,
                                                          bool isPointData,
                                                          std::vector<vtkDataArray*> vectors);
};

#endif
