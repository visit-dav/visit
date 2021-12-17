// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                              avtCellLocator.h                             //
// ************************************************************************* //

#ifndef AVT_CELLLOCATOR_H
#define AVT_CELLLOCATOR_H

#include <ivp_exports.h>
#include <avtVector.h>
#include <vtkType.h>
#include <vector>

#include <ref_ptr.h>

class vtkDataSet;
class vtkCellArray;

struct avtInterpolationWeight
{
    vtkIdType i;    // point id
    double    w;    // point weight
};

typedef std::vector<avtInterpolationWeight> avtInterpolationWeights;

// ************************************************************************* //
//  Modifications:
//
//    Hank Childs, Fri Oct 29 12:13:07 PDT 2010
//    Add new data members for efficient curvilinear location.
//
//    Hank Childs, Fri Nov 19 14:45:53 PST 2010
//    Add support for voxels.
//
//    Hank Childs, Sun Nov 28 11:34:04 PST 2010
//    Add support for caching cell locators via void_ref_ptr.
//
//    David Camp, Tue Sep 13 08:16:35 PDT 2011
//    Changed the SetDataSet function to virtual. You may need to reset
//    pointer to the new dataset.
//
// ************************************************************************* //

class IVP_API avtCellLocator
{
  public:
                    avtCellLocator( vtkDataSet* );
    virtual        ~avtCellLocator();

    vtkDataSet     *GetDataSet() { return dataSet; }
    virtual void    SetDataSet(vtkDataSet *ds);
    void            ReleaseDataSet();

    virtual vtkIdType FindCell( const double pos[3],
                                avtInterpolationWeights* iw,
                                bool ignoreGhostCells ) const = 0;
    static void     Destruct(void *);

  protected:

    void CopyCell( vtkIdType cellid, vtkIdType* ids,
                   double pts[][3] ) const;

    bool TestCell( vtkIdType id, const double pos[3],
                   avtInterpolationWeights* iw,
                   bool ignoreGhostCells ) const;

    bool TestTet( vtkIdType id, const double pos[3],
                  avtInterpolationWeights* iw ) const;
    bool TestHex( vtkIdType id, const double pos[3],
                  avtInterpolationWeights* iw ) const;
    bool TestPrism( vtkIdType id, const double pos[3],
                    avtInterpolationWeights* iw ) const;
    bool TestVoxel( vtkIdType id, const double pos[3],
                    avtInterpolationWeights* iw ) const;

    vtkDataSet*    dataSet {nullptr};
    vtkCellArray*  cellPtr {nullptr};
    int*           strDimPtr {nullptr};
    bool           normal2D;
    bool           normal3D;
    float*         fCoordPtr {nullptr};
    double*        dCoordPtr {nullptr};
    unsigned char* ghostPtr  {nullptr};
};

typedef ref_ptr<avtCellLocator> avtCellLocator_p;

#endif
