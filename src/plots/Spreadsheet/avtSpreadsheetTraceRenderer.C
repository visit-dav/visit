// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#include <avtSpreadsheetTraceRenderer.h>

#include <algorithm>

#include <vtkActor.h>
#include <vtkCell.h>
#include <vtkCellArray.h>
#include <vtkDataArray.h>
#include <vtkFieldData.h>
#include <vtkIdList.h>
#include <vtkMapper.h>
#include <vtkPointData.h>
#include <vtkPoints.h>
#include <vtkPolyData.h>
#include <vtkPolyDataMapper.h>
#include <vtkProperty.h>
#include <vtkRectilinearGrid.h>
#include <vtkRenderer.h>
#include <vtkStructuredGrid.h>
#include <vtkWindow.h>

#include <DebugStream.h>
#include <PickAttributes.h>

namespace
{
vtkPolyData *
NewPolyData()
{
    vtkPolyData *pd = vtkPolyData::New();
    vtkPoints *pts = vtkPoints::New();
    vtkCellArray *polys = vtkCellArray::New();
    vtkCellArray *lines = vtkCellArray::New();

    pd->SetPoints(pts);
    pd->SetPolys(polys);
    pd->SetLines(lines);

    pts->Delete();
    polys->Delete();
    lines->Delete();
    return pd;
}

vtkIdType
AddPoint(vtkPolyData *pd, const double p[3])
{
    return pd->GetPoints()->InsertNextPoint(p);
}

void
AddQuad(vtkPolyData *pd, const double p0[3], const double p1[3],
        const double p2[3], const double p3[3])
{
    vtkIdType ids[4];
    ids[0] = AddPoint(pd, p0);
    ids[1] = AddPoint(pd, p1);
    ids[2] = AddPoint(pd, p2);
    ids[3] = AddPoint(pd, p3);
    pd->GetPolys()->InsertNextCell(4, ids);
}

void
AddLine(vtkPolyData *pd, const double p0[3], const double p1[3])
{
    vtkIdType ids[2];
    ids[0] = AddPoint(pd, p0);
    ids[1] = AddPoint(pd, p1);
    pd->GetLines()->InsertNextCell(2, ids);
}

void
AddLineLoop(vtkPolyData *pd, const double p0[3], const double p1[3],
            const double p2[3], const double p3[3])
{
    AddLine(pd, p0, p1);
    AddLine(pd, p1, p2);
    AddLine(pd, p2, p3);
    AddLine(pd, p3, p0);
}

void
AddCellEdges(vtkPolyData *pd, vtkCell *cell)
{
    if(cell == 0)
        return;

    if(cell->GetCellDimension() <= 1)
    {
        vtkPoints *pts = cell->GetPoints();
        if(pts == 0 || pts->GetNumberOfPoints() < 2)
            return;
        for(vtkIdType i = 0; i < pts->GetNumberOfPoints() - 1; ++i)
            AddLine(pd, pts->GetPoint(i), pts->GetPoint(i + 1));
    }
    else if(cell->GetCellDimension() == 2)
    {
        const int nEdges = cell->GetNumberOfEdges();
        if(nEdges > 0)
        {
            for(int e = 0; e < nEdges; ++e)
            {
                vtkCell *edge = cell->GetEdge(e);
                if(edge != 0 && edge->GetPoints() != 0 &&
                   edge->GetPoints()->GetNumberOfPoints() >= 2)
                {
                    AddLine(pd, edge->GetPoints()->GetPoint(0),
                                edge->GetPoints()->GetPoint(1));
                }
            }
        }
        else
        {
            vtkPoints *pts = cell->GetPoints();
            if(pts == 0 || pts->GetNumberOfPoints() < 2)
                return;
            for(vtkIdType i = 0; i < pts->GetNumberOfPoints(); ++i)
            {
                AddLine(pd, pts->GetPoint(i),
                            pts->GetPoint((i + 1) % pts->GetNumberOfPoints()));
            }
        }
    }
    else
    {
        for(int f = 0; f < cell->GetNumberOfFaces(); ++f)
        {
            vtkCell *face = cell->GetFace(f);
            if(face == 0)
                continue;
            for(int e = 0; e < face->GetNumberOfEdges(); ++e)
            {
                vtkCell *edge = face->GetEdge(e);
                if(edge != 0 && edge->GetPoints() != 0 &&
                   edge->GetPoints()->GetNumberOfPoints() >= 2)
                {
                    AddLine(pd, edge->GetPoints()->GetPoint(0),
                                edge->GetPoints()->GetPoint(1));
                }
            }
        }
    }
}

int
ClampIndex(int index, int upper)
{
    if(upper <= 0)
        return 0;
    return std::max(0, std::min(index, upper));
}

int
AxisCellCount(int dim)
{
    return dim > 1 ? dim - 1 : 1;
}

bool
IsZonePick(int pickType)
{
    return pickType == PickAttributes::Zone ||
           pickType == PickAttributes::CurveZone ||
           pickType == PickAttributes::DomainZone ||
           pickType == PickAttributes::ZoneLabel;
}

bool
IsNodePick(int pickType)
{
    return pickType == PickAttributes::Node ||
           pickType == PickAttributes::CurveNode ||
           pickType == PickAttributes::DomainNode ||
           pickType == PickAttributes::NodeLabel;
}

void
GetBaseIndex(vtkDataSet *ds, int baseIndex[3])
{
    baseIndex[0] = baseIndex[1] = baseIndex[2] = 0;

    vtkFieldData *fd = (ds != 0) ? ds->GetFieldData() : 0;
    vtkDataArray *base = (fd != 0) ? fd->GetArray("base_index") : 0;
    if(base != 0 && base->GetNumberOfTuples() == 3)
    {
        baseIndex[0] = static_cast<int>(base->GetTuple1(0));
        baseIndex[1] = static_cast<int>(base->GetTuple1(1));
        baseIndex[2] = static_cast<int>(base->GetTuple1(2));
    }

    vtkDataArray *realDims = (fd != 0) ? fd->GetArray("avtRealDims") : 0;
    if(realDims != 0 && realDims->GetNumberOfTuples() == 6)
    {
        baseIndex[0] -= static_cast<int>(realDims->GetTuple1(0));
        baseIndex[1] -= static_cast<int>(realDims->GetTuple1(2));
        baseIndex[2] -= static_cast<int>(realDims->GetTuple1(4));
    }
}

bool
LogicalIndexFromLinearId(int id, const int dims[3], bool cellId, int ijk[3])
{
    const int nx = cellId ? AxisCellCount(dims[0]) : dims[0];
    const int ny = cellId ? AxisCellCount(dims[1]) : dims[1];
    const int nz = cellId ? AxisCellCount(dims[2]) : dims[2];
    if(nx < 1 || ny < 1 || nz < 1)
        return false;

    const int nxy = nx * ny;
    const int nvals = nxy * nz;
    if(id < 0 || id >= nvals)
        return false;

    ijk[2] = id / nxy;
    const int offset = id - (ijk[2] * nxy);
    ijk[1] = offset / nx;
    ijk[0] = offset % nx;
    return true;
}

bool
LogicalIndexFromPick(vtkDataSet *ds, const SpreadsheetAttributes &atts,
    const int dims[3], int ijk[3], bool &zonePick)
{
    ijk[0] = ijk[1] = ijk[2] = -1;
    const int pickType = atts.GetCurrentPickType();
    zonePick = IsZonePick(pickType);
    if(!zonePick && !IsNodePick(pickType))
        return false;

    int baseIndex[3];
    GetBaseIndex(ds, baseIndex);

    const int pickId = atts.GetCurrentPick();
    if(LogicalIndexFromLinearId(pickId - baseIndex[0], dims, zonePick, ijk))
        return true;
    if(baseIndex[0] != 0 &&
       LogicalIndexFromLinearId(pickId, dims, zonePick, ijk))
        return true;
    if(baseIndex[0] != 1 &&
       LogicalIndexFromLinearId(pickId - 1, dims, zonePick, ijk))
        return true;

    return false;
}

vtkIdType
CellIdFromLogicalIndex(const int ijk[3], const int dims[3], bool zonePick)
{
    const int cellDims[3] = {AxisCellCount(dims[0]),
                             AxisCellCount(dims[1]),
                             AxisCellCount(dims[2])};
    const int i = zonePick ? ijk[0] : ClampIndex(ijk[0], cellDims[0] - 1);
    const int j = zonePick ? ijk[1] : ClampIndex(ijk[1], cellDims[1] - 1);
    const int k = zonePick ? ijk[2] : ClampIndex(ijk[2], cellDims[2] - 1);

    if(i < 0 || i >= cellDims[0] ||
       j < 0 || j >= cellDims[1] ||
       k < 0 || k >= cellDims[2])
        return -1;

    return static_cast<vtkIdType>(
        (k * cellDims[1] * cellDims[0]) + (j * cellDims[0]) + i);
}

vtkIdType
LocalElementIdFromPick(vtkDataSet *ds, const SpreadsheetAttributes &atts,
    bool zonePick)
{
    const vtkIdType nvals =
        zonePick ? ds->GetNumberOfCells() : ds->GetNumberOfPoints();
    if(nvals < 1)
        return -1;

    int baseIndex[3];
    GetBaseIndex(ds, baseIndex);

    const vtkIdType pickId = static_cast<vtkIdType>(atts.GetCurrentPick());
    const vtkIdType id = pickId - baseIndex[0];
    if(id >= 0 && id < nvals)
        return id;
    if(baseIndex[0] != 0 && pickId >= 0 && pickId < nvals)
        return pickId;
    if(baseIndex[0] != 1 && pickId > 0 && pickId - 1 < nvals)
        return pickId - 1;

    return -1;
}

double
BoundValue(vtkDataArray *bounds, int i)
{
    return bounds->GetTuple1(i);
}

double
CoordinateForSlice(vtkDataArray *coords, int slice, bool cellCentered,
                   double fallbackMin, double fallbackMax)
{
    if(coords == 0 || coords->GetNumberOfTuples() < 1)
        return (fallbackMin + fallbackMax) / 2.;

    const int ncoords = static_cast<int>(coords->GetNumberOfTuples());
    int index = ClampIndex(slice, ncoords - 1);
    if(cellCentered && ncoords > 1)
    {
        index = ClampIndex(slice, ncoords - 2);
        return (coords->GetTuple1(index) + coords->GetTuple1(index + 1)) / 2.;
    }

    return coords->GetTuple1(index);
}

void
AddBoundingBox(vtkPolyData *pd, vtkDataArray *bounds)
{
    const double x0 = BoundValue(bounds, 0);
    const double x1 = BoundValue(bounds, 1);
    const double y0 = BoundValue(bounds, 2);
    const double y1 = BoundValue(bounds, 3);
    const double z0 = BoundValue(bounds, 4);
    const double z1 = BoundValue(bounds, 5);
    const double p000[3] = {x0, y0, z0};
    const double p100[3] = {x1, y0, z0};
    const double p110[3] = {x1, y1, z0};
    const double p010[3] = {x0, y1, z0};
    const double p001[3] = {x0, y0, z1};
    const double p101[3] = {x1, y0, z1};
    const double p111[3] = {x1, y1, z1};
    const double p011[3] = {x0, y1, z1};

    AddLineLoop(pd, p000, p100, p110, p010);
    AddLineLoop(pd, p001, p101, p111, p011);
    AddLine(pd, p000, p001);
    AddLine(pd, p100, p101);
    AddLine(pd, p110, p111);
    AddLine(pd, p010, p011);
}
}

