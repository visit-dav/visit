// ****************************************************************************
//                      avtUnstructuredDomainBoundaries.h
// ****************************************************************************

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
    
    virtual bool                      ConfirmMesh(vector<int>      domainNum,
                                        vector<vtkDataSet*> meshes);
    
  protected:
    template <class T>
    void                              CopyPointer(T *src, T *dest,
                                                  int components, int count);

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
                                              vector<vtkDataArray*> &data);
    
    
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

    template <class T>
    void            CommunicateDataInformation(const vector<int> &domain2proc,
                                               const vector<int> &domainNum,
                                               const vector<vtkDataArray *>&,
                                               bool isPointData,
                                               T ***&gainedData,
                                               int **&nGainedTuples);
};

#endif
