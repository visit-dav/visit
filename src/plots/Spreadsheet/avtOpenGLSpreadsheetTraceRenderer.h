// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#ifndef AVT_OPEN_GL_SPREADSHEET_TRACER_RENDERER_H
#define AVT_OPEN_GL_SPREADSHEET_TRACER_RENDERER_H
#include <avtSpreadsheetTraceRenderer.h>

class vtkDataArray;
class vtkDataSet;
class vtkRectilinearGrid;
class vtkStructuredGrid;
class vtkCell;

// ****************************************************************************
// Class: avtOpenGLSpreadsheetTraceRenderer
//
// Purpose:
//   OpenGL version of the spreadsheet highlight renderer.
//
// Notes:      
//
// Programmer: Brad Whitlock
// Creation:   Wed Feb 21 09:17:47 PDT 2007
//
// Modifications:
//   
// ****************************************************************************

class avtOpenGLSpreadsheetTraceRenderer : public avtSpreadsheetTraceRenderer
{
public:
    avtOpenGLSpreadsheetTraceRenderer();
    virtual ~avtOpenGLSpreadsheetTraceRenderer();

    virtual void Render(vtkDataSet *, vtkDataArray *, 
                        const SpreadsheetAttributes &, const double *);
private:
    void DrawRectilinearGrid(vtkRectilinearGrid *, vtkDataArray *, 
                             const SpreadsheetAttributes &, const double *);
    void DrawStructuredGrid(vtkStructuredGrid *, vtkDataArray *, 
                            const SpreadsheetAttributes &, const double *);
    void DrawBoundingBox(vtkDataArray *bounds, const double *);
    void Draw2DCell(vtkCell *cell, const double *fgColor);
    void Draw3DCell(vtkCell *cell, const double *fgColor);
};

#endif
