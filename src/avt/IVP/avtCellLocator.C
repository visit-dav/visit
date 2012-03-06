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

#include <avtCellLocator.h>
#include <vtkDataSet.h>
#include <vtkGenericCell.h>
#include <vtkUnstructuredGrid.h>
#include <vtkStructuredGrid.h>
#include <vtkCellArray.h>
#include <vtkCellData.h>
#include <vtkFloatArray.h>
#include <vtkDoubleArray.h>

#include <DebugStream.h>
#include <VisItException.h>
#include <cstdlib>
#include <cmath>
#include <cassert>

#include <vtkMath.h>

//----------------------------------------------------------------------------
//  Modifications:
//
//    Hank Childs, Fri Oct 29 12:13:07 PDT 2010
//    Initialize new data members for curvilinear location.
//    Remove assertion for 2D curvilinear data.
//
//----------------------------------------------------------------------------

avtCellLocator::avtCellLocator( vtkDataSet* ds ) : dataSet(NULL)
{
    SetDataSet(ds);
}

// ****************************************************************************
//  Method: avtCellLocator::SetDataSet
//
//  Purpose:
//      Set the data set for this avtCellLocator
//
//  Programmer: David Camp
//  Creation:   April 21, 2011
//
//  Modifications:
//
//  David Camp, Tue Sep 13 08:16:35 PDT 2011
//  Needed to reset the pointer to the dataset
//
// ****************************************************************************

void
avtCellLocator::SetDataSet(vtkDataSet *ds)
{
    ReleaseDataSet();
    dataSet = ds;
    dataSet->Register( NULL );

    cellIdxPtr = NULL;
    cellLocPtr = NULL;
    strDimPtr  = NULL;
    normal2D = false;
    normal3D = false;

    if( vtkUnstructuredGrid* ug = vtkUnstructuredGrid::SafeDownCast( dataSet ) )
    {
        cellIdxPtr = ug->GetCells()->GetPointer();
        cellLocPtr = ug->GetCellLocationsArray()->GetPointer(0);
    }
    else if( vtkStructuredGrid* sg = vtkStructuredGrid::SafeDownCast( dataSet ) )
    {
        strDimPtr = sg->GetDimensions();
        if (strDimPtr[0] > 1 && strDimPtr[1] > 1 && strDimPtr[2] == 1)
            normal2D = true;
        else if (strDimPtr[0] > 1 && strDimPtr[1] > 1 && strDimPtr[2] > 1)
            normal3D = true;
    }

    fCoordPtr = NULL;
    dCoordPtr = NULL;

    if( vtkPointSet* ps = vtkPointSet::SafeDownCast( dataSet ) )
    {
        if( vtkPoints* points = ps->GetPoints() )
        {
            if( vtkFloatArray* fc = vtkFloatArray::SafeDownCast( points->GetData() ) )
            {
                if( fc->GetNumberOfComponents() == 3 )
                    fCoordPtr = fc->GetPointer(0);
            }
            else if( vtkDoubleArray* dc = vtkDoubleArray::SafeDownCast( points->GetData() ) )
            {
                if( dc->GetNumberOfComponents() == 3 )
                    dCoordPtr = dc->GetPointer(0);
            }
        }
    }
}

//----------------------------------------------------------------------------

avtCellLocator::~avtCellLocator()
{
    if(dataSet)
        dataSet->Delete();
}

// ****************************************************************************
//  Method: avtCellLocator::ReleaseDataSet
//
//  Purpose:
//      Release data set. This is needed for the load on demand to release 
//    the data and it will be resigned if we load it again.
//
//  Programmer: David Camp
//  Creation:   April 21, 2011
//
// ****************************************************************************

void
avtCellLocator::ReleaseDataSet()
{
    if(dataSet)
    {
        dataSet->Delete();
        dataSet = NULL;

        cellIdxPtr = NULL;
        cellLocPtr = NULL;
        strDimPtr  = NULL;
        normal2D = false;
        normal3D = false;

        fCoordPtr = NULL;
        dCoordPtr = NULL;
    }
}

// ****************************************************************************
//  Method: avtCellLocator::Destruct
//
//  Purpose:
//      A routine that a void_ref_ptr can call to cleanly destruct a cell 
//      locator.
//
//  Programmer: Hank Childs
//  Creation:   November 28, 2010
//
// ****************************************************************************

