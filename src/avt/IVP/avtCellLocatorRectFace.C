// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#include <avtCellLocatorRectFace.h>

#include <vtkRectilinearGrid.h>
#include <vtkDataArray.h>
#include <vtkVoxel.h>
#include <vtkVisItUtility.h>
#include <DebugStream.h>
#include <cassert>
#include <algorithm>
#include <functional> // for 'less'

// -------------------------------------------------------------------------
//  Modifications:
// ---------------------------------------------------------------------------

avtCellLocatorRectFace::avtCellLocatorRectFace( vtkDataSet* ds ) :
    avtCellLocatorRect( ds ),
    direction(0)
{
}

// ---------------------------------------------------------------------------

avtCellLocatorRectFace::~avtCellLocatorRectFace()
{
    Free();
}

// ---------------------------------------------------------------------------

void avtCellLocatorRectFace::SetDirection(size_t dir)
{
    this->direction = dir;
}

// ---------------------------------------------------------------------------

void avtCellLocatorRectFace::Build()
{
}

// ---------------------------------------------------------------------------

void avtCellLocatorRectFace::Free()
{
}

vtkIdType
avtCellLocatorRectFace::FindCell(const double pos[3],
                             avtInterpolationWeights *weights,
                             bool ignoreGhostCells) const
{
  
    int    i[3];
    double l[3];

    for( unsigned int d=0; d<3; d++ )
    {
        if( coord[d].size() == 1 )
        {
            // flat grid
            if( pos[d] != coord[d].front() )
                return false;

            i[d] = 0;
            l[d] = 0.0;
        }
        else
        {
            // binary search
            std::vector<double>::const_iterator ci;
            if (ascending[d])
            {
                ci = std::lower_bound( coord[d].begin(), coord[d].end(), 
                                       pos[d], std::less<double>() );
            }
            else
            {
                ci = std::lower_bound( coord[d].begin(), coord[d].end(), 
                                       pos[d], std::greater<double>() );
            }
            
            if( ci == coord[d].end() )
                return -1;
            
            if( ci == coord[d].begin() )
            {
                if( ascending[d] )
                {
                    if (pos[d] < *ci )
                        return -1;
                }
                else
                {
                    if (pos[d] > *ci )
                        return -1;
                }
            }
            else 
                --ci;
            
            i[d] = ci - coord[d].begin(); 
            // This math works whether coord is monotonically increasing or
            // decreasing, since it just calculating a value in [0, 1] for
            // the distance between ci[0] and ci[1].
            l[d] = (pos[d] - ci[0])/(ci[1]-ci[0]);
        }
    }

    vtkIdType cell = (i[2]*(coord[1].size()-1) + i[1])*(coord[0].size()-1) + i[0];

    if( ignoreGhostCells && ghostPtr && ghostPtr[cell] )
        return -1;

    if( weights )
    {
        double k[3] = { 1.0-l[0], 1.0-l[1], 1.0-l[2] };

        vtkIdType base = (i[2]*coord[1].size() + i[1])*coord[0].size() + i[0];

        vtkIdType dx = (coord[0].size() > 1) ? 1 : 0;
        vtkIdType dy = (coord[1].size() > 1) ? coord[0].size() : 0;
        vtkIdType dz = (coord[2].size() > 1) ? coord[1].size()*coord[0].size() : 0;
        vtkIdType deltas[] = {dx, dy, dz};
        vtkIdType indxOffset = deltas[this->direction];

        weights->resize(2);

        (*weights)[0].i = base;
        (*weights)[0].w = k[this->direction];

        (*weights)[1].i = base + indxOffset;
        (*weights)[1].w = l[this->direction];        
    }

    return cell;

}

// ---------------------------------------------------------------------------