avtSpreadsheetTraceRenderer::avtSpreadsheetTraceRenderer()
{
    surfaceMapper = vtkPolyDataMapper::New();
    surfaceMapper->ScalarVisibilityOff();
    surfaceActor = vtkActor::New();
    surfaceActor->SetMapper(surfaceMapper);
    surfaceActor->PickableOff();
    surfaceActor->GetProperty()->LightingOff();

    tracerLineMapper = vtkPolyDataMapper::New();
    tracerLineMapper->ScalarVisibilityOff();
    tracerLineActor = vtkActor::New();
    tracerLineActor->SetMapper(tracerLineMapper);
    tracerLineActor->PickableOff();
    tracerLineActor->GetProperty()->LightingOff();
    tracerLineActor->GetProperty()->SetLineWidth(2.);

    foregroundLineMapper = vtkPolyDataMapper::New();
    foregroundLineMapper->ScalarVisibilityOff();
    foregroundLineActor = vtkActor::New();
    foregroundLineActor->SetMapper(foregroundLineMapper);
    foregroundLineActor->PickableOff();
    foregroundLineActor->GetProperty()->LightingOff();
    foregroundLineActor->GetProperty()->SetLineWidth(2.);

    currentLineMapper = vtkPolyDataMapper::New();
    currentLineMapper->ScalarVisibilityOff();
    currentLineActor = vtkActor::New();
    currentLineActor->SetMapper(currentLineMapper);
    currentLineActor->PickableOff();
    currentLineActor->GetProperty()->LightingOff();
    currentLineActor->GetProperty()->SetLineWidth(3.);
}

