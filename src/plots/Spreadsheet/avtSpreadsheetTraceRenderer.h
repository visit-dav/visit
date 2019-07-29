// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#ifndef AVT_SPREADSHEET_TRACER_RENDERER_H
#define AVT_SPREADSHEET_TRACER_RENDERER_H
#include <SpreadsheetAttributes.h>

class vtkDataArray;
class vtkDataSet;

// ****************************************************************************
// Class: avtSpreadsheetTraceRenderer
//
// Purpose:
//   Base class for the spreadsheet highlight renderer.
//
// Notes:      
//
// Programmer: Brad Whitlock
// Creation:   Wed Feb 21 09:17:47 PDT 2007
//
// Modifications:
//   
// ****************************************************************************

class avtSpreadsheetTraceRenderer
{
public:
    avtSpreadsheetTraceRenderer();
    virtual ~avtSpreadsheetTraceRenderer();

    virtual void Render(vtkDataSet *, vtkDataArray *, 
                        const SpreadsheetAttributes &, const double *) = 0;
};

#endif
