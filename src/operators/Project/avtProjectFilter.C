/*****************************************************************************
*
* Copyright (c) 2000 - 2007, The Regents of the University of California
* Produced at the Lawrence Livermore National Laboratory
* All rights reserved.
*
* This file is part of VisIt. For details, see http://www.llnl.gov/visit/. The
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
*    documentation and/or materials provided with the distribution.
*  - Neither the name of the UC/LLNL nor  the names of its contributors may be
*    used to  endorse or  promote products derived from  this software without
*    specific prior written permission.
*
* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT  HOLDERS AND CONTRIBUTORS "AS IS"
* AND ANY EXPRESS OR  IMPLIED WARRANTIES, INCLUDING,  BUT NOT  LIMITED TO, THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND  FITNESS FOR A PARTICULAR  PURPOSE
* ARE  DISCLAIMED.  IN  NO  EVENT  SHALL  THE  REGENTS  OF  THE  UNIVERSITY OF
* CALIFORNIA, THE U.S.  DEPARTMENT  OF  ENERGY OR CONTRIBUTORS BE  LIABLE  FOR
* ANY  DIRECT,  INDIRECT,  INCIDENTAL,  SPECIAL,  EXEMPLARY,  OR CONSEQUENTIAL
* DAMAGES (INCLUDING, BUT NOT  LIMITED TO, PROCUREMENT OF  SUBSTITUTE GOODS OR
* SERVICES; LOSS OF  USE, DATA, OR PROFITS; OR  BUSINESS INTERRUPTION) HOWEVER
* CAUSED  AND  ON  ANY  THEORY  OF  LIABILITY,  WHETHER  IN  CONTRACT,  STRICT
* LIABILITY, OR TORT  (INCLUDING NEGLIGENCE OR OTHERWISE)  ARISING IN ANY  WAY
* OUT OF THE  USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH
* DAMAGE.
*
*****************************************************************************/

// ************************************************************************* //
//  File: avtProjectFilter.C
// ************************************************************************* //

#include <avtProjectFilter.h>

#include <vtkCell.h>
#include <vtkCellData.h>
#include <vtkPointData.h>
#include <vtkRectilinearGrid.h>
#include <vtkStructuredGrid.h>
#include <vtkUnstructuredGrid.h>
#include <vtkPolyData.h>

#include <avtDatasetExaminer.h>
#include <avtExtents.h>

#include <ImproperUseException.h>


// ****************************************************************************
//  Method: avtProjectFilter constructor
//
//  Programmer: Jeremy Meredith
//  Creation:   September  3, 2004
//
// ****************************************************************************

avtProjectFilter::avtProjectFilter()
{
}


// ****************************************************************************
//  Method: avtProjectFilter destructor
//
//  Programmer: Jeremy Meredith
//  Creation:   September  3, 2004
//
//  Modifications:
//
// ****************************************************************************

avtProjectFilter::~avtProjectFilter()
{
}


// ****************************************************************************
//  Method:  avtProjectFilter::Create
//
//  Programmer: Jeremy Meredith
//  Creation:   September  3, 2004
//
// ****************************************************************************

avtFilter *
avtProjectFilter::Create()
{
    return new avtProjectFilter();
}


// ****************************************************************************
//  Method:      avtProjectFilter::SetAtts
//
//  Purpose:
//      Sets the state of the filter based on the attribute object.
//
//  Arguments:
//      a        The attributes to use.
//
//  Programmer: Jeremy Meredith
//  Creation:   September  3, 2004
//
// ****************************************************************************

void
avtProjectFilter::SetAtts(const AttributeGroup *a)
{
    atts = *(const ProjectAttributes*)a;
}


// ****************************************************************************
//  Method: avtProjectFilter::Equivalent
//
//  Purpose:
//      Returns true if creating a new avtProjectFilter with the given
//      parameters would result in an equivalent avtProjectFilter.
//
//  Programmer: Jeremy Meredith
//  Creation:   September  3, 2004
//
// ****************************************************************************

