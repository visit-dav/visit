// ************************************************************************* //
//                           IntervalTree_VTK.h                              //
// ************************************************************************* //

#ifndef INTERVAL_TREE_VTK_H
#define INTERVAL_TREE_VTK_H
#include <siloobj_vtk_exports.h>

#include <visitstream.h>
#include <silo.h>

#include <IntervalTree.h>


// ****************************************************************************
//  Class: IntervalTree_VTK
// 
//  Purpose:
//      Allows access to an interval tree object.
//
//  Programmer: Hank Childs
//  Creation:   February 2, 2000
//
//  Modifications:
// 
//     Hank Childs, Wed Apr 12 16:52:25 PDT 2000
//     Added GetMax and GetMin methods.
//
// ****************************************************************************

class SILOOBJ_VTK_API IntervalTree_VTK : public IntervalTree
{
  public:
    int            GetDomainsList(const float *, float,
                                  int **) const;
    float          GetMax(int) const;
    float          GetMin(int) const;
};


#endif


