/*****************************************************************************
*
* Copyright (c) 2000 - 2008, Lawrence Livermore National Security, LLC
* Produced at the Lawrence Livermore National Laboratory
* LLNL-CODE-400142
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

// ************************************************************************* //
//                               avtSideVolume.C                             //
// ************************************************************************* //

#include <avtSideVolume.h>

#include <float.h>

#include <vtkCell.h>
#include <vtkDataSet.h>
#include <vtkFloatArray.h>
#include <vtkPoints.h>

#include <avtCallback.h>

#include <InvalidDimensionsException.h>


// ****************************************************************************
//  Method: avtSideVolume constructor
//
//  Programmer: Hank Childs
//  Creation:   January 20, 2005
//
//  Modifications:
//
//    Hank Childs, Thu Sep 22 15:39:11 PDT 2005
//    Initialize takeMin.
//
// ****************************************************************************

avtSideVolume::avtSideVolume()
{
    haveIssuedWarning = false;
    takeMin = true;
}


// ****************************************************************************
//  Method: avtSideVolume::PreExecute
//
//  Purpose:
//      Sets up a data member that ensures that we don't issue multiple
//      warnings.
//
//  Programmer: Hank Childs
//  Creation:   January 20, 2005
//
// ****************************************************************************

void
avtSideVolume::PreExecute(void)
{
    avtSingleInputExpressionFilter::PreExecute();

    avtDataAttributes &atts = GetInput()->GetInfo().GetAttributes();
    if (atts.GetTopologicalDimension() != 3)
    {
        EXCEPTION2(InvalidDimensionsException, "Side volume",
                                               "3-dimensional");
    }
    haveIssuedWarning = false;
}


// ****************************************************************************
//  Method: avtSideVolume::DeriveVariable
//
//  Purpose:
//      Calculate each of the side volumes and assign the output to have the
//      smallest (or biggest) of each of its sides (for each zone).
//
//  Programmer: Hank Childs
//  Creation:   January 20, 2005
//
// ****************************************************************************

vtkDataArray *
avtSideVolume::DeriveVariable(vtkDataSet *in_ds)
{
    vtkFloatArray *arr = vtkFloatArray::New();
    int ncells = in_ds->GetNumberOfCells();
    arr->SetNumberOfTuples(ncells);

    for (int i = 0 ; i < ncells ; i++)
    {
        vtkCell *cell = in_ds->GetCell(i);
        float vol = (float) GetZoneVolume(cell);
        arr->SetTuple(i, &vol);
    }

    return arr;
}


// ****************************************************************************
//  Method: avtSideVolume::GetZoneVolume
//
//  Purpose:
//      Break the zone into sides and return the volume of the smallest 
//      (or biggest) side.
//
//  Arguments:
//      cell    The input zone.
//
//  Returns:    The side volume of the zone.
//
//  Programmer: Hank Childs
//  Creation:   January 20, 2005
//
//  Modifications:
//    Brad Whitlock, Thu Feb 24 16:16:05 PST 2005
//    Fixed i redefinition for win32.
//
//    Hank Childs, Thu Sep 22 15:39:11 PDT 2005
//    Account for max variant as well.
//
// ****************************************************************************
 
double
avtSideVolume::GetZoneVolume(vtkCell *cell)
{
    int  i, j;

    if (cell->GetCellDimension() != 3)
    {
        if (!haveIssuedWarning)
        {
           char msg[1024];
           sprintf(msg, "Side volumes can only be calculated for three "
                        "dimensional cells.  You have a cell of topological "
                        "dimension %d.  Assigning cells of this type side "
                        "volume 0.", cell->GetCellDimension());
           avtCallback::IssueWarning(msg);
        }
        haveIssuedWarning = true;
        return 0.;
    }

    //
    // First, calculate the center of each face.
    //
    int nFaces = cell->GetNumberOfFaces();
    double *face_cent = new double[3*nFaces];
    for (i = 0 ; i < nFaces ; i++)
    {
        vtkCell *face = cell->GetFace(i);
        face_cent[3*i]   = 0.;
        face_cent[3*i+1] = 0.;
        face_cent[3*i+2] = 0.;
        vtkPoints *pts = face->GetPoints();
        float *pts_ptr = (float *) pts->GetVoidPointer(0);
        int npts = face->GetNumberOfPoints();
        for (j = 0 ; j < npts ; j++)
        {
            face_cent[3*i]   += pts_ptr[3*j];
            face_cent[3*i+1] += pts_ptr[3*j+1];
            face_cent[3*i+2] += pts_ptr[3*j+2];
        }
        if (npts != 0)
        {
            face_cent[3*i]   /= npts;
            face_cent[3*i+1] /= npts;
            face_cent[3*i+2] /= npts;
        }
    }

    //
    // Now we can calculate the zone center from the face center.
    //
    double zone_cent[3] = { 0., 0., 0. };
    for (i = 0 ; i < nFaces ; i++)
    {
        zone_cent[0] += face_cent[3*i];
        zone_cent[1] += face_cent[3*i+1];
        zone_cent[2] += face_cent[3*i+2];
    }
    if (nFaces != 0)
    {
        zone_cent[0] /= nFaces;
        zone_cent[1] /= nFaces;
        zone_cent[2] /= nFaces;
    }
 
    //
    // Now that we have all of the centers calculated, lets calculate the
    // volume of each side.  Since we can only access certain data through 
    // VTK's vtkCell interface, the easiest way to get the sides is to iterate 
    // through the faces and consider each of the sides generated by the face's
    // edges.
    //
    double rv = +FLT_MAX;
    if (!takeMin)
        rv = -FLT_MAX;
    for (i = 0 ; i < nFaces ; i++)
    {
        vtkCell *face = cell->GetFace(i);
        vtkPoints *pts = face->GetPoints();
        float *pts_ptr = (float *) pts->GetVoidPointer(0);
        int npts = face->GetNumberOfPoints();
        for (j = 0 ; j < npts ; j++)
        {
            int id2 = j;
            int id1 = (j+1) % npts;

            //
            // If we represent the tetrahedron as three edge vectors, a, b,
            // and c, the volume is 1/6*|dot(a,cross(b,c))|.
            //
            double a[3];
            a[0] = pts_ptr[3*id2] - pts_ptr[3*id1];
            a[1] = pts_ptr[3*id2+1] - pts_ptr[3*id1+1];
            a[2] = pts_ptr[3*id2+2] - pts_ptr[3*id1+2];
            double b[3];
            b[0] = face_cent[3*i]   - pts_ptr[3*id1];
            b[1] = face_cent[3*i+1] - pts_ptr[3*id1+1];
            b[2] = face_cent[3*i+2] - pts_ptr[3*id1+2];
            double c[3];
            c[0] = zone_cent[0] - pts_ptr[3*id1];
            c[1] = zone_cent[1] - pts_ptr[3*id1+1];
            c[2] = zone_cent[2] - pts_ptr[3*id1+2];

            double cross[3];
            cross[0] = b[1]*c[2] - b[2]*c[1];
            cross[1] = b[2]*c[0] - b[0]*c[2];
            cross[2] = b[0]*c[1] - b[1]*c[0];

            double vol = (a[0]*cross[0] + a[1]*cross[1] + a[2]*cross[2]) / 6.0;
            if (takeMin)
                rv = (rv < vol ? rv : vol);
            else
                rv = (rv > vol ? rv : vol);
        }
    }
    
    //
    // Clean up memory.
    //
    delete [] face_cent;

    return rv;
}


