// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

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

#include <string>
#include <vector>


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
//      Set the vis windows spatial extents and the y axis tick spacing.
//
//  Programmer: Eric Brugger
//  Creation:   December 12, 2008
//
//  Modifications:
//    Eric Brugger, Tue Mar  3 15:07:09 PST 2009
//    I modified the routine to pass the y axis tick spacing in the plot
//    information so that the plot can include it in the legend.
//
//    Eric Brugger, Fri Mar  6 08:21:35 PST 2009
//    I modified the routine so that the filter could would also accept as
//    input a collection of poly data data sets representing the individual
//    curves to display.
//
//    Kathleen Biagas, Tue Dec 19, 2023
//    Add curves info to PlotInformation.
//
// ****************************************************************************

void
avtMultiCurveFilter::PostExecute(void)
{
    avtDatasetToDatasetFilter::PostExecute();

    avtDataAttributes &inAtts  = GetInput()->GetInfo().GetAttributes();
    avtDataAttributes &outAtts = GetOutput()->GetInfo().GetAttributes();

    //
    // Update the spatial extents.
    //
    std::vector<std::string> labels;
    outAtts.GetLabels(labels);
    int nCurves = (int)labels.size();

    outAtts.GetOriginalSpatialExtents()->Clear();
    outAtts.GetThisProcsOriginalSpatialExtents()->Clear();

    double spatialExtents[6];

    if (inAtts.GetOriginalSpatialExtents()->HasExtents())
    {
        inAtts.GetOriginalSpatialExtents()->CopyTo(spatialExtents);

        spatialExtents[2] = 0.0;
        spatialExtents[3] = nCurves;

        outAtts.GetThisProcsOriginalSpatialExtents()->Set(spatialExtents);
    }
    else if (inAtts.GetThisProcsOriginalSpatialExtents()->HasExtents())
    {
        inAtts.GetThisProcsOriginalSpatialExtents()->CopyTo(spatialExtents);

        spatialExtents[2] = 0.0;
        spatialExtents[3] = nCurves;

        outAtts.GetThisProcsOriginalSpatialExtents()->Set(spatialExtents);
    }

    //
    // If we created the curves (and set the yAxisTickSpacing) then add
    // the y axis tick spacing to the plot information so that the plot
    // can include it in the legend.
    //
    if (setYAxisTickSpacing)
    {
        MapNode axisTickSpacing;
        axisTickSpacing["spacing"] = yAxisTickSpacing;
        outAtts.AddPlotInformation("AxisTickSpacing", axisTickSpacing);
    }
    outAtts.AddPlotInformation("Curves", outputInfo);
}


// ****************************************************************************
//  Method: avtMultiCurveFilter::Execute
//
//  Purpose:
//      Does the actual VTK code to modify the datatree.
//
//  Arguments:
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
//    Eric Brugger, Thu Feb 26 17:40:54 PST 2009
//    I added code to not plot points where the marker variable was negative.
//
//    Eric Brugger, Tue Mar  3 13:31:06 PST 2009
//    I modified the curve scaling to match changes in the spacing and
//    positioning of axes tick marks.  That change involved increasing the
//    number of ticks displayed on an individual curve as the number of
//    axes decreased.
//
//    Eric Brugger, Thu Mar  5 17:21:37 PST 2009
//    I modified the code that calculates the minimum and maximum of the
//    data to exclude values where the marker variable was negative.
//
//    Eric Brugger, Thu Mar  5 17:58:19 PST 2009
//    I replaced UseYAxisRange and YAxisRange with UseYAxisTickSpacing and
//    YAxisTickSpacing.
//
//    Eric Brugger, Fri Mar  6 08:21:35 PST 2009
//    I modified the routine so that the filter could would also accept as
//    input a collection of poly data data sets representing the individual
//    curves to display.
//
//    Kathleen Bonnell, Thu Feb 17 10:11:15 PST 2011
//    Allow setting y-axis labels from input DataAtts labels if present, but
//    only if the number of labels matches the number of y-axes beig creaed.
//
//    Kathleen Biagas, Wed Oct 17 11:22:32 PDT 2012
//    Support double-precision in coordinates and/or data by using
//    vtkDataArray GetTuple1 methods instead of VoidPointer. Merged the
//    separate 'for (int j = 0; j < nx; j++)' loops into one.
//
//    Kathleen Biagas, Thu Jul 13 12:44:14 PDT 2017
//    Don't throw an exception for no datasets. This is valid when running in
//    parallel, doing a time-pick and creating MultiCurve plot from results.
//
//    Eric Brugger, Mon Jun 22 09:24:14 PDT 2020
//    Increased the maximum number of curves to 20. Modified the end cap
//    tick calculation to generate a single tick with 16 or more axes.
//    Previously this gave no tick marks.
//
//    Kathleen Biagas, Tue Dec 19, 2023
//    Fill outputInfo with the xy values of curves.
//
// ****************************************************************************

