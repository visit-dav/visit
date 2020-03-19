// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#include <avtCellLocatorClassic.h>
#include <vtkGenericCell.h>
#include <vtkDataSet.h>

#include <DebugStream.h>

#include <math.h>

typedef vtkIdList *vtkIdListPtr;

//----------------------------------------------------------------------------

avtCellLocatorClassic::avtCellLocatorClassic( vtkDataSet* ds ) : 
    avtCellLocator(ds)
{
    Tree = NULL;

    NumberOfCellsPerBucket = 25;
    MaxLevel = 8;
    Level = 8;

    NumberOfDivisions = 1;
    H[0] = H[1] = H[2] = 1.0;

    Build();
}

//----------------------------------------------------------------------------

avtCellLocatorClassic::~avtCellLocatorClassic()
{
    Free();
}

//----------------------------------------------------------------------------

void avtCellLocatorClassic::Free()
{
    if( Tree )
    {
        for( unsigned int i=0; i<NumberOfOctants; i++ )
        {
            if( Tree[i] )
                Tree[i]->Delete();
        }

        delete[] Tree;
        Tree = NULL;
    }
}

//----------------------------------------------------------------------------

void avtCellLocatorClassic::Build()
{
    vtkIdType numCells;

    if( !dataSet || (numCells = dataSet->GetNumberOfCells()) < 1 )
    {
        if (DebugStream::Level5())
        {
            debug5 << "avtCellLocatorClassic::Build(): no cells to subdivide\n";
        }
        return;
    }

    if( Tree )
        Free();

    dataSet->GetBounds( B );

    double length = sqrt( (B[1]-B[0])*(B[1]-B[0]) +
                          (B[3]-B[2])*(B[3]-B[2]) +
                          (B[5]-B[4])*(B[5]-B[4]) );

    for( int i=0; i<3; i++ )
    {
        if( (B[2*i+1] - B[2*i]) <= (length/1000.0) )
        {
            // bump out the B a little of if min == max
            B[2*i]   -= length*0.01;
            B[2*i+1] += length*0.01;
        }
    }

    Level = (int)ceil( log( (double)numCells/(double)NumberOfCellsPerBucket ) / log( 8.0 ) );

    if( Level > MaxLevel )
        Level = MaxLevel;

    // compute number of octants and number of divisions
    int ndivs = 1, prod = 1, noct = 1;

    for( unsigned int i=0; i<Level; ++i )
    {
        ndivs *= 2;
        prod  *= 8;
        noct  += prod;
    }

    NumberOfDivisions = ndivs;
    NumberOfOctants   = noct;

    printf( "Level = %u, Number of divisions: %u\nNumber of octants: %u\n", Level, ndivs, noct );

    Tree = new vtkIdListPtr[noct];
    memset( Tree, 0, noct*sizeof(vtkIdListPtr) );

    double htol[3];

    for( int i=0; i<3; i++ )
    {
        H[i] = (B[2*i+1] - B[2*i]) / ndivs;
        htol[i] = H[i]/100.0;
    }

    //  Insert each cell into the appropriate octant.  Make sure cell
    //  falls within octant.
    vtkIdType parentOffset = noct - (ndivs * ndivs * ndivs);

    for( vtkIdType id=0; id<numCells; id++ )
    {
        double bounds[6];
        dataSet->GetCellBounds( id, bounds );

        int min[3], max[3];

        // find min/max locations of bounding box
        for( int i=0; i<3; i++ )
        {
            min[i] = (int)( (bounds[2*i]   - B[2*i] - htol[i]) / H[i] );
            max[i] = (int)( (bounds[2*i+1] - B[2*i] + htol[i]) / H[i] );

            if( min[i] < 0 )
                min[i] = 0;

            if( max[i] >= ndivs )
                max[i] = ndivs-1;
        }

        // each octant in between min/max point may have cell in it
        for( int k=min[2]; k<=max[2]; k++ )
        {
            for( int j=min[1]; j<=max[1]; j++ )
            {
                for( int i=min[0]; i<=max[0]; i++ )
                {
                    vtkIdType idx = parentOffset + i + ndivs*(j + ndivs*k);

                    if( Tree[idx] == NULL )
                    {
                        Tree[idx] = vtkIdList::New();
                        Tree[idx]->Allocate( NumberOfCellsPerBucket, 
                                             NumberOfCellsPerBucket/2 );
                    }

                    Tree[idx]->InsertNextId(id);
                }
            }
        }
    }
}

//----------------------------------------------------------------------------

double Distance2ToBounds( const double x[3], double bounds[6] )
{
    double distance;
    double deltas[3];
    
    // Are we within the bounds?
    if (x[0] >= bounds[0] && x[0] <= bounds[1]
        && x[1] >= bounds[2] && x[1] <= bounds[3]
        && x[2] >= bounds[4] && x[2] <= bounds[5])
    {
        return 0.0;
    }
    
    deltas[0] = deltas[1] = deltas[2] = 0.0;
    
    // dx
    //
    if (x[0] < bounds[0])
    {
        deltas[0] = bounds[0] - x[0];
    }
    else if (x[0] > bounds[1])
    {
        deltas[0] = x[0] - bounds[1];
    }
    
    // dy
    //
    if (x[1] < bounds[2])
    {
        deltas[1] = bounds[2] - x[1];
    }
    else if (x[1] > bounds[3])
    {
        deltas[1] = x[1] - bounds[3];
    }
    
    // dz
    //
    if (x[2] < bounds[4])
    {
        deltas[2] = bounds[4] - x[2];
    }
    else if (x[2] > bounds[5])
    {
        deltas[2] = x[2] - bounds[5];
    }
    
    distance = deltas[0]*deltas[0] + deltas[1]*deltas[1] + deltas[2]*deltas[2];
    return distance;
}


vtkIdType avtCellLocatorClassic::FindCell( const double pos[3],
                                           avtInterpolationWeights* weights,
                                           bool ignoreGhostCells ) const
{
    int ijk[3];

    for( int j=0; j<3; j++ )
    {
        ijk[j] = (int)( (pos[j]-B[2*j]) / H[j] );

        if( ijk[j] < 0 )
            ijk[j] = 0;
        else if( ijk[j] >= (int)NumberOfDivisions )
            ijk[j] = NumberOfDivisions-1;
    }

    int leafStart = NumberOfOctants - NumberOfDivisions*NumberOfDivisions*NumberOfDivisions;

    vtkIdList* leafids = Tree[ leafStart + ijk[0] + ijk[1]*NumberOfDivisions +
                               ijk[2]*NumberOfDivisions*NumberOfDivisions ];

    if( leafids == NULL )
         return -1;

    for( int j=0; j < leafids->GetNumberOfIds(); j++ )
    {
        vtkIdType id = leafids->GetId( j );

        if( TestCell( id, pos, weights, ignoreGhostCells ) )
            return id;
    }

    return -1;
}
