// ************************************************************************* //
//                            avtDomainBoundaries.h                          //
// ************************************************************************* //

#ifndef AVT_DOMAIN_BOUNDARIES_H
#define AVT_DOMAIN_BOUNDARIES_H

#include <database_exports.h>

#include <avtGhostData.h>

#include <vector>
using std::vector;

class vtkDataSet;
class vtkDataArray;
class avtMixedVariable;
class avtMaterial;

// ****************************************************************************
//  Class:  avtDomainBoundaries
//
//  Purpose:
//    Encapsulate domain boundary information.
//
//  Programmer:  Jeremy Meredith
//  Creation:    October 25, 2001
//
//  Modifications:
//    Jeremy Meredith, Thu Dec 13 11:47:06 PST 2001
//    Added mats to the exchange mixvars call.
//
//    Kathleen Bonnell, Fri Feb  8 11:03:49 PST 2002
//    vtkScalars and vtkVectors have been deprecated in VTK 4.0, 
//    use vtkDataArray instead.
//
//    Kathleen Bonnell, Mon May 20 13:40:17 PDT 2002
//    Made ExhangeVector into two methods to handle different underlying
//    data types (int, float).
//
//    Hank Childs, Sat Aug 14 06:41:00 PDT 2004
//    Added ghost nodes.
//
//    Hank Childs, Sun Feb 27 12:00:12 PST 2005
//    Added pure virtual methods RequiresCommunication.  Added "allDomains"
//    argument to CreateGhostNodes.
//
//    Hank Childs, Mon Jun 27 16:28:22 PDT 2005
//    Added virtual method ResetCachedMembers.
//
// ****************************************************************************

class DATABASE_API avtDomainBoundaries
{
  public:
                 avtDomainBoundaries();
    virtual      ~avtDomainBoundaries();

    virtual vector<vtkDataSet*>       ExchangeMesh(vector<int>       domainNum,
                                               vector<vtkDataSet*>   meshes)  =0;

    virtual vector<vtkDataArray*>     ExchangeScalar(vector<int>     domainNum,
                                               bool                  isPointData,
                                               vector<vtkDataArray*> scalars) =0;

    virtual vector<vtkDataArray*>     ExchangeFloatVector(vector<int> domainNum,
                                               bool                   isPointData,
                                               vector<vtkDataArray*>  vectors) =0;

    virtual vector<vtkDataArray*>     ExchangeIntVector(vector<int>  domainNum,
                                               bool                  isPointData,
                                               vector<vtkDataArray*> vectors) =0;

    virtual vector<avtMaterial*>      ExchangeMaterial(vector<int>   domainNum,
                                              vector<avtMaterial*>   mats)    =0;

    virtual vector<avtMixedVariable*> ExchangeMixVar(vector<int>     domainNum,
                                        const vector<avtMaterial*>   mats,
                                        vector<avtMixedVariable*>    mixvars) =0;
    virtual void                      CreateGhostNodes(vector<int>   domainNum,
                                               vector<vtkDataSet*>   meshes,
                                               vector<int> &)  =0;
    virtual bool                      RequiresCommunication(avtGhostDataType) = 0;
    virtual bool                      ConfirmMesh(vector<int>      domainNum,
                                               vector<vtkDataSet*> meshes)  =0;
    virtual void                      ResetCachedMembers(void) {;};
};

#endif
