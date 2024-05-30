// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ****************************************************************************
//   avtLineGlyphFilter.C
// ****************************************************************************

#include <avtLineGlyphFilter.h>

#include <vtkAppendPolyData.h>
#include <vtkCellData.h>
#include <vtkCellArrayIterator.h>
#include <vtkConeSource.h>
#include <vtkExtractCellsByType.h>
#include <vtkGeometryFilter.h>
#include <vtkPointData.h>
#include <vtkPolyData.h>
#include <vtkRibbonFilter.h>
#include <vtkSphereSource.h>
#include <vtkTubeFilter.h>
#include <vtkUnstructuredGrid.h>

#include <string>
#include <vector>

using std::string;
using std::vector;

namespace {
    double GetBBoxSize( double *bbox )
    {
        double vol = 1;
        int    numDims = 0;
        if (bbox[1] > bbox[0])
        {
            vol *= (bbox[1]-bbox[0]);
            numDims++;
        }
        if (bbox[3] > bbox[2])
        {
            vol *= (bbox[3]-bbox[2]);
            numDims++;
        }
        if (bbox[5] > bbox[4])
        {
            vol *= (bbox[5]-bbox[4]);
            numDims++;
        }

        double length = pow(vol, 1.0/numDims);
        return length;
    }
}


// ****************************************************************************
//  Method: avtLineGlyphFilter constructor
//
//  Programmer: Kathleen Biagas
//  Creation:   June 4, 2020
//
//  Modifications:
//
// ****************************************************************************

avtLineGlyphFilter::avtLineGlyphFilter() : labelPrefix()
{
    keepNonLine = false;
}


// ****************************************************************************
//  Method: avtLineGlyphFilter destructor
//
//  Programmer: Kathleen Biagas
//  Creation:   June 4, 2020
//
//  Modifications:
//
// ****************************************************************************

avtLineGlyphFilter::~avtLineGlyphFilter()
{
}


// ****************************************************************************
//  Method: avtLineGlyphFilter::SetLineGlyphAtts
//
//  Purpose:    Sets the LineGlyphAttributes needed for this filter.
//
//  Programmer: Kathleen Biagas
//  Creation:   June 4, 2020
//
// ****************************************************************************

void
avtLineGlyphFilter::SetLineGlyphAtts(const LineGlyphAttributes *atts)
{
    lineGlyphAtts = *atts;
}


// ****************************************************************************
//  Method: avtLineGlyphFilter::SetLabelPrefix
//
//  Purpose:    Sets the label prefix to use when creating dataset labels.
//
//  Programmer: Kathleen Biagas
//  Creation:   June 4, 2020
//
// ****************************************************************************

void
avtLineGlyphFilter::SetLabelPrefix(const string &lp)
{
    labelPrefix = lp;
}


// ****************************************************************************
//  Method: avtLineGlyphFilter::SetKeepNonLine
//
//  Purpose:    Sets the flag indicating whether or not to keep non-line cells.
//
//  Programmer: Kathleen Biagas
//  Creation:   June 4, 2020
//
// ****************************************************************************

void
avtLineGlyphFilter::SetKeepNonLine(const bool keep)
{
    keepNonLine = keep;
}


// ****************************************************************************
//  Method: avtLineGlyphFilter::ExecuteData
//
//  Purpose:
//      Separates vertex data into separate dataset for processing by the
//      glyph mapper.  Separates lines and applies glyphs to them.
//
//  Arguments:
//      inDR    The input data representation.
//
//  Returns:    The output data tree.
//
//  Programmer: Kathleen Biagas
//  Creation:   June 4, 2020
//
// ****************************************************************************