bool
avtProjectFilter::Equivalent(const AttributeGroup *a)
{
    return (atts == *(ProjectAttributes*)a);
}


// ****************************************************************************
//  Method: avtProjectFilter::ExecuteData
//
//  Purpose:
//      Sends the specified input and output through the Project filter.
//
//  Arguments:
//      in_ds      The input dataset.
//      <unused>   The domain number.
//      <unused>   The label.
//
//  Returns:       The output dataset.
//
//  Programmer: Jeremy Meredith
//  Creation:   September  3, 2004
//
//  Modifications:
//    Jeremy Meredith, Fri Sep 10 16:15:55 PDT 2004
//    Added projection of vectors.
//
// ****************************************************************************

vtkDataSet *
avtProjectFilter::ExecuteData(vtkDataSet *in_ds, int, std::string)
{
    int  datatype = in_ds->GetDataObjectType();
    vtkPointSet *out_ds = NULL;
    switch (datatype)
    {
      case VTK_RECTILINEAR_GRID:
        out_ds = ProjectRectilinearGrid((vtkRectilinearGrid*)in_ds);
        break;

      case VTK_STRUCTURED_GRID:
      case VTK_UNSTRUCTURED_GRID:
      case VTK_POLY_DATA:
        out_ds = ProjectPointSet((vtkPointSet*)in_ds);
        break;

      default:
        EXCEPTION0(ImproperUseException);
    }

    //
    // We have said normals are inappropriate, since this will be a
    // 2D data set.  We have not touched the vectors, however --
    // we have only projected the mesh itself so far.
    //
    vtkDataArray *vectors;
    vectors = out_ds->GetPointData()->GetVectors();
    if (vectors)
    {
        vtkDataArray *arr = vectors->NewInstance();
        arr->SetNumberOfComponents(3);
        arr->Allocate(3*vectors->GetNumberOfTuples());
        ProjectVectors(in_ds, out_ds, vectors, arr, false);
        arr->SetName(vectors->GetName());
        out_ds->GetPointData()->RemoveArray(vectors->GetName());
        out_ds->GetPointData()->SetVectors(arr);
        arr->Delete();
    }
    vectors = out_ds->GetCellData()->GetVectors();
    if (vectors)
    {
        vtkDataArray *arr = vectors->NewInstance();
        arr->SetNumberOfComponents(3);
        arr->Allocate(3*vectors->GetNumberOfTuples());
        ProjectVectors(in_ds, out_ds, vectors, arr, true);
        arr->SetName(vectors->GetName());
        out_ds->GetPointData()->RemoveArray(vectors->GetName());
        out_ds->GetPointData()->SetVectors(arr);
        arr->Delete();
    }


    return out_ds;
}


// ****************************************************************************
//  Method: avtProjectFilter::PostExecute
//
//  Purpose:
//      Finds the extents once we have been projected and set those.
//
//  Programmer: Hank Childs
//  Creation:   January 20, 2005
//
// ****************************************************************************

void
avtProjectFilter::PostExecute(void)
{
    avtPluginStreamer::PostExecute();

    avtDataAttributes& outAtts = GetOutput()->GetInfo().GetAttributes();

    // get the outputs's spatial extents
    double se[6];
    avtDataset_p output = GetTypedOutput();
    avtDatasetExaminer::GetSpatialExtents(output, se);

    // over-write spatial extents
    outAtts.GetTrueSpatialExtents()->Clear();
    outAtts.GetCumulativeTrueSpatialExtents()->Set(se);
}


// ****************************************************************************
//  Method:  avtProjectFilter::ProjectPoint
//
//  Purpose:
//    Project a single point in-place.
//
//  Arguments:
//    x,y,z      the point to project
//
//  Programmer:  Jeremy Meredith
//  Creation:    September  3, 2004
//
// ****************************************************************************
void
avtProjectFilter::ProjectPoint(float &x,float &y,float &z)
{
    switch (atts.GetProjectionType())
    {
      case ProjectAttributes::XYCartesian:
        z = 0;
        break;

      case ProjectAttributes::ZRCylindrical:
        {
            float r = sqrt(x*x + y*y);
            x = z;
            y = r;
            z = 0;
        }
        break;
    }
}

