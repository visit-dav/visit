// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ****************************************************************************
//   avtVertexExtractor.C
// ****************************************************************************

#include <avtVertexExtractor.h>

#include <vtkAppendPolyData.h>
#include <vtkExtractCellsByType.h>
#include <vtkPolyData.h>
#include <vtkUnstructuredGrid.h>
#include <vtkVertexFilter.h>

#include <string>
#include <vector>

using std::string;
using std::vector;


// ****************************************************************************
//  Method: avtVertexExtractor constructor
//
//  Programmer: Kathleen Biagas
//  Creation:   June 4, 2020
//
//  Modifications:
//
// ****************************************************************************

avtVertexExtractor::avtVertexExtractor() : labelPrefix()
{
    keepNonVertex = false;
    convertAllPoints = false;
}


// ****************************************************************************
//  Method: avtVertexExtractor destructor
//
//  Programmer: Kathleen Biagas
//  Creation:   June 4, 2020
//
//  Modifications:
//
// ****************************************************************************

avtVertexExtractor::~avtVertexExtractor()
{
}


// ****************************************************************************
//  Method: avtVertexExtractor::SetPointGlyphAtts
//
//  Purpose:    Sets the PointGlyphAttributes needed for this filter.
//
//  Programmer: Kathleen Biagas
//  Creation:   June 4, 2020
//
// ****************************************************************************

void
avtVertexExtractor::SetPointGlyphAtts(const PointGlyphAttributes *atts)
{
    pointGlyphAtts = *atts;
}


// ****************************************************************************
//  Method: avtVertexExtractor::SetLabelPrefix
//
//  Purpose:    Sets the prefix needed when creating dataset labels.
//
//  Programmer: Kathleen Biagas
//  Creation:   June 4, 2020
//
// ****************************************************************************

void
avtVertexExtractor::SetLabelPrefix(const string &prefix)
{
    labelPrefix = prefix;
}


// ****************************************************************************
//  Method: avtVertexExtractor::SetKeepNonVertex
//
//  Purpose:    Sets the flag indicating whether or not to keep non-vertex cells.
//
//  Programmer: Kathleen Biagas
//  Creation:   June 4, 2020
//
// ****************************************************************************

void
avtVertexExtractor::SetKeepNonVertex(const bool keepNV)
{
    keepNonVertex = keepNV;
}


// ****************************************************************************
//  Method: avtVertexExtractor::SetConvertAllPoints
//
//  Purpose:    Sets the flag indicating whether or not convert all points
//              to vertex cells.
//
//  Programmer: Kathleen Biagas
//  Creation:   June 4, 2020
//
// ****************************************************************************

void
avtVertexExtractor::SetConvertAllPoints(const bool convert)
{
    convertAllPoints = convert;
}


// ****************************************************************************
//  Method: avtVertexExtractor::ExecuteData
//
//  Purpose:
//      Separates vertex data into separate dataset for processing by a
//      glyph mapper.
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
avtVertexExtractor::ExecuteDataTree(avtDataRepresentation *inDR)
{
    //
    // Get the VTK data set.
    //
    vtkDataSet *inDS = inDR->GetDataVTK();
    int domain       = inDR->GetDomain();
    string domString = std::to_string(domain);
    string label     = inDR->GetLabel();

    if (GetInput()->GetInfo().GetAttributes().GetTopologicalDimension() == 0)
    {
        // nothing to do here, input is all points
        return new avtDataTree(inDS, domain, label);
    }

    if (inDS->GetDataObjectType() != VTK_POLY_DATA &&
        inDS->GetDataObjectType() != VTK_UNSTRUCTURED_GRID)
    {
        // nothing to do here
        return new avtDataTree(inDS, domain, label);
    }

    if (pointGlyphAtts.GetPointType() == Point)
    {
        // nothing to do here, the mapper can handle points
        return new avtDataTree(inDS, domain, label);
    }

    vtkDataSet *vertsData = nullptr;
    bool removeVertsFromInput = false;
    bool inputIsAllVerts = false;

    // Separate vertex cells into new dataset 'vertsData'
    ProcessPoints(inDS, vertsData, removeVertsFromInput, inputIsAllVerts);
    if (inputIsAllVerts || !keepNonVertex)
    {
        // don't need input in the output, but need to designate as a points
        // dataset (since topological dim doesn't indicate so)
        label = labelPrefix + string("_points_") + label;
        return new avtDataTree(inDS, domain, label);
    }

    if (vertsData == nullptr)
    {
        // nothing was done
        return new avtDataTree(inDS, domain, label);
    }

    vtkDataSet *surfaceData = nullptr;
    if (keepNonVertex)
    {
        surfaceData = inDS;
        if (removeVertsFromInput)
        {
            vtkNew<vtkExtractCellsByType> remover;
            remover->SetInputData(inDS);
            // want all cell types
            remover->AddAllCellTypes();
            // except Verts
            remover->RemoveCellType(VTK_VERTEX);
            remover->RemoveCellType(VTK_POLY_VERTEX);
            remover->Update();
            surfaceData = remover->GetOutput();
            // ugrid data may report 'not all verts' but may truly be all verts
            // so need to double-check that remover produced cells.
            if (surfaceData->GetNumberOfCells() > 0)
                surfaceData->Register(NULL);
            else
                surfaceData = nullptr;
        }
    }

    vector<vtkDataSet *> outs;
    stringVector l;

    if (surfaceData != nullptr)
    {
        outs.push_back(surfaceData);
        l.push_back(labelPrefix + string("_surf_")+ label);
    }
    if (vertsData != nullptr)
    {
        outs.push_back(vertsData);
        l.push_back(labelPrefix + string("_points_")+ label);
    }

    return new avtDataTree(outs.size(), &outs[0], domain, l);
}