avtDataTree_p
avtLineGlyphFilter::ExecuteDataTree(avtDataRepresentation *inDR)
{
    //
    // Get the VTK data set.
    //
    vtkDataSet *inDS = inDR->GetDataVTK();
    int domain       = inDR->GetDomain();
    string label     = inDR->GetLabel();

    if ((GetInput()->GetInfo().GetAttributes().GetTopologicalDimension() == 0) ||
        (label.compare(0, labelPrefix.size()+8, labelPrefix+string("_points_")) == 0))
    {
        // nothing to do here, input is all points,
        return new avtDataTree(inDS, domain, label);
    }

    if (inDS->GetDataObjectType() != VTK_POLY_DATA &&
        inDS->GetDataObjectType() != VTK_UNSTRUCTURED_GRID)
    {
        // nothing to do here
        return new avtDataTree(inDS, domain, label);
    }

    if (lineGlyphAtts.GetLineType()  == LineGlyphAttributes::Line &&
        lineGlyphAtts.GetHeadStyle() == LineGlyphAttributes::None &&
        lineGlyphAtts.GetTailStyle() == LineGlyphAttributes::None)
    {
        // nothing to do here
        return new avtDataTree(inDS, domain, label);
    }

    vtkPolyData *processedLines = nullptr;
    bool removeLinesFromInput = false;

    ProcessLines(inDS, processedLines, removeLinesFromInput);

    if (processedLines == nullptr)
    {
        // nothing was done
        return new avtDataTree(inDS, domain, label);
    }


    vtkDataSet *surfaceData = nullptr;
    if (keepNonLine)
    {
        surfaceData = inDS;
        if (removeLinesFromInput)
        {
            vtkNew<vtkExtractCellsByType> remover;
            remover->SetInputData(inDS);
            // want all cell types
            remover->AddAllCellTypes();
            // except Lines
            remover->RemoveCellType(VTK_LINE);
            remover->RemoveCellType(VTK_POLY_LINE);
            remover->Update();
            surfaceData = remover->GetOutput();
            // double-check that remover produced cells.
            if (surfaceData->GetNumberOfCells() > 0)
                surfaceData->Register(NULL);
            else
                surfaceData = nullptr;
        }
    }

    // Check if label indicates it was previously processed by avtVertexGeometryFilter
    string surfLabel = labelPrefix + string("_surf_");
    string linesLabel = labelPrefix + string("_lines_");
    if (label.compare(0, surfLabel.size(), surfLabel) == 0)
       label = label.substr(surfLabel.size());

    vector<vtkDataSet *> outs;
    stringVector l;

    if (surfaceData != nullptr)
    {
        outs.push_back(surfaceData);
        l.push_back(surfLabel + label);
    }
    outs.push_back(processedLines);
    l.push_back(linesLabel + label);

    return new avtDataTree(outs.size(), &outs[0], domain, l);
}



// ****************************************************************************
//  Method: avtLineGlyphFilter::UpdateDataObjectInfo
//
//  Purpose:  Sets flags in the pipeline.
//
//  Programmer: Kathleen Biagas
//  Creation:   June 4, 2020
//
//  Modifications:
//
// ****************************************************************************

void
avtLineGlyphFilter::UpdateDataObjectInfo(void)
{
    if (GetInput()->GetInfo().GetAttributes().GetTopologicalDimension() < 2)
        GetOutput()->GetInfo().GetValidity().InvalidateZones();
}


// ****************************************************************************
//  Method: avtLineGlyphFilter::PostExcecute
//
//  Purpose:
//    Sets the output's label attributes to reflect what is currently
//    present in the tree.
//
//  Programmer: Kathleen Biagas
//  Creation:   June 4, 2020
//
//  Modifications:
//
// ****************************************************************************

void
avtLineGlyphFilter::PostExecute(void)
{
    // Use labels to ensure lines/polys/verts aren't merged back together
    // during CompactTreeFilter
    stringVector treeLabels;
    GetDataTree()->GetAllUniqueLabels(treeLabels);
    GetOutput()->GetInfo().GetAttributes().SetLabels(treeLabels);
}


// ****************************************************************************
//  Method: avtLineGlyphFilter::ProcessLines
//
//  Purpose:
//    Applies glyphs to line cells and line endpoints.
//
//  Arguments:
//    inputDS               The input.
//    processedLines        Place to store the dataset containing glyphed
//                          lines and/or line endpoints.
//    removeLinessFromInput Flag indicating if line cells should be removed
//                          from the input dataset.
//
//  Programmer: Kathleen Biagas
//  Creation:   June 4, 2020
//
//  Modifications:
//
// ****************************************************************************

