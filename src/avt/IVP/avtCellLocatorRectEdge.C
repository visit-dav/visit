/*****************************************************************************
*
* Copyright (c) 2000 - 2015, Lawrence Livermore National Security, LLC
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

#include <avtCellLocatorRectEdge.h>

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

avtCellLocatorRectEdge::avtCellLocatorRectEdge( vtkDataSet* ds ) :
    avtCellLocatorRect( ds ),
    direction(0)
{
}

// ---------------------------------------------------------------------------

avtCellLocatorRectEdge::~avtCellLocatorRectEdge()
{
    Free();
}

// ---------------------------------------------------------------------------

void avtCellLocatorRectEdge::SetDirection(size_t dir)
{
    this->direction = dir;
}

// ---------------------------------------------------------------------------

void avtCellLocatorRectEdge::Build()
{
}

// ---------------------------------------------------------------------------

void avtCellLocatorRectEdge::Free()
{
}

vtkIdType
avtCellLocatorRectEdge::FindCell(const double pos[3],
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
            {
                return false;
            } 

            i[d] = 0;
            l[d] = 0.0;
        }
        else
        {
            // binary search
            std::vector<float>::const_iterator ci;
            if (ascending[d])
            {
                ci = std::lower_bound( coord[d].begin(), coord[d].end(), 
                                  pos[d], std::less<float>() );
            }
            else
            {
                ci = std::lower_bound( coord[d].begin(), coord[d].end(), 
                                  pos[d], std::greater<float>() );
            }
            
            if( ci == coord[d].end() )
            {
                return -1;
            }
            
            if( ci == coord[d].begin() )
            {
                if( ascending[d] )
                {
                    if (pos[d] < *ci )
                    {
                        return -1;
                    }
                }
                else
                {
                    if (pos[d] > *ci )
                    {
                        return -1;
                    }
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
    {
        return -1;
    }

    if( weights )
    {
        const float k[3] = { 1.0f-l[0], 1.0f-l[1], 1.0f-l[2] };

        vtkIdType base = (i[2]*coord[1].size() + i[1])*coord[0].size() + i[0];

        vtkIdType dx = i[0] ? 1 : 0;
        vtkIdType dy = i[1] ? coord[0].size() : 0;
        vtkIdType dz = i[2] ? coord[1].size()*coord[0].size() : 0;
        vtkIdType deltas[] = {dx, dy, dz};
        size_t dir1 = (this->direction + 1) % 3;
        size_t dir2 = (this->direction + 2) % 3;
        vtkIdType indxOffset1 = deltas[dir1];
        vtkIdType indxOffset2 = deltas[dir2];
        
        weights->resize(4);

        (*weights)[0].i = base;
        (*weights)[0].w = k[dir1] * k[dir2];

        (*weights)[1].i = base + indxOffset1;
        (*weights)[1].w = l[dir1] * k[dir2];
        
        (*weights)[2].i = base + indxOffset1 + indxOffset2;
        (*weights)[2].w = l[dir1] * l[dir2];
        
        (*weights)[3].i = base + indxOffset2;
        (*weights)[3].w = k[dir1] * l[dir2];

    }

    return cell;

}

// ---------------------------------------------------------------------------
