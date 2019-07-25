// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#ifndef AVT_SPREADSHEET_RENDERER_H
#define AVT_SPREADSHEET_RENDERER_H

#include <avtCustomRenderer.h>
#include <SpreadsheetAttributes.h>

class avtSpreadsheetTraceRenderer;

// ****************************************************************************
//  Class: avtSpreadsheetRenderer
//
//  Purpose:
//      An implementation of an avtCustomRenderer for a spreadsheet plot.
//
//  Programmer: Brad Whitlock
//  Creation:   Tue Feb 6 16:01:07 PST 2007
//
//  Modifications:
//
// ****************************************************************************

class avtSpreadsheetRenderer : public avtCustomRenderer
{
public:
                            avtSpreadsheetRenderer();
    virtual                ~avtSpreadsheetRenderer();
    static avtSpreadsheetRenderer *New(void);

    virtual bool            OperatesOnScalars(void) { return true; };
    virtual void            Render(vtkDataSet *);
    virtual void            ReleaseGraphicsResources();
    virtual void            SetAlternateDisplay(void *);

    void                    SetAtts(const AttributeGroup *);
    bool                    SetColorTable(const char *);
    bool                    SetForegroundColor(const double *);
private:
    void RenderTracePlane(vtkDataSet *);

    void                        *plotDisplay;
    SpreadsheetAttributes        atts;
    double                       fgColor[3];
    avtSpreadsheetTraceRenderer *rendererImplementation;
};


typedef ref_ptr<avtSpreadsheetRenderer> avtSpreadsheetRenderer_p;


#endif