void
avtCellLocator::Destruct(void *p)
{
    avtCellLocator *cl = (avtCellLocator *) p;
    delete cl;
}

//----------------------------------------------------------------------------
// Modifications:
//   Kathleen Bonnell, Fri Aug 13 08:20:15 MST 2010
//   Fix compile error on windows. When creating an array like: a[npts], npts
//   must be a constant whose value can be determined at compile time, or
//   VisualStudio complains.
//
//   Hank Childs, Fri Nov 19 14:45:53 PST 2010
//   Added TestVoxel.
//
//----------------------------------------------------------------------------

bool avtCellLocator::TestCell( vtkIdType cellid, const double pos[3],
                               avtInterpolationWeights* weights ) const
{
    // check if we can take a fast path
    switch( dataSet->GetCellType( cellid ) )
    {
    case VTK_TETRA:
        return TestTet( cellid, pos, weights );
    case VTK_HEXAHEDRON:
        return TestHex( cellid, pos, weights );
    case VTK_WEDGE:
        return TestPrism( cellid, pos, weights );
    case VTK_VOXEL:
        return TestVoxel( cellid, pos, weights );
    default:
        break;
    }

    // no fast path, do it the circuitous and slow way
    double bounds[6];
    dataSet->GetCellBounds( cellid, bounds );

    if( pos[0]<bounds[0] || pos[0]>bounds[1] ||
        pos[1]<bounds[2] || pos[1]>bounds[3] ||
        pos[2]<bounds[4] || pos[2]>bounds[5] )
        return false;

    vtkGenericCell* cell = vtkGenericCell::New();
    dataSet->GetCell( cellid, cell );

    // make sure we have space for the per-point indices + weights
    const unsigned int npts = cell->GetNumberOfPoints();

    // perform the cell inversion
    double lcoord[3], tmp;
    std::vector<double> w(npts);
    int subid;

    int result = cell->EvaluatePosition( const_cast<double*>(pos), 0, 
                                         subid, lcoord, tmp, &w[0]);

    // if this is the cell containing pos, compute indices and derivatives
    if( result == 1 && weights )
    {
        weights->resize( npts );

        for( unsigned int i=0; i<npts; ++i )
        {
            (*weights)[i].i    = cell->GetPointId( i );
            (*weights)[i].w    = w[i];
        }
    }

    cell->Delete();
    return result == 1;
}

// ---------------------------------------------------------------------------
//  Modifications:
//
//    Hank Childs, Fri Oct 29 12:13:07 PDT 2010
//    Add support for 2D curvilinear.
//
// ---------------------------------------------------------------------------