void
avtLineGlyphFilter::ProcessLines(const vtkDataSet *inputDS,
    vtkPolyData *&processedLines, bool &removeLinesFromInput)
{
    //
    // Find out whether we can/should apply glyphs.
    //
    bool glyphingLines = lineGlyphAtts.GetLineType() != LineGlyphAttributes::Line;
    bool glyphingEnds  =
        !(lineGlyphAtts.GetTailStyle() == LineGlyphAttributes::None &&
          lineGlyphAtts.GetHeadStyle() == LineGlyphAttributes::None);

    if (!glyphingLines && !glyphingEnds)
    {
        removeLinesFromInput = false;
        return;
    }

    vtkDataSet *inDS = const_cast<vtkDataSet *>(inputDS);

    // does the input contain line cells?
    bool allLines = false;
    bool someLines = false;

    if (inDS->GetDataObjectType() == VTK_POLY_DATA)
    {
        // Check for verts only, create label if so
        vtkPolyData *pd  = vtkPolyData::SafeDownCast(inDS);
        if (pd->GetNumberOfLines() == pd->GetNumberOfCells())
        {
            allLines = true;
        }
        else if (pd->GetNumberOfLines() > 0)
        {
            someLines = true;
        }
    }
    else if (inDS->GetDataObjectType() == VTK_UNSTRUCTURED_GRID)
    {
        vtkUnstructuredGrid *ug  = vtkUnstructuredGrid::SafeDownCast(inDS);
        if (ug->IsHomogeneous() && (ug->GetCellType(0) == VTK_LINE ||
            ug->GetCellType(0) == VTK_POLY_LINE))
        {
            allLines = true;
        }
        else
        {
            // will assume there may be some line cells
            someLines = true;
        }
    }

    if (!allLines && !someLines)
    {
        removeLinesFromInput = false;
        return;
    }

    vtkPolyData *inPolys = NULL;
    if (inDS->GetDataObjectType() == VTK_POLY_DATA)
    {
        inPolys = vtkPolyData::SafeDownCast(inDS);
    }
    else  // already checked for polydata/ugrid, so this is the ugrid case
    {
        // convert it to polydata
        vtkNew<vtkGeometryFilter> geo;
        geo->SetInputData(inDS);
        geo->Update();
        inPolys = geo->GetOutput();
        inPolys->Register(NULL);
    }

    //
    // Verify there are lines
    //
    if (inPolys->GetNumberOfLines() < 1)
    {
        inPolys->Delete();
        removeLinesFromInput = false;
        return;
    }

    //
    // If input contains more than lines, want to extract lines for processing
    //
    vtkPolyData *linesOnly = inPolys;
    if ((glyphingLines || glyphingEnds) &&
        inPolys->GetNumberOfLines() < inPolys->GetNumberOfCells())
    {
        vtkNew<vtkExtractCellsByType> extractLines;
        extractLines->AddCellType(VTK_LINE);
        extractLines->AddCellType(VTK_POLY_LINE);
        extractLines->SetInputData(inPolys);
        extractLines->Update();
        linesOnly = vtkPolyData::SafeDownCast(extractLines->GetOutput());
        linesOnly->Register(NULL);
    }
    removeLinesFromInput = glyphingLines;

    // Get bounding box size, used for scaling tubes, ribbons, line-end glyphs
    double bbox[6] = {0.,1.,0.,1.,0.,1.};
    GetInput()->GetInfo().GetAttributes().GetOriginalSpatialExtents()->
        CopyTo(bbox);
    double bboxSize = GetBBoxSize(bbox);

    //
    // Handle tubes or ribbons applied to lines
    //
    vtkNew<vtkPolyData> glyphedLines;
    if (lineGlyphAtts.GetLineType() == LineGlyphAttributes::Tube)
    {
        AddTubes(linesOnly, glyphedLines, bboxSize);
    }
    else if (lineGlyphAtts.GetLineType() == LineGlyphAttributes::Ribbon)
    {
        AddRibbons(linesOnly, glyphedLines, bboxSize);
    }

    //
    // Handle glyphs applied to line endpoints
    //
    vtkNew<vtkPolyData> endPoints;
    if (glyphingEnds)
    {
        AddEndPoints(linesOnly, endPoints, bboxSize);
    }

    //
    // Append all glyphed output (tubes/ribbons, endpoints) into a single
    // dataset.
    //
    if (glyphedLines->GetNumberOfCells() > 0 &&
        endPoints->GetNumberOfCells() > 0)
    {
        vtkNew<vtkAppendPolyData>appender;
        appender->AddInputData(glyphedLines);
        appender->AddInputData(endPoints);
        appender->Update();
        processedLines = appender->GetOutput();
        processedLines->Register(NULL);
    }
    else if (glyphedLines->GetNumberOfCells() > 0)
    {
        processedLines = glyphedLines;
        processedLines->Register(NULL);
    }
    else if (endPoints->GetNumberOfCells() > 0)
    {
        processedLines = endPoints;
        processedLines->Register(NULL);
    }
}


