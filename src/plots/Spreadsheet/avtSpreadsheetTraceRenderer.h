// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#ifndef AVT_SPREADSHEET_TRACER_RENDERER_H
#define AVT_SPREADSHEET_TRACER_RENDERER_H
#include <SpreadsheetAttributes.h>

class vtkDataArray;
class vtkDataSet;
class vtkPolyDataMapper;
class vtkActor;
class vtkRectilinearGrid;
class vtkRenderer;
class vtkStructuredGrid;
class vtkCell;
class vtkWindow;

// ****************************************************************************
// Class: avtSpreadsheetTraceRenderer
//
// Purpose:
//   Renders the spreadsheet highlight plane and outlines.
//
// Notes:
//
// Programmer: Brad Whitlock
// Creation:   Wed Feb 21 09:17:47 PDT 2007
//
// Modifications:
//   Kathleen Biagas, Thu Aug 6, 2026
//   Move logic from avtOpenGLSpreadsheetTraceRenderer to this class, there
//   is no longer a need for a subclass. Update logic so that tracer planes
//   work.  Add vtkRenderer to Render call.  Add ReleaseGrahicsResources.
//
// ****************************************************************************

class avtSpreadsheetTraceRenderer
{
public:
    avtSpreadsheetTraceRenderer();
    virtual ~avtSpreadsheetTraceRenderer();

    void Render(vtkDataSet *, vtkDataArray *, vtkRenderer *,
                const SpreadsheetAttributes &, const double *);
    void ReleaseGraphicsResources(vtkWindow *);
private:
    void DrawRectilinearGrid(vtkRectilinearGrid *, vtkDataArray *,
                             const SpreadsheetAttributes &, const double *);
    void DrawStructuredGrid(vtkStructuredGrid *, vtkDataArray *,
                            const SpreadsheetAttributes &, const double *);
    void DrawBoundingBox(vtkDataArray *bounds, const double *);
    void DrawCell(vtkCell *cell, const double *fgColor);

    vtkPolyDataMapper *surfaceMapper;
    vtkActor          *surfaceActor;
    vtkPolyDataMapper *tracerLineMapper;
    vtkActor          *tracerLineActor;
    vtkPolyDataMapper *foregroundLineMapper;
    vtkActor          *foregroundLineActor;
    vtkPolyDataMapper *currentLineMapper;
    vtkActor          *currentLineActor;
};

#endif