void avtCellLocator::CopyCell( vtkIdType cellid, vtkIdType* ids, 
                               double pts[][3] ) const
{
    unsigned int npts = 0;

    // copy cell indices

    if( cellLocPtr )
    {
        vtkIdType* direct = cellIdxPtr + cellLocPtr[cellid];

        npts = *(direct++);

        for( unsigned int i=0; i<npts; ++i )
            ids[i] = direct[i];
    }
    else if( strDimPtr )
    {
        if (normal3D)
        {
            int i = cellid % (strDimPtr[0] - 1);
            int j = (cellid / (strDimPtr[0] - 1)) % (strDimPtr[1] - 1);
            int k = cellid / ((strDimPtr[0] - 1) * (strDimPtr[1] - 1));

            int idx = i + j*strDimPtr[0] + k*strDimPtr[0]*strDimPtr[1];
            int d0 = strDimPtr[0];
            int d1 = strDimPtr[0]*strDimPtr[1];
    
            ids[0] = idx;
            ids[1] = idx+1;
            ids[2] = idx+1+d0;
            ids[3] = idx+d0;
            idx += d1;
            ids[4] = idx;
            ids[5] = idx+1;
            ids[6] = idx+1+d0;
            ids[7] = idx+d0;
    
            npts = 8;
        }
        else if (normal2D)
        {
            int i = cellid % (strDimPtr[0] - 1);
            int j = cellid / (strDimPtr[0] - 1);

            int idx = i + j*strDimPtr[0];
            int d0 = strDimPtr[0];
            ids[0] = idx;
            ids[1] = idx+1;
            ids[2] = idx+1+d0;
            ids[3] = idx+d0;
            npts = 4;
        }
        else
        {
            int idx, d0;
            if (strDimPtr[0] == 1 && strDimPtr[1] > 1 && strDimPtr[2] > 1)
            {
                int j = cellid % (strDimPtr[1] - 1);
                int k = cellid / (strDimPtr[1] - 1);
        
                idx = j + k*strDimPtr[1];
                d0 = strDimPtr[1];
            }
            else if (strDimPtr[0] > 1 && strDimPtr[1] == 1 && strDimPtr[2] > 1)
            {
                int i = cellid % (strDimPtr[0] - 1);
                int k = cellid / (strDimPtr[0] - 1);
        
                idx = i + k*strDimPtr[0];
                d0 = strDimPtr[0];
            }
            else
            {
                EXCEPTION1(VisItException, "Unable to evaluate cells for particle advection");
            }
            ids[0] = idx;
            ids[1] = idx+1;
            ids[2] = idx+1+d0;
            ids[3] = idx+d0;
            npts = 4;
        }
    }
    else
    {
        vtkIdList* tmp = vtkIdList::New();
        dataSet->GetCellPoints( cellid, tmp );
        
        npts = tmp->GetNumberOfIds();

        for( int i=0; i<npts; ++i )
            ids[i] = tmp->GetId(i);

        tmp->Delete();
    }

    // copy points and compute bounding box on the fly

    if( fCoordPtr )
    {
        for( unsigned int i=0; i<npts; ++i )
            for( unsigned int j=0; j<3; ++j )
                pts[i][j] = fCoordPtr[3*ids[i]+j];
    }
    else if( dCoordPtr )
    {
        for( unsigned int i=0; i<npts; ++i )
            for( unsigned int j=0; j<3; ++j )
                pts[i][j] = dCoordPtr[3*ids[i]+j];
    }
    else
    {
        for( int i=0; i<npts; ++i )
            dataSet->GetPoint( ids[i], pts[i] );
    }
}

bool avtCellLocator::TestVoxel( vtkIdType cellid, const double pos[3],
                              avtInterpolationWeights* weights ) const
{
    vtkIdType ids[8];
    double pts[8][3];

    CopyCell( cellid, ids, pts );
    
    // bounding box test

    if (pos[0] < pts[0][0] || pos[0] > pts[7][0])
        return false;
    if (pos[1] < pts[0][1] || pos[1] > pts[7][1])
        return false;
    if (pos[2] < pts[0][2] || pos[2] > pts[7][2])
        return false;

    double xl = pts[7][0]-pts[0][0];
    double xp = 0.;
    if (xl > 0.)
        xp = (pos[0]-pts[0][0]) / xl;
    double yl = pts[7][1]-pts[0][1];
    double yp = 0.;
    if (yl > 0.)
        yp = (pos[1]-pts[0][1]) / yl;
    double zl = pts[7][2]-pts[0][2];
    double zp = 0.;
    if (zl > 0.)
        zp = (pos[2]-pts[0][2]) / zl;
    double c[3] = { xp, yp, zp };

    if( weights )
    {
        weights->resize( 8 );

        const double d[3] = { 1.0-c[0], 1.0-c[1], 1.0-c[2] };

        for( unsigned int i=0; i<8; ++i )
            (*weights)[i].i = ids[i];

        (*weights)[0].w = d[0]*d[1]*d[2];
        (*weights)[1].w = c[0]*d[1]*d[2];
        (*weights)[2].w = d[0]*c[1]*d[2];
        (*weights)[3].w = c[0]*c[1]*d[2];
        (*weights)[4].w = d[0]*d[1]*c[2];
        (*weights)[5].w = c[0]*d[1]*c[2];
        (*weights)[6].w = d[0]*c[1]*c[2];
        (*weights)[7].w = c[0]*c[1]*c[2];
    }

    return true;
}

// ---------------------------------------------------------------------------