avtSpreadsheetTraceRenderer::~avtSpreadsheetTraceRenderer()
{
    surfaceActor->Delete();
    surfaceMapper->Delete();
    tracerLineActor->Delete();
    tracerLineMapper->Delete();
    foregroundLineActor->Delete();
    foregroundLineMapper->Delete();
    currentLineActor->Delete();
    currentLineMapper->Delete();
}

void
avtSpreadsheetTraceRenderer::ReleaseGraphicsResources(vtkWindow *win)
{
    surfaceActor->ReleaseGraphicsResources(win);
    surfaceMapper->ReleaseGraphicsResources(win);
    tracerLineActor->ReleaseGraphicsResources(win);
    tracerLineMapper->ReleaseGraphicsResources(win);
    foregroundLineActor->ReleaseGraphicsResources(win);
    foregroundLineMapper->ReleaseGraphicsResources(win);
    currentLineActor->ReleaseGraphicsResources(win);
    currentLineMapper->ReleaseGraphicsResources(win);
}

void
avtSpreadsheetTraceRenderer::Render(vtkDataSet *ds, vtkDataArray *bounds,
    vtkRenderer *renderer, const SpreadsheetAttributes &atts,
    const double *fgColor)
{
    vtkPolyData *surface = NewPolyData();
    vtkPolyData *tracerLines = NewPolyData();
    vtkPolyData *foregroundLines = NewPolyData();
    vtkPolyData *currentLines = NewPolyData();

    surfaceMapper->SetInputData(surface);
    tracerLineMapper->SetInputData(tracerLines);
    foregroundLineMapper->SetInputData(foregroundLines);
    currentLineMapper->SetInputData(currentLines);

    const unsigned char *tc = atts.GetTracerColor().GetColor();
    surfaceActor->GetProperty()->SetColor(tc[0] / 255., tc[1] / 255.,
                                          tc[2] / 255.);
    surfaceActor->GetProperty()->SetOpacity(tc[3] / 255.);
    surfaceActor->SetForceTranslucent(tc[3] < 255);
    surfaceActor->SetForceOpaque(tc[3] == 255);
    tracerLineActor->GetProperty()->SetColor(tc[0] / 255., tc[1] / 255.,
                                             tc[2] / 255.);
    tracerLineActor->GetProperty()->SetOpacity(1.);
    tracerLineActor->ForceOpaqueOn();
    tracerLineActor->ForceTranslucentOff();
    foregroundLineActor->GetProperty()->SetColor(fgColor[0], fgColor[1],
                                                 fgColor[2]);
    foregroundLineActor->GetProperty()->SetOpacity(1.);
    foregroundLineActor->ForceOpaqueOn();
    foregroundLineActor->ForceTranslucentOff();
    currentLineActor->GetProperty()->SetColor(fgColor[0], fgColor[1],
                                              fgColor[2]);
    currentLineActor->GetProperty()->SetOpacity(1.);
    currentLineActor->ForceOpaqueOn();
    currentLineActor->ForceTranslucentOff();

    if(ds->IsA("vtkRectilinearGrid"))
    {
        DrawRectilinearGrid((vtkRectilinearGrid *)ds, bounds, atts, fgColor);
    }
    else if(ds->IsA("vtkStructuredGrid"))
    {
        DrawStructuredGrid((vtkStructuredGrid *)ds, bounds, atts, fgColor);
    }
    else
    {
        AddBoundingBox(foregroundLines, bounds);
        if(atts.GetShowCurrentCellOutline() && atts.GetCurrentPickValid())
        {
            if(IsZonePick(atts.GetCurrentPickType()))
            {
                vtkIdType cellId = LocalElementIdFromPick(ds, atts, true);
                DrawCell(cellId >= 0 ? ds->GetCell(cellId) : 0, 0);
            }
            else if(IsNodePick(atts.GetCurrentPickType()))
            {
                vtkIdType nodeId = LocalElementIdFromPick(ds, atts, false);
                vtkIdList *cellIds = vtkIdList::New();
                if(nodeId >= 0)
                    ds->GetPointCells(nodeId, cellIds);
                DrawCell(cellIds->GetNumberOfIds() > 0 ?
                         ds->GetCell(cellIds->GetId(0)) : 0, 0);
                cellIds->Delete();
            }
        }
    }

    surface->Modified();
    tracerLines->Modified();
    foregroundLines->Modified();
    currentLines->Modified();

    if(surface->GetNumberOfCells() > 0 && atts.GetTracerColor().Alpha() > 0)
    {
        if(atts.GetTracerColor().Alpha() < 255)
            surfaceActor->RenderTranslucentPolygonalGeometry(renderer);
        else
            surfaceActor->RenderOpaqueGeometry(renderer);
    }
    if(tracerLines->GetNumberOfCells() > 0)
        tracerLineActor->RenderOpaqueGeometry(renderer);
    if(foregroundLines->GetNumberOfCells() > 0)
        foregroundLineActor->RenderOpaqueGeometry(renderer);
    if(currentLines->GetNumberOfCells() > 0)
    {
        int resolveMode = vtkMapper::GetResolveCoincidentTopology();
        double lineFactor = 0., lineUnits = 0.;
        vtkMapper::GetResolveCoincidentTopologyLineOffsetParameters(
            lineFactor, lineUnits);

        vtkMapper::SetResolveCoincidentTopologyToPolygonOffset();
        vtkMapper::SetResolveCoincidentTopologyLineOffsetParameters(0., -8.);
        currentLineActor->RenderOpaqueGeometry(renderer);

        vtkMapper::SetResolveCoincidentTopology(resolveMode);
        vtkMapper::SetResolveCoincidentTopologyLineOffsetParameters(
            lineFactor, lineUnits);
    }

    surface->Delete();
    tracerLines->Delete();
    foregroundLines->Delete();
    currentLines->Delete();
}

