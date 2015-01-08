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

// ************************************************************************* //
//  File: avtTubeFilter.C
// ************************************************************************* //

#include <avtTubeFilter.h>

#include <vtkCellData.h>
#include <vtkCleanPolyData.h>
#include <vtkConnectedTubeFilter.h>
#include <vtkPointData.h>
#include <vtkPolyData.h>
#include <vtkVisItTubeFilter.h>
#include <vtkUnstructuredGrid.h>

#include <avtIntervalTree.h>
#include <avtMetaData.h>

#include <ImproperUseException.h>


// ****************************************************************************
//  Method: avtTubeFilter constructor
//
//  Programmer: childs<generated>
//  Creation:   August28,2001
//
// ****************************************************************************

avtTubeFilter::avtTubeFilter()
{
    scaleFactor = 1;
}


// ****************************************************************************
//  Method: avtTubeFilter destructor
//
//  Programmer: childs<generated>
//  Creation:   August28,2001
//
//  Modifications:
//
// ****************************************************************************

avtTubeFilter::~avtTubeFilter()
{
}


// ****************************************************************************
//  Method:  avtTubeFilter::Create
//
//  Programmer:  childs<generated>
//  Creation:    August28,2001
//
// ****************************************************************************

avtFilter *
avtTubeFilter::Create()
{
    return new avtTubeFilter();
}


// ****************************************************************************
//  Method:      avtTubeFilter::SetAtts
//
//  Purpose:
//      Sets the state of the filter based on the attribute object.
//
//  Arguments:
//      a        The attributes to use.
//
//  Programmer:  childs<generated>
//  Creation:    August28,2001
//
//  Modifications:
//    Jeremy Meredith, Wed Oct 15 15:51:39 EDT 2008
//    Added support for scaling by a variable.
//
// ****************************************************************************

void
avtTubeFilter::SetAtts(const AttributeGroup *a)
{
    atts = *(const TubeAttributes*)a;
}


// ****************************************************************************
//  Method: avtTubeFilter::Equivalent
//
//  Purpose:
//      Returns true if creating a new avtTubeFilter with the given
//      parameters would result in an equivalent avtTubeFilter.
//
//  Programmer: childs<generated>
//  Creation:   August28,2001
//
// ****************************************************************************

bool
avtTubeFilter::Equivalent(const AttributeGroup *a)
{
    return (atts == *(TubeAttributes*)a);
}


// ****************************************************************************
//  Method: avtTubeFilter::ExecuteData
//
//  Purpose:
//      Sends the specified input and output through the Tube filter.
//
//  Arguments:
//      in_dr      The input data representation.
//
//  Returns:       The output data representation.
//
//  Programmer: childs<generated>
//  Creation:   August28,2001
//
//  Modifications:
//
//    Hank Childs, Tue Sep 10 18:29:14 PDT 2002
//    Clean up leaks.
//
//    Hank Childs, Wed Oct 30 11:16:49 PST 2002
//    Added new options.  Also cleaned up poly data.
//
//    Jeremy Meredith, Fri Nov  1 19:07:40 PST 2002
//    Made it use the connected tube filter (our own creation) whenever
//    possible; it welds adjacent segments and creates good normals.
//
//    Hank Childs, Wed May 30 16:34:42 PDT 2007
//    Allow for unstructured grids to also be processed.
//
//    Jeremy Meredith, Wed Oct 15 15:58:15 EDT 2008
//    Added support for scaling by a variable.  Note that our
//    vtkConnectedTubeFilter does not yet support this at all, and 
//    the VTK one is limited to nodal variables.  This should be
//    cleaned up if we make this operator visible by default.
//
//    Jeremy Meredith, Wed May 26 14:54:35 EDT 2010
//    We made a clone of the vtk tube filter to add support for
//    cell-scalar based radius scaling.
//
//    Hank Childs, Thu Oct 21 06:32:59 PDT 2010
//    Use "scaleFactor" for scaling, which may either come from an absolute
//    value or from a fractino of the bounding box.
//
//    Kathleen Biagas, Tue Aug  7 10:52:13 PDT 2012
//    Send the scale variable to the vtk tube filter when needed.
// 
//    Eric Brugger, Wed Jan  9 13:17:11 PST 2013
//    Modified to inherit from vtkPolyDataAlgorithm.
//
//    Eric Brugger, Tue Aug 19 09:45:37 PDT 2014
//    Modified the class to work with avtDataRepresentation.
//
// ****************************************************************************

