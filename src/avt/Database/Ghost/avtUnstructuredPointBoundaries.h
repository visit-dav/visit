// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ****************************************************************************
//                      avtUnstructuredPointBoundaries.h
// ****************************************************************************

#ifndef AVT_UNSTRUCTURED_POINT_BOUNDARIES_H
#define AVT_UNSTRUCTURED_POINT_BOUNDARIES_H

#include <database_exports.h>
#include <avtUnstructuredDomainBoundaries.h>

#include <vector>
#include <utility>

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
    
    virtual std::vector<vtkDataSet*>       ExchangeMesh(std::vector<int>       domainNum,
                                         std::vector<vtkDataSet*>   meshes);

    virtual bool                      ConfirmMesh(std::vector<int>      domainNum,
                                        std::vector<vtkDataSet*> meshes);
    
  protected:
    void                              Generate(std::vector<int> domainNum,
                                               std::vector<vtkDataSet*> meshes);

    bool                              CheckGenerated(int d1, int d2);

    std::vector<std::vector<bool> >             generated;
};

#endif