void
avtSpreadsheetTraceRenderer::DrawRectilinearGrid(vtkRectilinearGrid *rgrid,
    vtkDataArray *bounds, const SpreadsheetAttributes &atts,
    const double *)
{
    vtkPolyData *surface = surfaceMapper->GetInput();
    vtkPolyData *tracerLines = tracerLineMapper->GetInput();
    vtkPolyData *foregroundLines = foregroundLineMapper->GetInput();

    int dims[3];
    rgrid->GetDimensions(dims);

    const bool drawCellOutline = atts.GetShowCurrentCellOutline() &&
                                 atts.GetCurrentPickValid();
    int pickIJK[3];
    bool zonePick = false;
    const vtkIdType cellId =
        (drawCellOutline &&
         LogicalIndexFromPick(rgrid, atts, dims, pickIJK, zonePick)) ?
        CellIdFromLogicalIndex(pickIJK, dims, zonePick) : -1;

    if(dims[2] < 2)
    {
        double ext[4] = {BoundValue(bounds, 0), BoundValue(bounds, 1),
                         BoundValue(bounds, 2), BoundValue(bounds, 3)};

        if(dims[1] < 2)
        {
            vtkDataArray *x = rgrid->GetXCoordinates();
            vtkDataArray *y = rgrid->GetPointData()->GetScalars();
            if(x != 0 && y != 0)
            {
                const int nnodes = static_cast<int>(
                    std::min(x->GetNumberOfTuples(), y->GetNumberOfTuples()));
                if(nnodes > 0)
                {
                    double minY = y->GetTuple1(0);
                    double maxY = minY;
                    for(int i = 1; i < nnodes; ++i)
                    {
                        const double yval = y->GetTuple1(i);
                        minY = std::min(minY, yval);
                        maxY = std::max(maxY, yval);
                    }
                    if(minY > 0.)
                        minY = 0.;
                    ext[2] = minY;
                    ext[3] = maxY;

                    if(atts.GetShowTracerPlane() && nnodes > 1)
                    {
                        for(int i = 0; i < nnodes - 1; ++i)
                        {
                            const double p0[3] = {x->GetTuple1(i), minY, 0.};
                            const double p1[3] = {x->GetTuple1(i + 1), minY, 0.};
                            const double p2[3] = {x->GetTuple1(i + 1),
                                                  y->GetTuple1(i + 1), 0.};
                            const double p3[3] = {x->GetTuple1(i),
                                                  y->GetTuple1(i), 0.};
                            AddQuad(surface, p0, p1, p2, p3);
                        }
                    }
                }
            }
        }
        else if(atts.GetShowTracerPlane())
        {
            const double p0[3] = {ext[0], ext[2], 0.};
            const double p1[3] = {ext[1], ext[2], 0.};
            const double p2[3] = {ext[1], ext[3], 0.};
            const double p3[3] = {ext[0], ext[3], 0.};
            AddQuad(surface, p0, p1, p2, p3);
        }

        if(atts.GetShowPatchOutline())
        {
            const double p0[3] = {ext[0], ext[2], 0.};
            const double p1[3] = {ext[1], ext[2], 0.};
            const double p2[3] = {ext[1], ext[3], 0.};
            const double p3[3] = {ext[0], ext[3], 0.};
            AddLineLoop(foregroundLines, p0, p1, p2, p3);
        }

        if(drawCellOutline)
            DrawCell(cellId >= 0 ? rgrid->GetCell(cellId) : 0, 0);
    }
    else
    {
        if(atts.GetShowTracerPlane())
        {
            vtkDataArray *arr = rgrid->GetPointData()->GetScalars();
            const bool cellCentered = (arr == 0);

            if(atts.GetNormal() == SpreadsheetAttributes::X)
            {
                const double x = CoordinateForSlice(rgrid->GetXCoordinates(),
                    atts.GetSliceIndex(), cellCentered,
                    BoundValue(bounds, 0), BoundValue(bounds, 1));
                const double p0[3] = {x, BoundValue(bounds, 2), BoundValue(bounds, 5)};
                const double p1[3] = {x, BoundValue(bounds, 2), BoundValue(bounds, 4)};
                const double p2[3] = {x, BoundValue(bounds, 3), BoundValue(bounds, 4)};
                const double p3[3] = {x, BoundValue(bounds, 3), BoundValue(bounds, 5)};
                AddQuad(surface, p0, p1, p2, p3);
                AddLineLoop(tracerLines, p0, p1, p2, p3);
            }
            else if(atts.GetNormal() == SpreadsheetAttributes::Y)
            {
                const double y = CoordinateForSlice(rgrid->GetYCoordinates(),
                    atts.GetSliceIndex(), cellCentered,
                    BoundValue(bounds, 2), BoundValue(bounds, 3));
                const double p0[3] = {BoundValue(bounds, 0), y, BoundValue(bounds, 4)};
                const double p1[3] = {BoundValue(bounds, 0), y, BoundValue(bounds, 5)};
                const double p2[3] = {BoundValue(bounds, 1), y, BoundValue(bounds, 5)};
                const double p3[3] = {BoundValue(bounds, 1), y, BoundValue(bounds, 4)};
                AddQuad(surface, p0, p1, p2, p3);
                AddLineLoop(tracerLines, p0, p1, p2, p3);
            }
            else if(atts.GetNormal() == SpreadsheetAttributes::Z)
            {
                const double z = CoordinateForSlice(rgrid->GetZCoordinates(),
                    atts.GetSliceIndex(), cellCentered,
                    BoundValue(bounds, 4), BoundValue(bounds, 5));
                const double p0[3] = {BoundValue(bounds, 0), BoundValue(bounds, 2), z};
                const double p1[3] = {BoundValue(bounds, 1), BoundValue(bounds, 2), z};
                const double p2[3] = {BoundValue(bounds, 1), BoundValue(bounds, 3), z};
                const double p3[3] = {BoundValue(bounds, 0), BoundValue(bounds, 3), z};
                AddQuad(surface, p0, p1, p2, p3);
                AddLineLoop(tracerLines, p0, p1, p2, p3);
            }
        }

        if(atts.GetShowPatchOutline())
            AddBoundingBox(foregroundLines, bounds);

        if(drawCellOutline)
            DrawCell(cellId >= 0 ? rgrid->GetCell(cellId) : 0, 0);
    }
}

