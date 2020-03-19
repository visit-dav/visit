// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                            avtCellLocatorRect.h                           //
// ************************************************************************* //

#ifndef AVT_CELL_LOCATOR_RECT
#define AVT_CELL_LOCATOR_RECT

#include <avtCellLocator.h>
#include <vector>

// ****************************************************************************
//  Modifications:
//
//    Hank Childs, Wed Sep  5 15:41:30 PDT 2012
//    Add support for coordinate arrays that are monotonically descending.
//
// ****************************************************************************

class IVP_API avtCellLocatorRect : public avtCellLocator
{
public:

    avtCellLocatorRect( vtkDataSet* ds );
    ~avtCellLocatorRect();

    vtkIdType FindCell( const double pos[3], 
                        avtInterpolationWeights* iw,
                        bool ignoreGhostCells ) const;

protected:

    void Build();
    void Free();

    std::vector<double> coord[3];
    bool                ascending[3];
};

#endif
