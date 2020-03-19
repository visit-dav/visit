// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                               avtScatterPlot.h                            //
// ************************************************************************* //

#ifndef AVT_SCATTER_PLOT_H
#define AVT_SCATTER_PLOT_H

#include <ScatterAttributes.h>

#include <avtPlot.h>

class avtLookupTable;
class avtVariableLegend;
class avtVariablePointGlyphMapper;
class avtScatterFilter;

// ****************************************************************************
//  Method: avtScatterPlot
//
//  Purpose:
//      A concrete type of avtPlot for scatter plots of scalar variables.
//
//  Programmer: Brad Whitlock
//  Creation:   Tue Nov 2 21:52:32 PST 2004
//
//  Modifications:
//   Brad Whitlock, Thu Jul 21 15:29:40 PST 2005
//   Added SetPointGlyphMode.
//
//   Kathleen Biagas, Wed Feb 29 13:10:11 MST 2012
//   Add GetExtraInfoForPick.
//
//   Kathleen Biagas, Thu Oct 31 12:39:39 MST 2019
//   Add PlotHasBeenGlyphed.
//
// ****************************************************************************

class avtScatterPlot : public avtPlot
{
public:
                    avtScatterPlot();
    virtual        ~avtScatterPlot();

    static avtPlot *Create();

    virtual const char *GetName(void)  { return "ScatterPlot"; }
    virtual void    ReleaseData(void);

    virtual void    SetAtts(const AttributeGroup*);
    virtual bool    SetColorTable(const char *ctName);

    virtual bool    Equivalent(const AttributeGroup*);

    void            SetLegend(bool);
    void            SetVarName(const char *);
    virtual bool    SetForegroundColor(const double *);

    virtual int     TargetTopologicalDimension(void);

    virtual const MapNode &GetExtraInfoForPick(void);

    virtual bool    PlotHasBeenGlyphed();

protected:
    avtVariablePointGlyphMapper     *glyphMapper;
    avtVariableLegend               *varLegend;
    avtLegend_p                      varLegendRefPtr;
    bool                             colorsInitialized;
    avtLookupTable                  *avtLUT;
    avtScatterFilter                *filter;
    double                           fgColor[3];

    ScatterAttributes                atts;

    virtual avtMapperBase   *GetMapper(void);
    virtual avtDataObject_p  ApplyOperators(avtDataObject_p);
    virtual avtDataObject_p  ApplyRenderingTransformation(avtDataObject_p);
    virtual void             CustomizeBehavior(void);
    virtual avtLegend_p      GetLegend(void) { return varLegendRefPtr; }
    virtual avtContract_p
                             EnhanceSpecification(avtContract_p);


    void SetScaling(int mode, double skew);
    void SetLimitsMode();
    void GetColorInformation(std::string &colorString,
                             int &mode, double &skew, bool &minFlag,
                             double &minVal, bool &maxFlag, double &maxVal);
    void SetPointGlyphSize();
};


#endif


