// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                            avtCellLocatorRectEdge.h                           //
// ************************************************************************* //

#ifndef AVT_CELL_LOCATOR_RECT_EDGE
#define AVT_CELL_LOCATOR_RECT_EDGE

#include <avtCellLocatorRect.h>
#include <vector>

// ****************************************************************************
//  Author: 
//
//    Alexander Pletzer, Wed Nov 27 13:16:34 MST 2013
//    Support for edge centered data
//
// ****************************************************************************

class IVP_API avtCellLocatorRectEdge : public avtCellLocatorRect
{
public:

    avtCellLocatorRectEdge( vtkDataSet* ds );
    ~avtCellLocatorRectEdge();

    vtkIdType FindCell( const double pos[3], 
                        avtInterpolationWeights* iw,
                        bool ignoreGhostCells ) const;

    // Each edge component will get a different set of weights
    void SetDirection(size_t dir);

protected:

    void Build();
    void Free();

    size_t direction;
};

#endif
