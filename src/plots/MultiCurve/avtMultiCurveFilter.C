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
//                           avtMultiCurveFilter.C                           //
// ************************************************************************* //

#include <avtMultiCurveFilter.h>

#include <vtkCellArray.h>
#include <vtkDataArray.h>
#include <vtkDataSet.h>
#include <vtkFieldData.h>
#include <vtkFloatArray.h>
#include <vtkIntArray.h>
#include <vtkPointData.h>
#include <vtkPolyData.h>
#include <vtkRectilinearGrid.h>

#include <avtCallback.h>
#include <avtExtents.h>

#include <ImproperUseException.h>

#include <snprintf.h>

// ****************************************************************************
//  Method: avtMultiCurveFilter constructor
//
//  Programmer: xml2avt
//  Creation:   omitted
//
// ****************************************************************************

avtMultiCurveFilter::avtMultiCurveFilter(MultiCurveAttributes &atts)
{
}


// ****************************************************************************
//  Method: avtMultiCurveFilter destructor
//
//  Programmer: xml2avt
//  Creation:   omitted
//
// ****************************************************************************

avtMultiCurveFilter::~avtMultiCurveFilter()
{
    return;
}


// ****************************************************************************
//  Method: avtMultiCurveFilter::SetAttributes
//
//  Purpose:
//      Sets the attributes for this filter.
//
//  Arguments:
//      mc_atts The attributes for the plot.
//
//  Programmer: Eric Brugger
//  Creation:   December 12, 2008
//
// ****************************************************************************

void
avtMultiCurveFilter::SetAttributes(const MultiCurveAttributes &mc_atts)
{
    atts = mc_atts;
}


// ****************************************************************************
//  Method: avtMultiCurveFilter::PreExecute
//
//  Purpose:
//      Set the labels for the curves.
//
//  Programmer: Eric Brugger
//  Creation:   December 12, 2008
//
// ****************************************************************************

void
avtMultiCurveFilter::PreExecute(void)
{
    avtDatasetToDatasetFilter::PreExecute();
}


// ****************************************************************************
//  Method: avtMultiCurveFilter::PostExecute
//
//  Purpose:
//      Set the vis windows spatial extents.
//
//  Programmer: Eric Brugger
//  Creation:   December 12, 2008
//
// ****************************************************************************

void
avtMultiCurveFilter::PostExecute(void)
{
    avtDatasetToDatasetFilter::PostExecute();

    avtDataAttributes &inAtts  = GetInput()->GetInfo().GetAttributes();
    avtDataAttributes &outAtts = GetOutput()->GetInfo().GetAttributes();

    std::vector<std::string> labels;
    outAtts.GetLabels(labels);
    int nCurves = labels.size();

    outAtts.GetTrueSpatialExtents()->Clear();
    outAtts.GetCumulativeTrueSpatialExtents()->Clear();

    double spatialExtents[6];

    if (inAtts.GetTrueSpatialExtents()->HasExtents())
    {
        inAtts.GetTrueSpatialExtents()->CopyTo(spatialExtents);

        spatialExtents[2] = 0.0;
        spatialExtents[3] = nCurves;

        outAtts.GetCumulativeTrueSpatialExtents()->Set(spatialExtents);
    }
    else if (inAtts.GetCumulativeTrueSpatialExtents()->HasExtents())
    {
        inAtts.GetCumulativeTrueSpatialExtents()->CopyTo(spatialExtents);

        spatialExtents[2] = 0.0;
        spatialExtents[3] = nCurves;

        outAtts.GetCumulativeTrueSpatialExtents()->Set(spatialExtents);
    }
}


// ****************************************************************************
//  Method: avtMultiCurveFilter::ExecuteDataTree
//
//  Purpose:
//      Does the actual VTK code to modify the datatree.
//
//  Arguments:
//      inDS      The input dataset.
//      domain    The domain number.
//      label     The label.
//
//  Returns:      The output datatree.
//
//  Programmer: xml2avt
//  Creation:   omitted
//
//  Modifications:
//    Eric Brugger, Wed Jan 21 08:15:16 PST 2009
//    I added code to properly set the y axis scale.  I added the ability
//    to specify the format for the curve label strings, which are used for
//    the y-axis title for the individual curves.
//
//    Eric Brugger, Wed Feb 18 08:25:08 PST 2009
//    I added code to handle the id variable.
//
// ****************************************************************************