// ****************************************************************************
//  Method:  avtProjectFilter::ProjectRectilinearGrid
//
//  Purpose:
//    Converts a rectilinear grid to a curvilinear grid while projecting
//
//  Arguments:
//    in_ds      the rectilinear grid to project
//
//  Programmer:  Jeremy Meredith
//  Creation:    September  6, 2004
//
//  Modifications:
//    Jeremy Meredith, Fri Sep 10 16:16:12 PDT 2004
//    Always convert to a curvilinear grid.  The extra code wasn't even
//    worth it because I expect no one will ever want to do it.
//
// ****************************************************************************
vtkPointSet *
avtProjectFilter::ProjectRectilinearGrid(vtkRectilinearGrid *in_ds)
{
    int  dims[3];
    in_ds->GetDimensions(dims);

    int  numPts = dims[0]*dims[1]*dims[2];

    vtkPoints *pts = vtkPoints::New();
    pts->SetNumberOfPoints(numPts);

    vtkDataArray *x = in_ds->GetXCoordinates();
    vtkDataArray *y = in_ds->GetYCoordinates();
    vtkDataArray *z = in_ds->GetZCoordinates();

    int index = 0;
    for (int k = 0 ; k < dims[2] ; k++)
    {
        for (int j = 0 ; j < dims[1] ; j++)
        {
            for (int i = 0 ; i < dims[0] ; i++)
            {
                float pt[3];
                pt[0] = x->GetComponent(i,0);
                pt[1] = y->GetComponent(j,0);
                pt[2] = z->GetComponent(k,0);

                ProjectPoint(pt[0],pt[1],pt[2]);

                pts->SetPoint(index++, pt);
            }
        }
    }

    vtkStructuredGrid *out = vtkStructuredGrid::New();
    out->SetDimensions(dims);
    out->SetPoints(pts);
    pts->Delete();
    out->GetCellData()->ShallowCopy(in_ds->GetCellData());
    out->GetPointData()->ShallowCopy(in_ds->GetPointData());

    //
    // We want to reduce the reference count of this dataset so it doesn't get
    // leaked.  But where to store it?  Fortunately, our base class handles
    // this for us.
    //
    ManageMemory(out);
    out->Delete();

    return out;
}


// ****************************************************************************
//  Method:  avtProjectFilter::ProjectPointSet
//
//  Purpose:
//    Projects a descendent of vtkPointSet.  This includes
//    curvilinear, unstructured, point meshes, and poly data.
//
//  Arguments:
//    in_ds      the data set to project
//
//  Programmer:  Jeremy Meredith
//  Creation:    September  6, 2004
//
// ****************************************************************************
vtkPointSet *
avtProjectFilter::ProjectPointSet(vtkPointSet *in_ds)
{
    vtkPointSet *out_ds = in_ds->NewInstance();
    out_ds->ShallowCopy(in_ds);

    vtkPoints *old_pts = in_ds->GetPoints();
    int npoints = old_pts->GetNumberOfPoints();

    // Make a new point array
    vtkPoints *new_pts = old_pts->NewInstance();
    new_pts->DeepCopy(old_pts);
    float *points = (float*)new_pts->GetVoidPointer(0); // Assume float
    for (int i = 0 ; i < npoints ; i++)
    {
        
        ProjectPoint(points[i*3+0],
                     points[i*3+1],
                     points[i*3+2]);
    }
    out_ds->SetPoints(new_pts);
    new_pts->Delete();

    ManageMemory(out_ds);
    out_ds->Delete();
    return out_ds;
}

