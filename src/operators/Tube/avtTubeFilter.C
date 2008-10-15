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
//  File: avtTubeFilter.C
// ************************************************************************* //

#include <avtTubeFilter.h>

#include <vtkCellData.h>
#include <vtkCleanPolyData.h>
#include <vtkConnectedTubeFilter.h>
#include <vtkPointData.h>
#include <vtkPolyData.h>
#include <vtkTubeFilter.h>
#include <vtkUnstructuredGrid.h>

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
//      in_ds      The input dataset.
//      <unused>   The domain number.
//      <unused>   The label.
//
//  Returns:       The output dataset.
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
// ****************************************************************************

vtkDataSet *
avtTubeFilter::ExecuteData(vtkDataSet *in_ds, int, std::string)
{
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

    vtkTubeFilter          *vtktube = vtkTubeFilter::New();
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
        int ncells = ugrid->GetNumberOfCells();
        ugridAsPD->Allocate(ncells);
        for (int i = 0 ; i < ncells ; i++)
        {
            int celltype = ugrid->GetCellType(i);
            vtkIdType *pts;
            int npts;
            ugrid->GetCellPoints(i, npts, pts);
            ugridAsPD->InsertNextCell(celltype, npts, pts);
        }

        cpd->SetInput(ugridAsPD);
        tube->SetInput(ugridAsPD);
    }
    else
    {
        cpd->SetInput((vtkPolyData *)in_ds);
        tube->SetInput((vtkPolyData*)in_ds);
    }

    // Note -- if we're scaling by a variable, our vtkConnectedTubeFilter
    // doesn't yet support this, so fall back to the old VTK one.
    if (atts.GetScaleByVarFlag()==false &&
        tube->BuildConnectivityArrays())
    {
        tube->SetRadius(atts.GetWidth());
        tube->CreateNormalsOn();
        tube->SetNumberOfSides(atts.GetFineness());
        tube->SetCapping(atts.GetCapping() ? 1 : 0);
        output = tube->GetOutput();
    }
    else
    {
        // The vtk tube filter is sensitive to duplicated points, etc.
        // Use the vtkCleanPolyData filter to make sure it is in a good
        // format.
        vtktube->SetInput(cpd->GetOutput());
        if (atts.GetScaleByVarFlag())
            vtktube->SetVaryRadius(VTK_VARY_RADIUS_BY_ABSOLUTE_SCALAR);
        else
            vtktube->SetVaryRadius(VTK_VARY_RADIUS_OFF);
        vtktube->SetRadius(atts.GetWidth());
        vtktube->SetUseDefaultNormal(1);
        vtktube->SetDefaultNormal(0.001, 0.001, 0.001);
        vtktube->SetNumberOfSides(atts.GetFineness());
        vtktube->SetCapping(atts.GetCapping() ? 1 : 0);
        output = vtktube->GetOutput();
    }

    output->Update();
    ManageMemory(output);
    vtktube->Delete();
    tube->Delete();
    cpd->Delete();

    if (ugridAsPD != NULL)
        ugridAsPD->Delete();

    return output;
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
//
// ****************************************************************************

void
avtTubeFilter::UpdateDataObjectInfo(void)
{
    avtDataValidity   &outValidity = GetOutput()->GetInfo().GetValidity();
   
    outValidity.InvalidateZones();
    outValidity.SetNormalsAreInappropriate(true);
}
