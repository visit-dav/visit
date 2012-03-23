/*****************************************************************************
*
* Copyright (c) 2000 - 2012, Lawrence Livermore National Security, LLC
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

#include <avtCellLocatorRect.h>

#include <vtkRectilinearGrid.h>
#include <vtkDataArray.h>
#include <vtkVoxel.h>
#include <vtkVisItUtility.h>
#include <DebugStream.h>
#include <cassert>
#include <algorithm>
#include <functional> // for 'less'

// -------------------------------------------------------------------------

avtCellLocatorRect::avtCellLocatorRect( vtkDataSet* ds ) :
    avtCellLocator( ds )
{
    vtkRectilinearGrid* rg = vtkRectilinearGrid::SafeDownCast( dataSet );

    if( dataSet->GetDataObjectType() != VTK_RECTILINEAR_GRID || rg == NULL )
        EXCEPTION1( ImproperUseException, "avtCellLocatorRect: Dataset is not rectilinear." );

    // copy the coordinate arrays for faster access later
    vtkDataArray* ca[3] = { 
        rg->GetXCoordinates(),
        rg->GetYCoordinates(),
        rg->GetZCoordinates(),
    };

    for( unsigned int d=0; d<3; ++d )
    {
        coord[d].resize( ca[d]->GetNumberOfTuples() );

        for( unsigned int i=0; i<coord[d].size(); ++i )
            coord[d][i] = ca[d]->GetComponent( i, 0 );
    }    
}

// ---------------------------------------------------------------------------

avtCellLocatorRect::~avtCellLocatorRect()
{
    Free();
}

// ---------------------------------------------------------------------------

void avtCellLocatorRect::Build()
{
}

// ---------------------------------------------------------------------------

void avtCellLocatorRect::Free()
{
}

// ---------------------------------------------------------------------------

vtkIdType avtCellLocatorRect::FindCell( const double pos[3],
                                        avtInterpolationWeights* weights,
                                        bool ignoreGhostCells ) const
{
#if 0

    vtkRectilinearGrid* rg = (vtkRectilinearGrid*)dataSet;
    int ijk[3];

    if( vtkVisItUtility::ComputeStructuredCoordinates(rg, (double*)pos, ijk) == 0 )
        return -1;

    vtkIdType cell = rg->ComputeCellId( ijk );
    
    if( cell < 0 )
        return -1;

    return TestCell( cell, pos, weights ) ? cell : -1;

#else

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
            std::vector<float>::const_iterator ci = 
                std::lower_bound( coord[d].begin(), coord[d].end(), 
                                  pos[d], std::less<float>() );
            
            if( ci == coord[d].end() )
                return -1;
            
            if( ci == coord[d].begin() )
            {
                if( pos[d] < *ci )
                    return -1;
            }
            else 
                --ci;
            
            i[d] = ci - coord[d].begin(); 
            l[d] = (pos[d] - ci[0])/(ci[1]-ci[0]);
        }
    }

    vtkIdType cell = (i[2]*(coord[1].size()-1) + i[1])*(coord[0].size()-1) + i[0];

    if( ignoreGhostCells && ghostPtr && ghostPtr[cell] )
        return -1;

    if( weights )
    {
        const float k[3] = { 1.0f-l[0], 1.0f-l[1], 1.0f-l[2] };

        vtkIdType base = (i[2]*coord[1].size() + i[1])*coord[0].size() + i[0];

        vtkIdType dx = i[0] ? 1 : 0;
        vtkIdType dy = i[1] ? coord[0].size() : 0;
        vtkIdType dz = i[2] ? coord[1].size()*coord[0].size() : 0;

        weights->resize( 8 );

        (*weights)[0].i = base;
        (*weights)[0].w = k[0]*k[1]*k[2];
     
        (*weights)[1].i = base+dx;
        (*weights)[1].w = l[0]*k[1]*k[2];

        (*weights)[2].i = base+dy;
        (*weights)[2].w = k[0]*l[1]*k[2];

        (*weights)[3].i = base+dx+dy;
        (*weights)[3].w = l[0]*l[1]*k[2];

        (*weights)[4].i = base+dz;
        (*weights)[4].w = k[0]*k[1]*l[2];

        (*weights)[5].i = base+dz+dx;
        (*weights)[5].w = l[0]*k[1]*l[2];

        (*weights)[6].i = base+dz+dy;
        (*weights)[6].w = k[0]*l[1]*l[2];

        (*weights)[7].i = base+dx+dy+dz;
        (*weights)[7].w = l[0]*l[1]*l[2];
    }

    return cell;

#endif
}

// ---------------------------------------------------------------------------