// ****************************************************************************
//  Method:  avtProjectFilter::ProjectVectors
//
//  Purpose:
//    Project some vectors!
//
//  Arguments:
//    old_ds          the original dataset
//    new_ds          the transformed one
//    in              the vectors to project
//    out             the place to store the new vectors
//    cell_centered   true if these vectors are cell data
//
//  Notes:  Yes, it is horribly inefficient.  Get over it or rewrite it.
//          Plus, it might not even be doing the right thing!  If you
//          know what it truly means to project a vector cylindrically,
//          you are welcome to fix that as well.
//
//  Programmer:  Jeremy Meredith
//  Creation:    September 10, 2004
//
// ****************************************************************************
void
avtProjectFilter::ProjectVectors(vtkDataSet *old_ds, 
                                 vtkDataSet *new_ds,
                                 vtkDataArray *in,
                                 vtkDataArray *out,
                                 bool cell_centered)
{
    int nvectors  = in->GetNumberOfTuples();
    float *inptr  = (float*)in->GetVoidPointer(0);
    float *outptr = (float*)out->GetVoidPointer(0);

    for (int i=0; i<nvectors; i++)
    {
        double oldpt[3];
        double newpt[3];

        if (cell_centered)
        {
            old_ds->GetPoint(old_ds->GetCell(i)->GetPointId(0), oldpt);
            new_ds->GetPoint(new_ds->GetCell(i)->GetPointId(0), newpt);
        }
        else
        {
            old_ds->GetPoint(i, oldpt);
            new_ds->GetPoint(i, newpt);
        }

        // What the heck is the right thing for projecting a
        // vector!?  Especially a vector that is cell-centered?!?!?
        // Especially a cylindrical projection!

        // Well, we'll do something defensible.  Assume the vector is
        // a displacement.  The new vector will be the one that takes
        // us FROM the post-transformed location TO the spot that
        // started at the original location, was first displaced
        // using the original vector, and was *then* projected.
        float u = inptr[i*3+0];
        float v = inptr[i*3+1];
        float w = inptr[i*3+2];

        float x = oldpt[0] + u;
        float y = oldpt[1] + v;
        float z = oldpt[2] + w;

        ProjectPoint(x,y,z);

        outptr[i*3+0] = x - newpt[0];
        outptr[i*3+1] = y - newpt[1];
        outptr[i*3+2] = z - newpt[2];
    }
}

// ****************************************************************************
//  Method:  avtProjectFilter::RefashionDataObjectInfo
//
//  Purpose:
//    Set up the atttributes and validity for the output of the filter.
//
//  Arguments:
//    none
//
//  Programmer:  Jeremy Meredith
//  Creation:    September  6, 2004
//
//  Modifications:
//
//    Hank Childs, Tue Feb  1 15:37:37 PST 2005
//    Allow normals, since if they are not appropriate, it will be detected.
//
//    Hank Childs, Fri Jan 13 09:58:47 PST 2006
//    Invalidate spatial meta-data.
//
// ****************************************************************************
void
avtProjectFilter::RefashionDataObjectInfo(void)
{
    avtDataAttributes &inAtts      = GetInput()->GetInfo().GetAttributes();
    avtDataAttributes &outAtts     = GetOutput()->GetInfo().GetAttributes();
   
    if (inAtts.GetSpatialDimension() == 3)
        outAtts.SetSpatialDimension(2);

    GetOutput()->GetInfo().GetValidity().InvalidateSpatialMetaData();
}

// ****************************************************************************
//  Method: avtProjectFilter::PerformRestriction
//
//  Purpose:
//    Turn on zone/node numbers if needed for pick (for example).
//
//  Programmer: Jeremy Meredith
//  Creation:   September  9, 2004
//
//  Note:  I copied this implementation from Displace for now since
//         it seemed the most alike to this requirements for getting
//         pick to work.
//
//  Modifications:
//
// ****************************************************************************

avtPipelineSpecification_p
avtProjectFilter::PerformRestriction(avtPipelineSpecification_p spec)
{
    avtPipelineSpecification_p rv = new avtPipelineSpecification(spec);
    if (rv->GetDataSpecification()->MayRequireZones())
    {
        rv->GetDataSpecification()->TurnZoneNumbersOn();
    }
    if (rv->GetDataSpecification()->MayRequireNodes())
    {
        rv->GetDataSpecification()->TurnNodeNumbersOn();
    }
    return rv;
}

