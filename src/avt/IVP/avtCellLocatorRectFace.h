// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                            avtCellLocatorRectFace.h                           //
// ************************************************************************* //

#ifndef AVT_CELL_LOCATOR_RECT_FACE
#define AVT_CELL_LOCATOR_RECT_FACE

#include <avtCellLocatorRect.h>
#include <vector>

// ****************************************************************************
//  Author: 
//
//    Alexander Pletzer, Wed Nov 27 13:16:34 MST 2013
//    Support for face centered data
//
// ****************************************************************************

class IVP_API avtCellLocatorRectFace : public avtCellLocatorRect
{
public:

    avtCellLocatorRectFace( vtkDataSet* ds );
    ~avtCellLocatorRectFace();

    vtkIdType FindCell( const double pos[3], 
                        avtInterpolationWeights* iw,
                        bool ignoreGhostCells ) const;

    // Each face component will get a different set of weights
    void SetDirection(size_t dir);

protected:

    void Build();
    void Free();

    size_t direction;
};

#endif
