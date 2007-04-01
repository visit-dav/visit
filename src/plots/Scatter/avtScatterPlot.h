// ************************************************************************* //
//                               avtScatterPlot.h                            //
// ************************************************************************* //

#ifndef AVT_SCATTER_PLOT_H
#define AVT_SCATTER_PLOT_H

#include <ScatterAttributes.h>

#include <avtPlot.h>
#include <avtSurfaceAndWireframeRenderer.h>

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
//   
// ****************************************************************************

class avtScatterPlot : public avtPlot
{
public:
                    avtScatterPlot();
    virtual        ~avtScatterPlot();

    static avtPlot *Create();

    virtual const char *GetName(void)  { return "ScatterPlot"; };
    virtual void    ReleaseData(void);

    virtual void    SetAtts(const AttributeGroup*);
    virtual bool    SetColorTable(const char *ctName);

    virtual bool    Equivalent(const AttributeGroup*);

    void            SetLegend(bool);
    void            SetVarName(const char *);
    virtual bool    SetForegroundColor(const double *);

    virtual int     TargetTopologicalDimension(void);

protected:
    avtVariablePointGlyphMapper     *glyphMapper;
    avtVariableLegend               *varLegend;
    avtLegend_p                      varLegendRefPtr;
    bool                             colorsInitialized;
    avtLookupTable                  *avtLUT;
    avtScatterFilter                *filter;
    double                           fgColor[3];

    ScatterAttributes                atts;

    virtual avtMapper       *GetMapper(void);
    virtual avtDataObject_p  ApplyOperators(avtDataObject_p);
    virtual avtDataObject_p  ApplyRenderingTransformation(avtDataObject_p);
    virtual void             CustomizeBehavior(void);
    virtual avtLegend_p      GetLegend(void) { return varLegendRefPtr; };
    virtual avtPipelineSpecification_p     
                             EnhanceSpecification(avtPipelineSpecification_p);


    void SetScaling(int mode, float skew);
    void SetLimitsMode();
    void GetColorInformation(std::string &colorString,
                             int &mode, float &skew, bool &minFlag,
                             float &minVal, bool &maxFlag, float &maxVal);
};


#endif