// ****************************************************************************
//  Method: avtLineGlyphFilter::AddTubes
//
//  Purpose:  Applies vtkTubeFilter to input.
//
//  Notes:    Content taken from avtPolylineToTubeFilter.
//
//  Arguments:
//    input     The input. (Original input minus all but line cells).
//    ouput     A place to store the output.
//    bboxSize  The size of the bounding box.
//
//  Programmer: Kathleen Biagas
//  Creation:   June 4, 2020
//
//  Modifications:
//
// ****************************************************************************

void
avtLineGlyphFilter::AddTubes(vtkPolyData *input,
                                       vtkPolyData *output,
                                       double bboxSize)
{
    vtkNew<vtkTubeFilter> tubeFilter;
    tubeFilter->SetInputData(input);
    if (lineGlyphAtts.GetTubeRadiusSizeType() == LineGlyphAttributes::Absolute)
        tubeFilter->SetRadius(lineGlyphAtts.GetTubeRadiusAbsolute());
    else
        tubeFilter->SetRadius(lineGlyphAtts.GetTubeRadiusBBox() *bboxSize);
    tubeFilter->SetNumberOfSides(lineGlyphAtts.GetTubeResolution());
    tubeFilter->SetCapping(1);

    if (lineGlyphAtts.GetTubeRadiusVarEnabled())
    {
        string radiusVar = lineGlyphAtts.GetTubeRadiusVar();
        if (!radiusVar.empty() && radiusVar != "default")
        {
            int fieldAssociation = vtkDataObject::FIELD_ASSOCIATION_POINTS;
            if (input->GetCellData()->HasArray(radiusVar.c_str()))
            {
                fieldAssociation = vtkDataObject::FIELD_ASSOCIATION_CELLS;
            }
            tubeFilter->SetInputArrayToProcess(0, 0,0, fieldAssociation,
                                               radiusVar.c_str());
        }
        tubeFilter->SetVaryRadiusToVaryRadiusByScalar();
        tubeFilter->SetRadiusFactor(lineGlyphAtts.GetTubeRadiusVarRatio());
    }
    tubeFilter->Update();

    output->ShallowCopy(tubeFilter->GetOutput());
}


// ****************************************************************************
//  Method: avtLineGlyphFilter::AddRibbons
//
//  Purpose:  Applies vtkRibbonFilter to input, stores in output.
//
//  Notes:    Content taken from avtPolylineToRibbonFilter.
//
//  Arguments:
//  Arguments:
//    input     The input. (Original input minus all but line cells).
//    ouput     A place to store the output.
//    bboxSize  The size of the bounding box.
//
//  Programmer: Kathleen Biagas
//  Creation:   June 4, 2020
//
//  Modifications:
//
// ****************************************************************************