bool avtCellLocator::TestHex( vtkIdType cellid, const double pos[3],
                              avtInterpolationWeights* weights ) const
{
    vtkIdType ids[8];
    double pts[8][3];

    CopyCell( cellid, ids, pts );

    // bounding box test

    for( unsigned int j=0; j<3; ++j )
    {
        if( (pos[j] < pts[0][j] && pos[j] < pts[1][j] && pos[j] < pts[2][j] && pos[j] < pts[3][j] &&
             pos[j] < pts[4][j] && pos[j] < pts[5][j] && pos[j] < pts[6][j] && pos[j] < pts[7][j]) ||
            (pos[j] > pts[0][j] && pos[j] > pts[1][j] && pos[j] > pts[2][j] && pos[j] > pts[3][j] &&
             pos[j] > pts[4][j] && pos[j] > pts[5][j] && pos[j] > pts[6][j] && pos[j] > pts[7][j]) )
            return false;
    }

    // perform Newton iteration in local coordinates c
    const int    maxiter = 8;
    const double epsilon = 1e-4;

    double h[3], D[3][3], p[3], c[3];

    for( int i=0; i<3; ++i )
        c[i] = 0.5;

    for( int iter=0; iter<maxiter; ++iter )
    {
        const double d[3] = { 1.0-c[0], 1.0-c[1], 1.0-c[2] };

        // Note: Formulas below are arranged as a good compromise between numerical
        // stability and speed. Rearrange at your own peril.
        for( int i=0; i<3; ++i )
        {
            p[i] = pts[0][i]*d[0]*d[1]*d[2] + pts[1][i]*c[0]*d[1]*d[2] +
                   pts[2][i]*c[0]*c[1]*d[2] + pts[3][i]*d[0]*c[1]*d[2] +
                   pts[4][i]*d[0]*d[1]*c[2] + pts[5][i]*c[0]*d[1]*c[2] +
                   pts[6][i]*c[0]*c[1]*c[2] + pts[7][i]*d[0]*c[1]*c[2] - pos[i];

            D[0][i] = -pts[0][i]*d[1]*d[2] + pts[1][i]*d[1]*d[2]
                      +pts[2][i]*c[1]*d[2] - pts[3][i]*c[1]*d[2]
                      -pts[4][i]*d[1]*c[2] + pts[5][i]*d[1]*c[2]
                      +pts[6][i]*c[1]*c[2] - pts[7][i]*c[1]*c[2];

            D[1][i] = -pts[0][i]*d[0]*d[2] - pts[1][i]*c[0]*d[2]
                      +pts[2][i]*c[0]*d[2] + pts[3][i]*d[0]*d[2]
                      -pts[4][i]*d[0]*c[2] - pts[5][i]*c[0]*c[2]
                      +pts[6][i]*c[0]*c[2] + pts[7][i]*d[0]*c[2];

            D[2][i] = -pts[0][i]*d[0]*d[1] - pts[1][i]*c[0]*d[1]
                      -pts[2][i]*c[0]*c[1] - pts[3][i]*d[0]*c[1]
                      +pts[4][i]*d[0]*d[1] + pts[5][i]*c[0]*d[1]
                      +pts[6][i]*c[0]*c[1] + pts[7][i]*d[0]*c[1];
        }

        // update local coordinates by solving the linear system using
        // Cramer's method
        double denom = D[0][0]*(D[1][1]*D[2][2] - D[2][1]*D[1][2]) +
                       D[1][0]*(D[2][1]*D[0][2] - D[0][1]*D[2][2]) +
                       D[2][0]*(D[0][1]*D[1][2] - D[1][1]*D[0][2]);

        c[0] -= (h[0] = ( p[0]*(D[1][1]*D[2][2] - D[2][1]*D[1][2]) +
                          D[1][0]*(D[2][1]*p[2] - p[1]*D[2][2]) +
                          D[2][0]*(p[1]*D[1][2] - D[1][1]*p[2]) ) / denom );

        c[1] -= (h[1] = ( D[0][0]*(p[1]*D[2][2] - D[2][1]*p[2]) +
                          p[0]*(D[2][1]*D[0][2] - D[0][1]*D[2][2]) +
                          D[2][0]*(D[0][1]*p[2] - p[1]*D[0][2]) ) / denom );

        c[2] -= (h[2] = ( D[0][0]*(D[1][1]*p[2] - p[1]*D[1][2]) +
                          D[1][0]*(p[1]*D[0][2] - D[0][1]*p[2]) +
                          p[0]*(D[0][1]*D[1][2] - D[1][1]*D[0][2]) ) / denom );

        // if update is small enough, exit early
        if( std::abs(h[0])<epsilon && std::abs(h[1])<epsilon && std::abs(h[2])<epsilon )
            break;
    }

    if( c[0] < -0.001 || c[1] < -0.001 || c[2] < -0.001 ||
        c[0] >  1.001 || c[1] >  1.001 || c[2] >  1.001 )
        return false;

    if( weights )
    {
        weights->resize( 8 );

        const double d[3] = { 1.0-c[0], 1.0-c[1], 1.0-c[2] };

        for( unsigned int i=0; i<8; ++i )
            (*weights)[i].i = ids[i];

        (*weights)[0].w = d[0]*d[1]*d[2];
        (*weights)[1].w = c[0]*d[1]*d[2];
        (*weights)[2].w = c[0]*c[1]*d[2];
        (*weights)[3].w = d[0]*c[1]*d[2];
        (*weights)[4].w = d[0]*d[1]*c[2];
        (*weights)[5].w = c[0]*d[1]*c[2];
        (*weights)[6].w = c[0]*c[1]*c[2];
        (*weights)[7].w = d[0]*c[1]*c[2];
    }

    return true;
}

