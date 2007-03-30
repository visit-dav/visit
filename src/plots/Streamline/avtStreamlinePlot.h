// ************************************************************************* //
//                                 avtStreamlinePlot.h                       //
// ************************************************************************* //

#ifndef AVT_Streamline_PLOT_H
#define AVT_Streamline_PLOT_H

#include <avtLegend.h>
#include <avtPlot.h>

#include <StreamlineAttributes.h>

class     avtLookupTable;
class     avtShiftCenteringFilter;
class     avtStreamlineFilter;
class     avtVariableLegend;
class     avtVariableMapper;

// ****************************************************************************
//  Class:  avtStreamlinePlot
//
//  Purpose:
//      A concrete type of avtPlot, this is the Streamline plot.
//
//  Programmer: Brad Whitlock
//  Creation:   Mon Oct 21 12:48:28 PDT 2002
//
//  Modifications:
//
// ****************************************************************************

class avtStreamlinePlot : public avtLineDataPlot
{
  public:
                                avtStreamlinePlot();
    virtual                    ~avtStreamlinePlot();

    virtual const char         *GetName(void) { return "StreamlinePlot"; };

    static avtPlot             *Create();

    virtual void                SetAtts(const AttributeGroup*);
    virtual bool                SetColorTable(const char *ctName);

    virtual void                ReleaseData(void);

  protected:
    StreamlineAttributes        atts;
    bool                        colorsInitialized;

    avtVariableMapper          *varMapper;
    avtVariableLegend          *varLegend;
    avtLegend_p                 varLegendRefPtr;
    avtStreamlineFilter        *streamlineFilter;
    avtShiftCenteringFilter    *shiftCenteringFilter;
    avtLookupTable             *avtLUT;

    virtual avtMapper          *GetMapper(void);
    virtual avtDataObject_p     ApplyOperators(avtDataObject_p);
    virtual avtDataObject_p     ApplyRenderingTransformation(avtDataObject_p);
    virtual void                CustomizeBehavior(void);

    virtual avtLegend_p         GetLegend(void) { return varLegendRefPtr; };

    void                        SetLegendRanges();
    void                        SetLegend(bool);
    void                        SetLighting(bool);
    void                        SetLineWidth(int);
};


#endif
