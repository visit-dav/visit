// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                                 avtSpreadsheetPlot.h                      //
// ************************************************************************* //

#ifndef AVT_Spreadsheet_PLOT_H
#define AVT_Spreadsheet_PLOT_H


#include <avtLegend.h>
#include <avtPlot.h>

#include <SpreadsheetAttributes.h>
#include <avtSpreadsheetRenderer.h>

class     avtUserDefinedMapper;
class     avtSpreadsheetFilter;
class     avtSpreadsheetMapper;

// ****************************************************************************
//  Class:  avtSpreadsheetPlot
//
//  Purpose:
//      A concrete type of avtPlot, this is the Spreadsheet plot.
//
//  Programmer: Brad Whitlock
//  Creation:   Tue Feb 6 12:08:42 PDT 2007
//
//  Modifications:
//    Kathleen Bonnell, Wed May  9 16:58:50 PDT 2007
//    Set CanDo2DViewScaling to false.
//
//    Hank Childs, Fri Feb  1 13:16:26 PST 2008
//    Define UtilizeRenderingFilters.
//
//    Kathleen Bonnell, Tue Mar  3 13:37:13 PST 2009
//    Removed CanDo2DViewScaling (moved into Viewer PluginInfo)
//
// ****************************************************************************

class avtSpreadsheetPlot : public avtVolumeDataPlot
{
  public:
                                avtSpreadsheetPlot();
    virtual                    ~avtSpreadsheetPlot();

    virtual const char         *GetName(void) { return "Spreadsheet"; };

    static avtPlot             *Create();

    virtual void                SetAtts(const AttributeGroup*);
    virtual bool                SetColorTable(const char *ctName);
    virtual bool                SetForegroundColor(const double *fg);

  protected:
    SpreadsheetAttributes       atts;

    avtSpreadsheetFilter       *SpreadsheetFilter;
    avtUserDefinedMapper       *mapper;
    avtSpreadsheetRenderer_p    renderer;
    bool                        colorsInitialized;

    virtual avtMapperBase      *GetMapper(void);
    virtual avtDataObject_p     ApplyOperators(avtDataObject_p);
    virtual avtDataObject_p     ApplyRenderingTransformation(avtDataObject_p);
    virtual void                CustomizeBehavior(void);
    virtual void                CustomizeMapper(avtDataObjectInformation &);

    virtual avtLegend_p         GetLegend(void) { return NULL; };

    virtual bool                UtilizeRenderingFilters(void) { return false; };
};


#endif