void
avtMultiCurveFilter::Execute(void)
{
    //
    // Get the input data tree to obtain the data sets.
    //
    avtDataTree_p tree = GetInputDataTree();

    //
    // Get the data sets.
    //
    int nSets;
    vtkDataSet **dataSets = tree->GetAllLeaves(nSets);

    //
    // Get the domain ids.
    //
    std::vector<int> domains;
    tree->GetAllDomainIds(domains);

    //
    // If we have 1 data set then it is a rectilinear grid and this routine
    // will convert it to polydata, if we have more than one data set then
    // it is has already been converted to poly data and we will just pass
    // it along.
    //
    if (nSets < 1)
    {
        // Free the memory from the GetAllLeaves function call.
        delete [] dataSets;
        SetOutputDataTree(tree);
        return;
    }
    else if (nSets > 1)
    {
        for (int i = 0; i < nSets; i++)
        {
            if (dataSets[i]->GetDataObjectType() != VTK_POLY_DATA)
            {
                // Free the memory from the GetAllLeaves function call.
                delete [] dataSets;
                EXCEPTION1(ImproperUseException, "Expecting poly data");
            }
        }
        SetOutputDataTree(tree);
        setYAxisTickSpacing = false;

        // Free the memory from the GetAllLeaves function call.
        delete [] dataSets;
        return;
    }

    vtkDataSet *inDS = dataSets[0];
    // Free the memory from the GetAllLeaves function call.
    delete [] dataSets;

    int domain = domains[0];

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
    if (ny > 20)
    {
        avtCallback::IssueWarning("The MultiCurve plot only allows up to "
            "20 curves, displaying the first 20 curves.");
        ny = 20;
    }

    vtkDataArray *xpts = grid->GetXCoordinates();
    vtkDataArray *ypts = grid->GetYCoordinates();
    int ptype = VTK_FLOAT;
    if (xpts->GetDataType() == VTK_DOUBLE ||
        ypts->GetDataType() == VTK_DOUBLE)
        ptype = VTK_DOUBLE;

    vtkDataArray *var = inDS->GetPointData()->GetScalars();
    if (var == NULL)
    {
        EXCEPTION1(ImproperUseException, "No point data");
    }
    if (var->GetNumberOfComponents() != 1)
    {
        EXCEPTION1(ImproperUseException, "Expecting a scalar variable");
    }

    vtkDataArray *var2 = NULL;
    if (atts.GetMarkerVariable() != "default")
        var2 = inDS->GetPointData()->GetArray(atts.GetMarkerVariable().c_str());
    vtkDataArray *var3 = NULL;
    if (atts.GetIdVariable() != "default")
        var3 = inDS->GetPointData()->GetArray(atts.GetIdVariable().c_str());

    //
    // Create the data sets and labels for each of the curves.
    //
    vtkDataSet **out_ds = new vtkDataSet*[ny];
    std::vector<std::string> labels;

    //
    // Calculate the scale. If the user has specified the Y axis range use
    // that, otherwise use the range of the data.
    //
    double scale;
    double nTicks = floor(32./ny-1.);
    double tickSize = ny / 64.;
    if (ny > 16)
    {
        nTicks = 1.;
        tickSize = 0.25;
    }
    if (atts.GetUseYAxisTickSpacing())
    {
        scale = tickSize / atts.GetYAxisTickSpacing();
        yAxisTickSpacing = atts.GetYAxisTickSpacing();
    }
    else
    {
        double yMin, yMax;
        if (var2 == NULL)
        {
            yMin = var->GetTuple1(0);
            yMax = var->GetTuple1(0);
            for (int i = 1; i < nx * ny; i++)
            {
                double v = var->GetTuple1(i);
                yMin = yMin < v ? yMin : v;
                yMax = yMax > v ? yMax : v;
            }
        }
        else
        {
            yMin = 1.;
            yMax = 1.;
            int i;
            for (i = 0; i < nx * ny && var2->GetTuple1(i) < 0.; i++)
                /* do nothing */;
            if (i < nx * ny)
            {
                yMin = var->GetTuple1(i);
                yMax = var->GetTuple1(i);
                for (; i < nx * ny; i++)
                {
                    if (var2->GetTuple1(i) >= 0.)
                    {
                        double v = var->GetTuple1(i);
                        yMin = yMin < v ? yMin : v;
                        yMax = yMax > v ? yMax : v;
                    }
                }
            }
        }
        double yAbsMax = fabs(yMin) > fabs(yMax) ? fabs(yMin) : fabs(yMax);
        scale = 1. / yAbsMax * nTicks * tickSize;
        yAxisTickSpacing = yAbsMax / nTicks;
    }
    setYAxisTickSpacing = true;

    stringVector inLabels;
    GetInput()->GetInfo().GetAttributes().GetLabels(inLabels);
    doubleVector outputArray;
    for (int i = 0; i < ny; i++)
    {
        //
        // Create the data set.
        //
        vtkPoints *points = vtkPoints::New(ptype);

        //
        // Create the array specifying the curve markers.
        //
        vtkIntArray *symbols = vtkIntArray::New();
        symbols->SetName("CurveSymbols");
        symbols->SetNumberOfComponents(1);
        symbols->SetNumberOfTuples(nx);
        int *sbuf = symbols->GetPointer(0);

        //
        // Create the array specifying the curve ids.
        //
        vtkIntArray *ids = vtkIntArray::New();
        ids->SetName("CurveIds");
        ids->SetNumberOfComponents(1);
        ids->SetNumberOfTuples(nx);
        int *ibuf = ids->GetPointer(0);

        double xLine[3];
        xLine[2] = 0.0;
        int npts = 0;
        for (int j = 0; j < nx; j++)
        {
            if (var2 == NULL || var2->GetTuple1(i*nx+j) >= 0.)
            {
                xLine[0] = xpts->GetTuple1(j);
                xLine[1] = (double)i + 0.5 + var->GetTuple1(i*nx+j) * scale;
                points->InsertNextPoint(xLine);
                outputArray.push_back(xLine[0]);
                outputArray.push_back(var->GetTuple1(i*nx+j));

                // curve markers
                if (var2 == NULL)
                    sbuf[npts] = 0;
                else
                    sbuf[npts] = int(var2->GetTuple1(i*nx+j));

                // curve ids
                if (var3 == NULL)
                    ibuf[npts] = i*nx+j;
                else
                    ibuf[npts] = int(var3->GetTuple1(i*nx+j));

                npts++;
            }
        }

        vtkCellArray *lines = vtkCellArray::New();

        if (npts == 1)
        {
            vtkIdType vtkPointIDs[1];
            vtkPointIDs[0] = 0;
            lines->InsertNextCell(1, vtkPointIDs);
        }
        else if (npts > 1)
        {
            for (int j = 0; j < npts - 1; j++)
            {
                vtkIdType vtkPointIDs[2];
                vtkPointIDs[0] = j;
                vtkPointIDs[1] = j + 1;
                lines->InsertNextCell(2, vtkPointIDs);
            }
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
        if(inLabels.size() != (size_t)ny)
        {
            char label[80];

            snprintf(label, 80, atts.GetYAxisTitleFormat().c_str(),
                     ypts->GetTuple1(i));
            labels.push_back(label);
        }
        else
            labels.push_back(inLabels[i]);
        outputInfo[labels[labels.size()-1]] = outputArray;
        outputArray.clear();
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

    SetOutputDataTree(outDT);
}
