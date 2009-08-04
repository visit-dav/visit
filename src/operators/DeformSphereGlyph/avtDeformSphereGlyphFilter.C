/*****************************************************************************
*
* Copyright (c) 2000 - 2009, Lawrence Livermore National Security, LLC
* Produced at the Lawrence Livermore National Laboratory
* LLNL-CODE-400124
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
//  File: avtDeformSphereGlyphFilter.C
// ************************************************************************* //

#include <avtDeformSphereGlyphFilter.h>
#include <vtkDataSet.h>
#include <vtkCellData.h>
#include <vtkPointData.h>
#include <avtDataObject.h>
#include <vtkPolyData.h>
#include <vtkCellArray.h>
#include <vtkPoints.h>
#include <vtkFloatArray.h>

#include <GeometricHelpers.h>

// ****************************************************************************
//  Method: avtDeformSphereGlyphFilter constructor
//
//  Programmer: Jeremy Meredith
//  Creation:   March 19, 2009
//
// ****************************************************************************

avtDeformSphereGlyphFilter::avtDeformSphereGlyphFilter()
{
}


// ****************************************************************************
//  Method: avtDeformSphereGlyphFilter destructor
//
//  Programmer: Jeremy Meredith
//  Creation:   March 19, 2009
//
//  Modifications:
//
// ****************************************************************************

avtDeformSphereGlyphFilter::~avtDeformSphereGlyphFilter()
{
}


// ****************************************************************************
//  Method:  avtDeformSphereGlyphFilter::Create
//
//  Programmer: Jeremy Meredith
//  Creation:   March 19, 2009
//
// ****************************************************************************

avtFilter *
avtDeformSphereGlyphFilter::Create()
{
    return new avtDeformSphereGlyphFilter();
}


// ****************************************************************************
//  Method:      avtDeformSphereGlyphFilter::SetAtts
//
//  Purpose:
//      Sets the state of the filter based on the attribute object.
//
//  Arguments:
//      a        The attributes to use.
//
//  Programmer: Jeremy Meredith
//  Creation:   March 19, 2009
//
// ****************************************************************************

void
avtDeformSphereGlyphFilter::SetAtts(const AttributeGroup *a)
{
    atts = *(const DeformSphereGlyphAttributes*)a;
}


// ****************************************************************************
//  Method: avtDeformSphereGlyphFilter::Equivalent
//
//  Purpose:
//      Returns true if creating a new avtDeformSphereGlyphFilter with the given
//      parameters would result in an equivalent avtDeformSphereGlyphFilter.
//
//  Programmer: Jeremy Meredith
//  Creation:   March 19, 2009
//
// ****************************************************************************

bool
avtDeformSphereGlyphFilter::Equivalent(const AttributeGroup *a)
{
    return (atts == *(DeformSphereGlyphAttributes*)a);
}


// ****************************************************************************
//  Method: avtDeformSphereGlyphFilter::ExecuteData
//
//  Purpose:
//      Sends the specified input and output through the DeformSphereGlyph filter.
//
//  Arguments:
//      in_ds      The input dataset.
//      <unused>   The domain number.
//      <unused>   The label.
//
//  Returns:       The output dataset.
//
//  Programmer: Jeremy Meredith
//  Creation:   March 19, 2009
//
// ****************************************************************************

vtkDataSet *
avtDeformSphereGlyphFilter::ExecuteData(vtkDataSet *in_ds, int, std::string)
{
    // grab the input data
    vtkPointData *inPD = in_ds->GetPointData();
    vtkCellData  *inCD = in_ds->GetCellData();
    vtkDataArray *data = inPD->GetArray(var.c_str());
    bool pointVar = true;
    if (!data)
    {
        pointVar = false;
        data = inCD->GetArray(var.c_str());
    }


    if (!data)
        EXCEPTION1(ImproperUseException, "Could not find variable");

    if (data->GetNumberOfComponents() != geodesic_sphere_npts)
        EXCEPTION1(ImproperUseException, "Array variable had wrong "
                   "number of components.");

    // get the scaling parameters
    double scale = atts.GetScale();
    double minsize = atts.GetMinSize();

    // set up the output data
    vtkPolyData *output = vtkPolyData::New();
    vtkPoints *pts = vtkPoints::New();
    output->SetPoints(pts);
    pts->Delete();
    vtkCellArray *polys = vtkCellArray::New();
    output->SetPolys(polys);
    polys->Delete();

    vtkPointData *outPD = output->GetPointData();
    vtkCellData  *outCD = output->GetCellData();

    int nOutSpheres = pointVar ? in_ds->GetNumberOfPoints()
        : in_ds->GetNumberOfCells();
    int nOutPoints = nOutSpheres * geodesic_sphere_npts;
    int nOutTris   = nOutSpheres * geodesic_sphere_ntris;

    // It is meaningless to copy cell data if we're making our
    // spheres from a point array, and meaningless to copy point
    // data if we're making the sphered from a cell array.  But
    // since later filters seem to blindly assume their requested
    // array will still exist, we'll just copy over the 0th input
    // value for all output data values in the meaningless cases....
    outPD->CopyAllocate(inPD, nOutPoints);
    outCD->CopyAllocate(inCD, nOutTris);

    // actually greate the deformed spheres
    pts->SetNumberOfPoints(nOutPoints);
    for (int s = 0 ; s < nOutSpheres ; s++)
    {
        // calculate the center for the sphere
        double pt[3];
        if (pointVar)
        {
            in_ds->GetPoint(s, pt);
        }
        else
        {
            pt[0] = pt[1] = pt[2] = 0.0;
            vtkCell *cell = in_ds->GetCell(s);
            int ncp = cell->GetNumberOfPoints();
            for (int p=0; p<ncp; p++)
            {
                int id = cell->GetPointId(p);
                double *tp = in_ds->GetPoint(id);
                pt[0] += tp[0];
                pt[1] += tp[1];
                pt[2] += tp[2];
            }
            pt[0] /= double(ncp);
            pt[1] /= double(ncp);
            pt[2] /= double(ncp);
        }

        // base index
        int baseptindex = s*geodesic_sphere_npts;

        // add the points
        double newpt[3];
        for (int p=0; p<geodesic_sphere_npts; p++)
        {
            int newid = baseptindex+p;
            double factor = minsize + scale*data->GetComponent(s,p);
            newpt[0] = pt[0]+factor*geodesic_sphere_points[p][0];
            newpt[1] = pt[1]+factor*geodesic_sphere_points[p][1];
            newpt[2] = pt[2]+factor*geodesic_sphere_points[p][2];
            output->GetPoints()->SetPoint(newid, newpt);
            if (pointVar)
                outPD->CopyData(inPD, s, newid);
            else
                outPD->CopyData(inPD, 0, newid); //meaningless
        }

        // add the tris
        vtkIdType tri[3];
        for (int t=0; t<geodesic_sphere_ntris; t++)
        {
            tri[0] = baseptindex + geodesic_sphere_tris[t][0];
            tri[1] = baseptindex + geodesic_sphere_tris[t][1];
            tri[2] = baseptindex + geodesic_sphere_tris[t][2];
            int newid = polys->InsertNextCell(3, tri);
            if (!pointVar)
                outCD->CopyData(inCD, s, newid);
            else
                outCD->CopyData(inCD, 0, newid); //meaningless
        }
    }

    // Don't keep the array variable!  Total waste of space
    // now that we're done with it.....
    // (Ideally we shouldn't have copied it over at all,
    // but offhand I'm not sure how to easily prevent that.)
    if (pointVar)
        outPD->RemoveArray(var.c_str());
    else
        outCD->RemoveArray(var.c_str());

    ManageMemory(output);

    return output;
}


// ****************************************************************************
//  Method:  avtDeformSphereGlyphFilter::UpdateDataObjectInfo
//
//  Purpose:
//    Invalidate/change various data attributes and validities....
//
//  Arguments:
//    none
//
//  Programmer:  Jeremy Meredith
//  Creation:    March 19, 2009
//
// ****************************************************************************

void
avtDeformSphereGlyphFilter::UpdateDataObjectInfo(void)
{
    avtDataValidity &va = GetOutput()->GetInfo().GetValidity();
    va.InvalidateSpatialMetaData();
    va.InvalidateZones();
    va.SetPointsWereTransformed(true);
    avtDataAttributes &da = GetOutput()->GetInfo().GetAttributes();
}


// ****************************************************************************
//  Method:  avtDeformSphereGlyphFilter::ModifyContract
//
//  Purpose:
//    Add the secondary variable as needed, and grab the
//    name of the variable we're actually using for later reference.
//
//  Arguments:
//    contract   the contract to read/modify
//
//  Programmer:  Jeremy Meredith
//  Creation:    March 19, 2009
//
// ****************************************************************************

avtContract_p
avtDeformSphereGlyphFilter::ModifyContract(avtContract_p contract)
{
    //
    // Get the old contract.
    //
    avtDataRequest_p ds = contract->GetDataRequest();
    const char *primaryVariable = ds->GetVariable();

    //
    // Make a new one
    //
    avtDataRequest_p ndr = new avtDataRequest(ds);

    // If we're using the default variable, save its name
    if (atts.GetVar() == "Default")
    {
        var = primaryVariable;
    }
    else
    {
        var = atts.GetVar();
        ndr->AddSecondaryVariable(strdup(atts.GetVar().c_str()));
    }

    //
    // Create the new pipeline contract from the data request, and return
    //
    avtContract_p rv = new avtContract(contract, ndr);

    return rv;
}
