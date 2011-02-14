/*****************************************************************************
*
* Copyright (c) 2000 - 2011, Lawrence Livermore National Security, LLC
* Produced at the Lawrence Livermore National Laboratory
* LLNL-CODE-442911
* All rights reserved.
*
* This file is  part of VisIt. For  details, see https://visit.llnl.gov/.  The
* full copyright notice is contained in the file COPYRIGHT located at the root
* of the VisIt distribution or at http://www.llnl.gov/visit/copyright.html.
*
* Redistribution  and  use  in  source  and  binary  forms,  with  or  without
* modification, are permitted provided that the following conditions are met:
*
*  - Redistributions of  source code must  retain the above  copyright notice,
*    this list of conditions and the disclaimer below.
*  - Redistributions in binary form must reproduce the above copyright notice,
*    this  list of  conditions  and  the  disclaimer (as noted below)  in  the
*    documentation and/or other materials provided with the distribution.
*  - Neither the name of  the LLNS/LLNL nor the names of  its contributors may
*    be used to endorse or promote products derived from this software without
*    specific prior written permission.
*
* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT  HOLDERS AND CONTRIBUTORS "AS IS"
* AND ANY EXPRESS OR  IMPLIED WARRANTIES, INCLUDING,  BUT NOT  LIMITED TO, THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND  FITNESS FOR A PARTICULAR  PURPOSE
* ARE  DISCLAIMED. IN  NO EVENT  SHALL LAWRENCE  LIVERMORE NATIONAL  SECURITY,
* LLC, THE  U.S.  DEPARTMENT OF  ENERGY  OR  CONTRIBUTORS BE  LIABLE  FOR  ANY
* DIRECT,  INDIRECT,   INCIDENTAL,   SPECIAL,   EXEMPLARY,  OR   CONSEQUENTIAL
* DAMAGES (INCLUDING, BUT NOT  LIMITED TO, PROCUREMENT OF  SUBSTITUTE GOODS OR
* SERVICES; LOSS OF  USE, DATA, OR PROFITS; OR  BUSINESS INTERRUPTION) HOWEVER
* CAUSED  AND  ON  ANY  THEORY  OF  LIABILITY,  WHETHER  IN  CONTRACT,  STRICT
* LIABILITY, OR TORT  (INCLUDING NEGLIGENCE OR OTHERWISE)  ARISING IN ANY  WAY
* OUT OF THE  USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH
* DAMAGE.
*
*****************************************************************************/

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
        debug5 << "avtCellLocatorClassic::Build(): no cells to subdivide\n";
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
                                           avtInterpolationWeights* weights ) const
{
    int ijk[3];

    for( int j=0; j<3; j++ )
    {
        ijk[j] = (int)( (pos[j]-B[2*j]) / H[j] );

        if( ijk[j] < 0 )
            ijk[j] = 0;
        else if( ijk[j] >= NumberOfDivisions )
            ijk[j] = NumberOfDivisions-1;
    }

    int leafStart = NumberOfOctants - NumberOfDivisions*NumberOfDivisions*NumberOfDivisions;

    vtkIdList* leafids = Tree[ leafStart + ijk[0] + ijk[1]*NumberOfDivisions +
                               ijk[2]*NumberOfDivisions*NumberOfDivisions ];

    if( leafids == NULL )
         return -1;

    unsigned int ntest = 0;

    for( int j=0; j < leafids->GetNumberOfIds(); j++ )
    {
        vtkIdType id = leafids->GetId( j );

        if( TestCell( id, pos, weights ) )
            return id;
    }

    return -1;
}