void
avtLineGlyphFilter::AddRibbons(vtkPolyData *input,
                                         vtkPolyData *output,
                                         double bboxSize)
{
    vtkNew<vtkRibbonFilter> ribbonFilter;
    ribbonFilter->SetInputData(input);
    if(lineGlyphAtts.GetTubeRadiusSizeType() == LineGlyphAttributes::Absolute)
        ribbonFilter->SetWidth(lineGlyphAtts.GetTubeRadiusAbsolute());
    else
        ribbonFilter->SetWidth(lineGlyphAtts.GetTubeRadiusBBox()*bboxSize);

    ribbonFilter->SetVaryWidth(lineGlyphAtts.GetTubeRadiusVarEnabled());

    if (lineGlyphAtts.GetTubeRadiusVarEnabled())
    {
        string widthVar(lineGlyphAtts.GetTubeRadiusVar());
        if (widthVar != "" && widthVar != "\0" && widthVar != "default")
        {
            int fieldAssociation = vtkDataObject::FIELD_ASSOCIATION_POINTS;
            if (input->GetCellData()->HasArray(widthVar.c_str()))
            {
                fieldAssociation = vtkDataObject::FIELD_ASSOCIATION_CELLS;
            }
            ribbonFilter->SetInputArrayToProcess(0, 0,0, fieldAssociation,
                                                 widthVar.c_str());
        }
    }
    ribbonFilter->Update();

    output->ShallowCopy(ribbonFilter->GetOutput());
}


// ****************************************************************************
//  Method: avtLineGlyphFilter::AddEndPoints
//
//  Purpose:  Applies glyphs to line endpoints.
//
//  Notes:    Content taken from avtPolylineAddEndPointsFilter.
//
//  Arguments:
//    input     The input. (Original input minus all but line cells).
//    ouput     A place to store the output.
//    bboxSize  The size of the bounding box.
//
//  Programmer: Kathleen Biagas
//  Creation:   June 4, 2020
//
//  Modifications:
//    Kathleen Biagas, Thu Aug 11 2022
//    Support VTK9: use vtkCellArrayIterator.
//
//    Kathleen Biagas, Thu Dec 14, 2023
//    Don't increment lineIndex in the for loop statement (VTK 9), as it is
//    incremented at the bottom of the loop.
//
// ****************************************************************************