// ****************************************************************************
//  Method: avtVertexExtractor::UpdateDataObjectInfo
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
avtVertexExtractor::UpdateDataObjectInfo(void)
{
    if (GetInput()->GetInfo().GetAttributes().GetTopologicalDimension() < 2)
        GetOutput()->GetInfo().GetValidity().InvalidateZones();
}


// ****************************************************************************
//  Method: avtVertexExtractor::PostExcecute
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
avtVertexExtractor::PostExecute(void)
{
    // Use labels to ensure lines/polys/verts aren't merged back together
    // during CompactTreeFilter
    stringVector treeLabels;
    GetDataTree()->GetAllUniqueLabels(treeLabels);
    GetOutput()->GetInfo().GetAttributes().SetLabels(treeLabels);
}


// ****************************************************************************
//  Method: avtVertexExtractor::ProcessPoints
//
//  Purpose:
//    Extracts vertex cells from input.
//
//  Arguments:
//    inputDS               The input.
//    vertsData             Place to store the dataset containing only vertex
//                          cells.
//    removeVertsFromInput  Flag indicating if vertex cells should be removed
//                          from the input dataset.
//    inputIsAllVerts       Flag indicating that input contained only vertex
//                          cells.
//
//  Programmer: Kathleen Biagas
//  Creation:   June 4, 2020
//
//  Modifications:
//
// ****************************************************************************

void
avtVertexExtractor::ProcessPoints(
    const vtkDataSet *inputDS, vtkDataSet *&vertsData,
    bool &removeVertsFromInput, bool &inputIsAllVerts)
{
    vtkDataSet *inDS = const_cast<vtkDataSet*>(inputDS);

    // does the input contain vertex cells?
    bool allVerts = false;
    bool someVerts = false;

    if (inDS->GetDataObjectType() == VTK_POLY_DATA)
    {
        // Check for verts only
        vtkPolyData *pd  = vtkPolyData::SafeDownCast(inDS);
        if (pd->GetNumberOfVerts() == pd->GetNumberOfCells())
        {
            allVerts = true;
        }
        else if (pd->GetNumberOfVerts() > 0)
        {
            someVerts = true;
        }
    }
    else if (inDS->GetDataObjectType() == VTK_UNSTRUCTURED_GRID)
    {
        // Check for verts only
        vtkUnstructuredGrid *ug  = vtkUnstructuredGrid::SafeDownCast(inDS);
        if (ug->IsHomogeneous() && (ug->GetCellType(0) == VTK_VERTEX ||
            ug->GetCellType(0) == VTK_POLY_VERTEX))
        {
            allVerts = true;
        }
        else
        {
            // will assume there may be some vertex cells
            someVerts = true;
        }
    }
    if (allVerts)
    {
        inputIsAllVerts = true;
        return;
    }

    if (!someVerts)
    {
        removeVertsFromInput = false;
        return;
    }

    // May have some verts, so extract them into a new dataset.
    // and determine if vertex cells need to be removed from the input dataset.
    removeVertsFromInput = false;
    vertsData = nullptr;
    if (convertAllPoints)
    {
        // want all points converted to vertex cells
        vtkNew<vtkVertexFilter> vertexFilter;
        vertexFilter->VertexAtPointsOn();
        vertexFilter->SetInputData(inDS);
        vertexFilter->Update();
        vertsData = vertexFilter->GetOutput();
        if(vertsData->GetNumberOfCells() > 0)
        {
            vertsData->Register(NULL);
            removeVertsFromInput = true;
        }
        else
        {
            vertsData = nullptr;
        }
    }
    else
    {
        // want to extract only vertex cells
        vtkNew<vtkExtractCellsByType> extractVerts;
        extractVerts->AddCellType(VTK_VERTEX);
        extractVerts->AddCellType(VTK_POLY_VERTEX);
        extractVerts->SetInputData(inDS);
        extractVerts->Update();
        vertsData = extractVerts->GetOutput();
        if(vertsData->GetNumberOfCells() > 0)
        {
            vertsData->Register(NULL);
            removeVertsFromInput = true;
        }
        else
        {
            vertsData = nullptr;
        }
    }
}