void
avtSpreadsheetTraceRenderer::DrawStructuredGrid(vtkStructuredGrid *sgrid,
    vtkDataArray *bounds, const SpreadsheetAttributes &atts,
    const double *)
{
    vtkPolyData *surface = surfaceMapper->GetInput();
    vtkPolyData *foregroundLines = foregroundLineMapper->GetInput();

    int dims[3];
    sgrid->GetDimensions(dims);

    const bool drawCellOutline = atts.GetShowCurrentCellOutline() &&
                                 atts.GetCurrentPickValid();
    int pickIJK[3];
    bool zonePick = false;
    const vtkIdType cellId =
        (drawCellOutline &&
         LogicalIndexFromPick(sgrid, atts, dims, pickIJK, zonePick)) ?
        CellIdFromLogicalIndex(pickIJK, dims, zonePick) : -1;

    if(dims[2] < 2)
    {
        if(atts.GetShowTracerPlane() && dims[0] > 1 && dims[1] > 1)
        {
            for(int j = 0; j < dims[1] - 1; ++j)
            {
                vtkIdType row0 = j * dims[0];
                vtkIdType row1 = (j + 1) * dims[0];
                for(int i = 0; i < dims[0] - 1; ++i)
                {
                    AddQuad(surface, sgrid->GetPoint(row0),
                                     sgrid->GetPoint(row0 + 1),
                                     sgrid->GetPoint(row1 + 1),
                                     sgrid->GetPoint(row1));
                    ++row0;
                    ++row1;
                }
            }
        }

        if(atts.GetShowPatchOutline())
        {
            const double p0[3] = {BoundValue(bounds, 0), BoundValue(bounds, 2), 0.};
            const double p1[3] = {BoundValue(bounds, 1), BoundValue(bounds, 2), 0.};
            const double p2[3] = {BoundValue(bounds, 1), BoundValue(bounds, 3), 0.};
            const double p3[3] = {BoundValue(bounds, 0), BoundValue(bounds, 3), 0.};
            AddLineLoop(foregroundLines, p0, p1, p2, p3);
        }

        if(drawCellOutline)
            DrawCell(cellId >= 0 ? sgrid->GetCell(cellId) : 0, 0);
    }
    else
    {
        if(atts.GetShowTracerPlane())
        {
            if(atts.GetNormal() == SpreadsheetAttributes::X && dims[0] > 0)
            {
                const int i = ClampIndex(atts.GetSliceIndex(), dims[0] - 1);
                const int nxny = dims[1] * dims[0];
                for(int j = 0; j < dims[1] - 1; ++j)
                {
                    const int jnx = j * dims[0];
                    const int j1nx = (j + 1) * dims[0];
                    for(int k = 0; k < dims[2] - 1; ++k)
                    {
                        const vtkIdType p0 = k * nxny + jnx + i;
                        const vtkIdType p1 = (k + 1) * nxny + jnx + i;
                        const vtkIdType p2 = (k + 1) * nxny + j1nx + i;
                        const vtkIdType p3 = k * nxny + j1nx + i;
                        AddQuad(surface, sgrid->GetPoint(p0), sgrid->GetPoint(p1),
                                         sgrid->GetPoint(p2), sgrid->GetPoint(p3));
                    }
                }
            }
            else if(atts.GetNormal() == SpreadsheetAttributes::Y && dims[1] > 0)
            {
                const int j = ClampIndex(atts.GetSliceIndex(), dims[1] - 1);
                const int jnx = j * dims[0];
                for(int k = 0; k < dims[2] - 1; ++k)
                {
                    const int knxny = k * dims[1] * dims[0];
                    const int k1nxny = (k + 1) * dims[1] * dims[0];
                    for(int i = 0; i < dims[0] - 1; ++i)
                    {
                        const vtkIdType p0 = knxny + jnx + i;
                        const vtkIdType p1 = knxny + jnx + (i + 1);
                        const vtkIdType p2 = k1nxny + jnx + (i + 1);
                        const vtkIdType p3 = k1nxny + jnx + i;
                        AddQuad(surface, sgrid->GetPoint(p0), sgrid->GetPoint(p1),
                                         sgrid->GetPoint(p2), sgrid->GetPoint(p3));
                    }
                }
            }
            else if(atts.GetNormal() == SpreadsheetAttributes::Z && dims[2] > 0)
            {
                const int k = ClampIndex(atts.GetSliceIndex(), dims[2] - 1);
                const int knxny = k * dims[1] * dims[0];
                for(int j = 0; j < dims[1] - 1; ++j)
                {
                    vtkIdType row0 = knxny + j * dims[0];
                    vtkIdType row1 = knxny + (j + 1) * dims[0];
                    for(int i = 0; i < dims[0] - 1; ++i)
                    {
                        AddQuad(surface, sgrid->GetPoint(row0),
                                         sgrid->GetPoint(row0 + 1),
                                         sgrid->GetPoint(row1 + 1),
                                         sgrid->GetPoint(row1));
                        ++row0;
                        ++row1;
                    }
                }
            }
        }

        if(atts.GetShowPatchOutline())
            AddBoundingBox(foregroundLines, bounds);

        if(drawCellOutline)
            DrawCell(cellId >= 0 ? sgrid->GetCell(cellId) : 0, 0);
    }
}

void
avtSpreadsheetTraceRenderer::DrawBoundingBox(vtkDataArray *bounds,
    const double *)
{
    AddBoundingBox(foregroundLineMapper->GetInput(), bounds);
}

void
avtSpreadsheetTraceRenderer::DrawCell(vtkCell *cell, const double *)
{
    if(cell != 0)
        AddCellEdges(currentLineMapper->GetInput(), cell);
    else
        debug1 << "Internal error: Cannot locate cell." << std::endl;
}
