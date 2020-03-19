// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                         avtCellLocatorClasic.h                            //
// ************************************************************************* //

#ifndef AVT_CELLLOCATORCLASSIC_H
#define AVT_CELLLOCATORCLASSIC_H

#include <avtCellLocator.h>
#include <ivp_exports.h>

class vtkIdList;

class IVP_API avtCellLocatorClassic : public avtCellLocator
{
public:

    avtCellLocatorClassic( vtkDataSet* );
    ~avtCellLocatorClassic();

    vtkIdType FindCell( const double pos[3], 
                        avtInterpolationWeights* weights,
                        bool ignoreGhostCells ) const;

protected:

    void Free();
    void Build();

    unsigned int Level;
    unsigned int MaxLevel;
    unsigned int NumberOfDivisions;
    unsigned int NumberOfOctants;
    unsigned int NumberOfCellsPerBucket;
    unsigned int NumberOfCellsPerNode;

    double       H[3];
    double       B[6];

    vtkIdList**  Tree;
};

#endif