// ---------------------------------------------------------------------------

bool avtCellLocator::TestPrism( vtkIdType cellid, const double pos[3],
                                avtInterpolationWeights* weights ) const
{
    vtkIdType ids[6];
    double pts[6][3];

    CopyCell( cellid, ids, pts );

    // bounding box test

    for( unsigned int j=0; j<3; ++j )
    {
        if( (pos[j] < pts[0][j] && pos[j] < pts[1][j] && pos[j] < pts[2][j] && pos[j] < pts[3][j] &&
             pos[j] < pts[4][j] && pos[j] < pts[5][j]) ||
            (pos[j] > pts[0][j] && pos[j] > pts[1][j] && pos[j] > pts[2][j] && pos[j] > pts[3][j] &&
             pos[j] > pts[4][j] && pos[j] > pts[5][j]) )
            return false;
    }


    // perform Newton iteration in local coordinates c
    const int    maxiter = 8;
    const double epsilon = 1e-4;

    double h[3], D[3][3], p[3], c[3] = { 0.333, 0.333, 0.5 };

    for( int iter=0; iter<maxiter; ++iter )
    {
        const double d[2] = { 1.0-c[0]-c[1], 1.0-c[2] };

        // Note: Formulas below are arranged as a good compromise between numerical
        // stability and speed. Rearrange at your own peril.
        for( int i=0; i<3; ++i )
        {
            p[i] = pts[0][i]*d[0]*d[1] +
                   pts[1][i]*c[0]*d[1] +
                   pts[2][i]*c[1]*d[1] +
                   pts[3][i]*d[0]*c[2] +
                   pts[4][i]*c[0]*c[2] +
                   pts[5][i]*c[1]*c[2] - pos[i];

            D[0][i] = d[1]*( pts[1][i] - pts[0][i] ) + 
                      c[2]*( pts[4][i] - pts[3][i] );

            D[1][i] = d[1]*( pts[2][i] - pts[0][i] ) +
                      c[2]*( pts[5][i] - pts[3][i] );

            D[2][i] = d[0]*( pts[3][i] - pts[0][i] ) +
                      c[0]*( pts[4][i] - pts[1][i] ) +
                      c[1]*( pts[5][i] - pts[2][i] );
        }

        // update local coordinates by solving the linear system using
        // Cramer's method
        double denom = D[0][0]*(D[1][1]*D[2][2] - D[2][1]*D[1][2]) +
                       D[1][0]*(D[2][1]*D[0][2] - D[0][1]*D[2][2]) +
                       D[2][0]*(D[0][1]*D[1][2] - D[1][1]*D[0][2]);

        c[0] -= (h[0] = ( p[0]*(D[1][1]*D[2][2] - D[2][1]*D[1][2]) +
                          D[1][0]*(D[2][1]*p[2] - p[1]*D[2][2]) +
                          D[2][0]*(p[1]*D[1][2] - D[1][1]*p[2]) ) / denom );

        c[1] -= (h[1] = ( D[0][0]*(p[1]*D[2][2] - D[2][1]*p[2]) +
                          p[0]*(D[2][1]*D[0][2] - D[0][1]*D[2][2]) +
                          D[2][0]*(D[0][1]*p[2] - p[1]*D[0][2]) ) / denom );

        c[2] -= (h[2] = ( D[0][0]*(D[1][1]*p[2] - p[1]*D[1][2]) +
                          D[1][0]*(p[1]*D[0][2] - D[0][1]*p[2]) +
                          p[0]*(D[0][1]*D[1][2] - D[1][1]*D[0][2]) ) / denom );

        // if update is small enough, exit early
        if( std::abs(h[0])<epsilon && std::abs(h[1])<epsilon && std::abs(h[2])<epsilon )
            break;
    }

    if( c[0] < -0.001 || c[1] < -0.001 || c[2] < -0.001 ||
        c[0] >  1.001 || c[1] >  1.001 || c[2] >  1.001 )
        return false;

    if( weights )
    {
        weights->resize( 6 );

        for( unsigned int i=0; i<6; ++i )
            (*weights)[i].i = ids[i];

        const double d[2] = { 1.0-c[0]-c[1], 1.0-c[2] };

        (*weights)[0].w = d[0]*d[1];
        (*weights)[1].w = c[0]*d[1];
        (*weights)[2].w = c[1]*d[1];
        (*weights)[3].w = d[0]*c[2];
        (*weights)[4].w = c[0]*c[2];
        (*weights)[5].w = c[1]*c[2];
    }

    return true;
}