avtDataTree_p
avtMultiCurveFilter::ExecuteDataTree(vtkDataSet *inDS, int domain,
    std::string label)
{
    if (inDS->GetDataObjectType() != VTK_RECTILINEAR_GRID)
    {
        EXCEPTION1(ImproperUseException, "Expecting a rectilinear grid");
    }

    vtkRectilinearGrid *grid = vtkRectilinearGrid::SafeDownCast(inDS);
    if (grid->GetDataDimension() != 2)
    {
        EXCEPTION1(ImproperUseException, "Expecting a 2D grid");
    }

    int dims[3];
    grid->GetDimensions(dims);

    int nx = dims[0];
    int ny = dims[1];
    if (ny > 16)
    {
        avtCallback::IssueWarning("The MultiCurve plot only allows up to "
            "16 curves, displaying the first 16 curves.");
        ny = 16;
    }

    float *xpts = vtkFloatArray::SafeDownCast(grid->GetXCoordinates())
        ->GetPointer(0);
    float *ypts = vtkFloatArray::SafeDownCast(grid->GetYCoordinates())
        ->GetPointer(0);

    vtkDataArray *var = inDS->GetPointData()->GetScalars();
    vtkDataArray *var2 = NULL;
    if (atts.GetMarkerVariable() != "default")
        var2 = inDS->GetPointData()->GetArray(atts.GetMarkerVariable().c_str());
    vtkDataArray *var3 = NULL;
    if (atts.GetIdVariable() != "default")
        var3 = inDS->GetPointData()->GetArray(atts.GetIdVariable().c_str());
    if (var == NULL)
    {
        EXCEPTION1(ImproperUseException, "No point data");
    }
    if (var->GetNumberOfComponents() != 1)
    {
        EXCEPTION1(ImproperUseException, "Expecting a scalar variable");
    }
    float *vals = vtkFloatArray::SafeDownCast(var)->GetPointer(0);
    float *vals2 = NULL;
    if (var2 != NULL)
        vals2 = vtkFloatArray::SafeDownCast(var2)->GetPointer(0);
    float *vals3 = NULL;
    if (var3 != NULL)
        vals3 = vtkFloatArray::SafeDownCast(var3)->GetPointer(0);

    //
    // Create the data sets and labels for each of the curves.
    //
    vtkDataSet **out_ds = new vtkDataSet*[ny];
    vector<string> labels;

    //
    // Calculate the scale. If the user has specified the Y axis range use
    // that, otherwise use the range of the data.
    //
    double scale;
    if (atts.GetUseYAxisRange())
    {
        scale = 1. / (atts.GetYAxisRange() / 2.);
    }
    else
    {
        double yMin = vals[0];
        double yMax = vals[0];
        for (int i = 0; i < nx * ny; i++)
        {
            yMin = yMin < vals[i] ? yMin : vals[i];
            yMax = yMax > vals[i] ? yMax : vals[i];
        }
        double yAbsMax = fabs(yMin) > fabs(yMax) ? fabs(yMin) : fabs(yMax);
        scale = 1. / yAbsMax;
    }

    for (int i = 0; i < ny; i++)
    {
        //
        // Create the data set.
        //
        vtkPoints *points = vtkPoints::New();

        double xLine[3];
        xLine[2] = 0.0;
        for (int j = 0; j < nx; j++)
        {
            xLine[0] = xpts[j];
            xLine[1] = (double)i + 0.5 + vals[i*nx+j] * scale * (1. / 3.);
            points->InsertNextPoint(xLine);
        }

        vtkCellArray *lines = vtkCellArray::New();

        for (int j = 0; j < nx - 1; j++)
        {
            vtkIdType vtkPointIDs[2];
            vtkPointIDs[0] = j;
            vtkPointIDs[1] = j + 1;
            lines->InsertNextCell(2, vtkPointIDs);
        }

        //
        // Create the array specifying the curve markers.
        //
        vtkIntArray *symbols = vtkIntArray::New();
        symbols->SetName("CurveSymbols");
        symbols->SetNumberOfComponents(1);
        symbols->SetNumberOfTuples(nx);
        int *buf = symbols->GetPointer(0);
        for (int j = 0; j < nx; j++)
        {
            if (vals2 == NULL)
                buf[j] = 0;
            else
                buf[j] = int(vals2[i*nx+j]);
        }

        //
        // Create the array specifying the curve ids.
        //
        vtkIntArray *ids = vtkIntArray::New();
        ids->SetName("CurveIds");
        ids->SetNumberOfComponents(1);
        ids->SetNumberOfTuples(nx);
        buf = ids->GetPointer(0);
        for (int j = 0; j < nx; j++)
        {
            if (vals3 == NULL)
                buf[j] = i*nx+j;
            else
                buf[j] = int(vals3[i*nx+j]);
        }

        //
        // Create the data set.
        //
        vtkPolyData *polyData = vtkPolyData::New();

        polyData->SetLines(lines);
        polyData->SetPoints(points);
        polyData->GetPointData()->AddArray(symbols);
        polyData->GetPointData()->CopyFieldOn("CurveSymbols");
        polyData->GetPointData()->AddArray(ids);
        polyData->GetPointData()->CopyFieldOn("CurveIds");
        lines->Delete();
        points->Delete();
        symbols->Delete();
        ids->Delete();

        out_ds[i] = polyData;
    
        //
        // Create the label.
        //
        char label[80];
        SNPRINTF(label, 80, atts.GetYAxisTitleFormat().c_str(), ypts[i]);
        labels.push_back(label);
    }

    //
    // Turn the data sets into a data tree.
    //
    avtDataTree_p outDT = NULL;
    outDT = new avtDataTree(ny, out_ds, domain, labels);

    for (int i = 0; i < ny; i++)
    {
        out_ds[i]->Delete();
    }
    delete [] out_ds;

    //
    // Set the levels for the level mapper.
    //
    GetOutput()->GetInfo().GetAttributes().SetLabels(labels);

    return outDT;
}