avtDataRepresentation *
avtTubeFilter::ExecuteData(avtDataRepresentation *in_dr)
{
    //
    // Get the VTK data set.
    //
    vtkDataSet *in_ds = in_dr->GetDataVTK();

    bool haveSurface = false;
    if (in_ds->GetDataObjectType() == VTK_POLY_DATA)
        haveSurface = true;
    if (in_ds->GetDataObjectType() == VTK_UNSTRUCTURED_GRID &&
        GetInput()->GetInfo().GetAttributes().GetTopologicalDimension() <= 2)
        haveSurface = true;

    if (!haveSurface)
    {
        EXCEPTION0(ImproperUseException);
    }

    vtkVisItTubeFilter     *vtktube = vtkVisItTubeFilter::New();
    vtkCleanPolyData       *cpd     = vtkCleanPolyData::New();
    vtkConnectedTubeFilter *tube    = vtkConnectedTubeFilter::New();
    vtkDataSet *output;

    vtkPolyData *ugridAsPD = NULL;
    if (in_ds->GetDataObjectType() == VTK_UNSTRUCTURED_GRID)
    {
        vtkUnstructuredGrid *ugrid = (vtkUnstructuredGrid *) in_ds;
        vtkPolyData *ugridAsPD = vtkPolyData::New();
        ugridAsPD->SetPoints(ugrid->GetPoints());
        ugridAsPD->GetPointData()->ShallowCopy(ugrid->GetPointData());
        ugridAsPD->GetCellData()->ShallowCopy(ugrid->GetCellData());
        ugridAsPD->GetFieldData()->ShallowCopy(ugrid->GetFieldData());
        vtkIdType ncells = ugrid->GetNumberOfCells();
        ugridAsPD->Allocate(ncells);
        for (vtkIdType i = 0 ; i < ncells ; i++)
        {
            int celltype = ugrid->GetCellType(i);
            vtkIdType *pts, npts;
            ugrid->GetCellPoints(i, npts, pts);
            ugridAsPD->InsertNextCell(celltype, npts, pts);
        }

        cpd->SetInputData(ugridAsPD);
        tube->SetInputData(ugridAsPD);
    }
    else
    {
        cpd->SetInputData(in_ds);
        tube->SetInputData(in_ds);
    }

    // Note -- if we're scaling by a variable, our vtkConnectedTubeFilter
    // doesn't yet support this, so fall back to the old VTK one.
    if (atts.GetScaleByVarFlag()==false &&
        tube->BuildConnectivityArrays((vtkPolyData*)in_ds))
    {
        tube->SetRadius(scaleFactor);
        tube->CreateNormalsOn();
        tube->SetNumberOfSides(atts.GetFineness());
        tube->SetCapping(atts.GetCapping() ? 1 : 0);
        tube->Update();
        output = tube->GetOutput();
    }
    else
    {
        // The vtk tube filter is sensitive to duplicated points, etc.
        // Use the vtkCleanPolyData filter to make sure it is in a good
        // format.
        vtktube->SetInputConnection(cpd->GetOutputPort());
        if (atts.GetScaleByVarFlag())
        {
            vtktube->SetVaryRadius(VTK_VARY_RADIUS_BY_ABSOLUTE_SCALAR);
            std::string v = atts.GetScaleVariable();
            if (!v.empty() && v != "default")
                vtktube->SetScalarsForRadius(v.c_str());
        }
        else
            vtktube->SetVaryRadius(VTK_VARY_RADIUS_OFF);
        vtktube->SetRadius(scaleFactor);
        vtktube->SetUseDefaultNormal(1);
        vtktube->SetDefaultNormal(0.001, 0.001, 0.001);
        vtktube->SetNumberOfSides(atts.GetFineness());
        vtktube->SetCapping(atts.GetCapping() ? 1 : 0);
        vtktube->Update();
        output = vtktube->GetOutput();
    }

    avtDataRepresentation *out_dr = new avtDataRepresentation(output,
        in_dr->GetDomain(), in_dr->GetLabel());

    vtktube->Delete();
    tube->Delete();
    cpd->Delete();

    if (ugridAsPD != NULL)
        ugridAsPD->Delete();

    return out_dr;
}


