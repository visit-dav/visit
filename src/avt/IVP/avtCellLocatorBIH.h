// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                            avtCellTreeLocator.h                           //
// ************************************************************************* //

#ifndef AVT_CELL_LOCATOR_BIH
#define AVT_CELL_LOCATOR_BIH

#include <avtCellLocator.h>

class celltree;
class vtkIdTypeArray;
class vtkCellArray;

class IVP_API avtCellLocatorBIH : public avtCellLocator
{
public:

    avtCellLocatorBIH( vtkDataSet* ds );
    ~avtCellLocatorBIH();

    vtkIdType FindCell( const double pos[3], 
                        avtInterpolationWeights* iw,
                        bool ignoreGhostCells ) const;

protected:

    void FindCellRecursive( const double pos[3], 
                            avtInterpolationWeights* weights,
                            unsigned int node,
                            vtkIdType& cell,
                            bool ignoreGhostCells ) const;

    void Build();
    void Free();

    int MaxCellsPerLeaf;
    int NumberOfBuckets;

    celltree* Tree;

    vtkIdType* CellArray;
    vtkIdType* Locations;
};

#endif