void
avtLineGlyphFilter::AddEndPoints(vtkPolyData *input, vtkPolyData *output,
                                 double bboxSize)
{
    vtkNew<vtkAppendPolyData> appender;
    double radius = 0.;
    if(lineGlyphAtts.GetEndPointRadiusSizeType() == LineGlyphAttributes::Absolute)
        radius = lineGlyphAtts.GetEndPointRadiusAbsolute();
    else
        radius = lineGlyphAtts.GetEndPointRadiusBBox() * bboxSize;

    const avtDataAttributes &datts = GetInput()->GetInfo().GetAttributes();
    string activeVar = datts.GetVariableName();

    double ratio          = lineGlyphAtts.GetEndPointRatio();
    bool   varyRadius     = lineGlyphAtts.GetEndPointRadiusVarEnabled();
    string radiusVar      = lineGlyphAtts.GetEndPointRadiusVar();
    double radiusFactor   = lineGlyphAtts.GetEndPointRadiusVarRatio();
    int    resolution     = lineGlyphAtts.GetEndPointResolution();

    vtkDataArray *radiusArray = NULL;
    double range[2] = {0,1}, scale = 1;

    if (varyRadius && !radiusVar.empty())
    {
        if (radiusVar == "default")
            radiusVar = activeVar;

        radiusArray = input->GetPointData()->GetArray(radiusVar.c_str());
        if (!radiusArray)
            radiusArray = input->GetCellData()->GetArray(radiusVar.c_str());

        radiusArray->GetRange(range, 0);

        if ((range[1] - range[0]) == 0.0)
            range[1] = range[0] + 1.0;

        scale = (radiusFactor-1) / (range[1]-range[0]);
    }

    vtkCellData  *inputCellData  = input->GetCellData();
    vtkPointData *inputPointData = input->GetPointData();
    vtkPoints    *points = input->GetPoints();
    vtkIdType     numPts;
    vtkIdType     lineIndex = 0;

    const vtkIdType *ptIndexs;

    auto lines = vtk::TakeSmartPointer(input->GetLines()->NewIterator());
    for (lines->GoToFirstCell(); !lines->IsDoneWithTraversal(); lines->GoToNextCell())
    {
        lines->GetCurrentCell(numPts, ptIndexs);
        vtkPolyData *outPD;

        double p0[3], p1[3];

        avtVector vec;

        // Do the two endpoints in a loop. The first iteration is for the
        // tail and the second is for the head.
        for (int i = 0; i < 2; ++i)
        {
            if ((i == 0 && lineGlyphAtts.GetTailStyle() != LineGlyphAttributes::None) ||
                (i == 1 && lineGlyphAtts.GetHeadStyle() != LineGlyphAttributes::None))
            {
                int style, tip, tail;

                if (i == 0)
                {
                    style = lineGlyphAtts.GetTailStyle();
                    tip  = 0;
                    tail = 1;
                }
                else
                {
                    style = lineGlyphAtts.GetHeadStyle();
                    tip  = numPts - 1;
                    tail = numPts - 2;
                }

                points->GetPoint(ptIndexs[tip], p0);

                double scaledRadius = radius;

                if (varyRadius && radiusArray)
                {
                    scaledRadius *=
                      (1.0 + (radiusArray->GetComponent( ptIndexs[tip], 0 ) -
                              range[0]) * scale);
                }

                if (style == LineGlyphAttributes::Spheres)
                {
                    vtkNew<vtkSphereSource> sphere;

                    sphere->SetRadius(scaledRadius);
                    sphere->SetPhiResolution(resolution);
                    sphere->SetThetaResolution(resolution);
                    sphere->SetCenter(p0);
                    sphere->Update();

                    outPD = sphere->GetOutput();
                    outPD->Register(NULL);

                }
                else if (style == LineGlyphAttributes::Cones)
                {
                    points->GetPoint(ptIndexs[tail], p1);

                    vec = avtVector(p0[0]-p1[0], p0[1]-p1[1],  p0[2]-p1[2]);
                    vec.normalize();

                    p0[0] += (vec * scaledRadius * ratio / 2.0).x;
                    p0[1] += (vec * scaledRadius * ratio / 2.0).y;
                    p0[2] += (vec * scaledRadius * ratio / 2.0).z;

                    vtkNew<vtkConeSource> cone;

                    cone->SetRadius(scaledRadius);
                    cone->SetHeight(scaledRadius * ratio);
                    cone->SetResolution(resolution);
                    cone->SetCenter(p0);
                    cone->SetDirection(vec.x, vec.y, vec.z);
                    cone->CappingOn();
                    cone->Update();

                    outPD = cone->GetOutput();
                    outPD->Register(NULL);
                }
                int npts   = outPD->GetNumberOfPoints();
                int ncells = outPD->GetNumberOfCells();

                vtkCellData  *outputCellData  = outPD->GetCellData();
                vtkPointData *outputPointData = outPD->GetPointData();

                // remove the generated normals
                outputPointData->RemoveArray("Normals");

                // Copy over all of the point data from the lines to the
                // glyph's points

                int nArrays = inputPointData->GetNumberOfArrays();
                for (int j = 0; j < nArrays; ++j)
                {
                    vtkDataArray *array = inputPointData->GetArray(j);
                    vtkDataArray *scalars = array->NewInstance();
                    scalars->Allocate(npts);
                    scalars->SetName(array->GetName());

                    outputPointData->AddArray(scalars);
                    if (array->GetName() == activeVar)
                    {
                        outputPointData->SetActiveScalars(activeVar.c_str());
                    }

                    double scalar = array->GetComponent(ptIndexs[tip], 0);

                    for (int k = 0; k < npts; ++k)
                        scalars->InsertTuple(k, array->GetTuple(ptIndexs[tip]));

                    scalars->Delete();
                }

                // Copy over all of the cell data from the lines to the
                // glyph's cells.
                nArrays = inputCellData->GetNumberOfArrays();
                for (int j = 0; j < nArrays; ++j)
                {
                    vtkDataArray *array = inputCellData->GetArray(j);
                    vtkDataArray *scalars = array->NewInstance();
                    scalars->Allocate(ncells);
                    scalars->SetName(array->GetName());
                    outputCellData->AddArray(scalars);
                    if (array->GetName() == activeVar)
                    {
                        outputCellData->SetActiveScalars(activeVar.c_str());
                    }
                    for (int k = 0; k < ncells; ++k)
                        scalars->InsertTuple(k, array->GetTuple(lineIndex));

                    scalars->Delete();
                }
                appender->AddInputData(outPD);

                outPD->Delete();
            }
        }
        ++lineIndex;
    }
    appender->Update();
    output->ShallowCopy(appender->GetOutput());
}