// ****************************************************************************
//  Method: avtTubeFilter::UpdateDataObjectInfo
//
//  Purpose:
//      Changes attributes of output.
//
//  Programmer: Jeremy Meredith
//  Creation:   November  1, 2002
//
//  Modifications:
//    Brad Whitlock, Mon Apr  7 15:55:02 PDT 2014
//    Add filter metadata used in export.
//    Work partially supported by DOE Grant SC0007548.
//
// ****************************************************************************

void
avtTubeFilter::UpdateDataObjectInfo(void)
{
    avtDataValidity   &outValidity = GetOutput()->GetInfo().GetValidity();
   
    outValidity.InvalidateZones();
    outValidity.SetNormalsAreInappropriate(true);

    GetOutput()->GetInfo().GetAttributes().AddFilterMetaData("Tube");
}


// ****************************************************************************
//  Modify: avtTubeFilter::ModifyContract
//
//  Purpose:
//      Figure out if we will need to do collective communication later on.
//
//  Programmer: Hank Childs
//  Creation:   October 20, 2010
//
//  Modifications:
//    Kathleen Biagas, Tue Aug  7 10:50:38 PDT 2012
//    Request secondary variable for scaling radius when needed.
//
// ****************************************************************************

avtContract_p
avtTubeFilter::ModifyContract(avtContract_p c)
{
    avtContract_p spec = new avtContract(c);
    if (atts.GetScaleByVarFlag() == false &&
        atts.GetTubeRadiusType() == TubeAttributes::FractionOfBBox)
    {
        avtIntervalTree *it = GetMetaData()->GetSpatialExtents();
        if (it == NULL)
            spec->NoStreaming();
    }
    if (atts.GetScaleByVarFlag())
    { 
        std::string v = atts.GetScaleVariable();
        if (!v.empty() && v != "default")
        {
            avtDataRequest_p dataRequest = spec->GetDataRequest();
            dataRequest->AddSecondaryVariable(v.c_str());
        }
    }
        
    return spec;
}


// ****************************************************************************
//  Method: avtTubeFilter::PreExecute
//
//  Purpose:
//      Calculate the scale factor.
//
//  Programmer: Hank Childs
//  Creation:   October 20, 2010
//
// ****************************************************************************

void
avtTubeFilter::PreExecute(void)
{
    if (atts.GetScaleByVarFlag() == true)
        return;
    if (atts.GetTubeRadiusType() == TubeAttributes::Absolute)
    {
        scaleFactor = atts.GetRadiusAbsolute();
    }
    else
    {
        int dim = GetInput()->GetInfo().GetAttributes().GetSpatialDimension();
        double bbox[6];
        avtIntervalTree *it = GetMetaData()->GetSpatialExtents();
        if (it != NULL)
            it->GetExtents(bbox);
        else
            GetSpatialExtents(bbox);

        int numReal = 0;
        double volume = 1.0;
        for (int i = 0 ; i < dim ; i++)
        {
            if (bbox[2*i] != bbox[2*i+1])
            {
                numReal++;
                volume *= (bbox[2*i+1]-bbox[2*i]);
            }
        }
        if (volume < 0)
            volume *= -1.;
        if (numReal > 0)
            scaleFactor = pow(volume, 1.0/numReal)*atts.GetRadiusFractionBBox();
        else
            scaleFactor = 1*atts.GetRadiusFractionBBox();
    }
}