// --------------------------------------------------------------------------

inline static double inv3( const double A[4][3], 
                           const double b[3],
                           double r[3] )
{
    double tmp[3], vol;

    tmp[0] = A[1][1]*A[2][2] - A[1][2]*A[2][1];
    tmp[1] = A[1][2]*A[2][0] - A[1][0]*A[2][2];
    tmp[2] = A[1][0]*A[2][1] - A[1][1]*A[2][0];

    vol  = A[0][0]*tmp[0] + A[0][1]*tmp[1] + A[0][2]*tmp[2];
    r[0] = A[3][0]*tmp[0] + A[3][1]*tmp[1] + A[3][2]*tmp[2];

    tmp[0] = A[0][1]*A[2][2] - A[0][2]*A[2][1];
    tmp[1] = A[0][2]*A[2][0] - A[0][0]*A[2][2];
    tmp[2] = A[0][0]*A[2][1] - A[0][1]*A[2][0];

    r[1] = -(A[3][0]*tmp[0] + A[3][1]*tmp[1] + A[3][2]*tmp[2]);

    tmp[0] = A[0][1]*A[1][2] - A[0][2]*A[1][1];
    tmp[1] = A[0][2]*A[1][0] - A[0][0]*A[1][2];
    tmp[2] = A[0][0]*A[1][1] - A[0][1]*A[1][0];

    r[2] = A[3][0]*tmp[0] + A[3][1]*tmp[1] + A[3][2]*tmp[2];

    return vol;
}


bool avtCellLocator::TestTet( vtkIdType cellid, const double pos[3],
                              avtInterpolationWeights* weights ) const
{
    const double epsilon = 1e-5;
        
    vtkIdType ids[4];
    double pts[4][3];

    CopyCell( cellid, ids, pts );

    // bounding box test
    for( unsigned int j=0; j<3; ++j )
    {
        if( (pos[j] < pts[0][j] && pos[j] < pts[1][j] && pos[j] < pts[2][j] && pos[j] < pts[3][j]) ||
            (pos[j] > pts[0][j] && pos[j] > pts[1][j] && pos[j] > pts[2][j] && pos[j] > pts[3][j]) )
            return false;
    }

    // solve the linear system for the tet basis coefficients
    for( int j=0; j<3; ++j )
    {
        pts[j][0] -= pts[3][0];
        pts[j][1] -= pts[3][1];
        pts[j][2] -= pts[3][2];
    }

    pts[3][0] = pos[0] - pts[3][0];
    pts[3][1] = pos[1] - pts[3][1];
    pts[3][2] = pos[2] - pts[3][2];

    double c[4], vol;

    vol = inv3( pts, pts[3], c );
    c[3] = vol - c[0] - c[1] - c[2];

    for( int i=0; i<4; ++i )
        c[i] /= vol;

    // coefficients inside?
    if( c[0] < -epsilon || c[1] < -epsilon || 
        c[2] < -epsilon || c[3] < -epsilon )
        return false;

    // fill in weights
    if( weights )
    {
        weights->resize( 4 );

        for( unsigned int i=0; i<4; ++i )
        {
            (*weights)[i].i = ids[i];
            (*weights)[i].w = c[i];
        }
    }

    return true;
}

