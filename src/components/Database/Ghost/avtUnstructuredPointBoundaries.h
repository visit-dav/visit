// ****************************************************************************
//                      avtUnstructuredPointBoundaries.h
// ****************************************************************************

#ifndef AVT_UNSTRUCTURED_POINT_BOUNDARIES_H
#define AVT_UNSTRUCTURED_POINT_BOUNDARIES_H

#include <database_exports.h>
#include <avtUnstructuredDomainBoundaries.h>

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
//  Class:  avtUnstructuredPointBoundaries
//
//  Purpose:
//    A domain boundaries for unstructured datasets which constructs
//    the boundaries from the list of shared points. When using this
//    class, only SetSharedPoints needs to be called. It will provide
//    the needed information to SetGivenCellsAndPoints.
//
//  Notes:
//    This class is based on the assumption that ExchangeMesh or
//    ConfirmMesh will be called before any of the other Exchange
//    methods.
//
//  Programmer:  Akira Haddox
//  Creation:    August 11, 2003
//
// ****************************************************************************

class DATABASE_API avtUnstructuredPointBoundaries
                                       : public avtUnstructuredDomainBoundaries
{
  public:
                 avtUnstructuredPointBoundaries();
    virtual      ~avtUnstructuredPointBoundaries();

    static void Destruct(void *);
    
    virtual vector<vtkDataSet*>       ExchangeMesh(vector<int>       domainNum,
                                         vector<vtkDataSet*>   meshes);

    virtual bool                      ConfirmMesh(vector<int>      domainNum,
                                        vector<vtkDataSet*> meshes);
    
  protected:
    void                              Generate(vector<int> domainNum,
                                               vector<vtkDataSet*> meshes);

    bool                              CheckGenerated(int d1, int d2);

    vector<vector<bool> >             generated;
};

#endif
