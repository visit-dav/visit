// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                               avtSideVolume.C                             //
// ************************************************************************* //

#include <avtSideVolume.h>

#include <float.h>

#include <vtkCell.h>
#include <vtkDataArray.h>
#include <vtkDataSet.h>
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
//    Alister Maguire, Thu Jun 18 10:02:58 PDT 2020
//    Set canApplyToDirectDatabaseQOT to false.
//
// ****************************************************************************

avtSideVolume::avtSideVolume()
{
    haveIssuedWarning = false;
    takeMin = true;
    canApplyToDirectDatabaseQOT = false;
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
avtSideVolume::DeriveVariable(vtkDataSet *in_ds, int currentDomainsIndex)
{
    vtkDataArray *arr = CreateArrayFromMesh(in_ds);
    vtkIdType ncells = in_ds->GetNumberOfCells();
    arr->SetNumberOfTuples(ncells);

    for (vtkIdType i = 0 ; i < ncells ; i++)
    {
        vtkCell *cell = in_ds->GetCell(i);
        double vol = GetZoneVolume(cell);
        arr->SetTuple1(i, vol);
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
//    Alister Maguire, Wed Apr 21 10:55:48 PDT 2021
//    Fixed a bug that resulted in invalid volumes.
//
// ****************************************************************************
 
double
avtSideVolume::GetZoneVolume(vtkCell *cell)
{
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
    for (int i = 0 ; i < nFaces ; i++)
    {
        vtkCell *face = cell->GetFace(i);
        face_cent[3*i]   = 0.;
        face_cent[3*i+1] = 0.;
        face_cent[3*i+2] = 0.;
        vtkPoints *pts = face->GetPoints();
        int npts = face->GetNumberOfPoints();
        for (int j = 0 ; j < npts ; j++) 
        {
            double *pts_ptr = pts->GetPoint(j);
            face_cent[3*i]   += pts_ptr[0];
            face_cent[3*i+1] += pts_ptr[1];
            face_cent[3*i+2] += pts_ptr[2];
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
    for (int i = 0 ; i < nFaces ; i++)
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
    for (int i = 0 ; i < nFaces ; i++)
    {
        vtkCell *face = cell->GetFace(i);
        vtkPoints *pts = face->GetPoints();
        int npts = face->GetNumberOfPoints();
        for (int j = 0 ; j < npts ; j++)
        {
            int id2 = j;
            int id1 = (j+1) % npts;
            //
            // NOTE: using the following method for retrieving a point
            // was leading to invalid values. It's unclear why (maybe a
            // vtk bug??), but we should avoid this for now:
            //
            //     double *pt = pts->GetPoint(id);
            //
            double pt1[3];
            double pt2[3];
            pts->GetPoint(id1, pt1);
            pts->GetPoint(id2, pt2);

            //
            // If we represent the tetrahedron as three edge vectors, a, b,
            // and c, the volume is 1/6*|dot(a,cross(b,c))|.
            //
            double a[3];
            a[0] = pt2[0] - pt1[0];
            a[1] = pt2[1] - pt1[1];
            a[2] = pt2[2] - pt1[2];
            double b[3];
            b[0] = face_cent[3*i]   - pt1[0];
            b[1] = face_cent[3*i+1] - pt1[1];
            b[2] = face_cent[3*i+2] - pt1[2];
            double c[3];
            c[0] = zone_cent[0] - pt1[0];
            c[1] = zone_cent[1] - pt1[1];
            c[2] = zone_cent[2] - pt1[2];

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


