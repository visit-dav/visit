// ************************************************************************* //
//                             avtPseudocolorPlot.h                          //
// ************************************************************************* //

#ifndef AVT_PSEUDOCOLOR_PLOT_H
#define AVT_PSEUDOCOLOR_PLOT_H


#include <avtLegend.h>
#include <avtPlot.h>
#include <PseudocolorAttributes.h>

class     avtLookupTable;
class     avtPointToGlyphFilter;
class     avtShiftCenteringFilter;
class     avtVariableLegend;
class     avtVariableMapper;
class     avtSmoothPolyDataFilter;


// ****************************************************************************
//  Class:  avtPseudocolorPlot
//
//  Purpose:
//      A concrete type of avtPlot, this is the standard pseudocolor plot.
//
//  Programmer: Hank Childs
//  Creation:   December 28, 2000
//
//  Modifications:
//
//    Jeremy Meredith, Fri Mar  2 13:10:02 PST 2001
//    Made SetAtts virtual and take a generic AttributeGroup.
//
//    Jeremy Meredith, Sun Mar  4 16:59:57 PST 2001
//    Added a static Create method.
//
//    Hank Childs, Tue Mar 27 14:47:03 PST 2001
//    Inherited from avtSurfaceDataPlot instead of avtPlot and added GetName.
//
//    Kathleen Bonnell, Tue Apr  3 08:56:47 PDT 2001 
//    Made PseudocolorAttributes a data member so other methods can have access
//    to the atts.  Added SetScaling, DefineLogLUT, and DefineSkewLUT methods. 
//
//    Kathleen Bonnell, Tue Apr 24 12:22:01 PDT 2001
//    Added avtShiftCenterFilter. 
//    
//    Jeremy Meredith, Tue Jun  5 20:45:02 PDT 2001
//    Allow storage of attributes as a class member.
//
//    Brad Whitlock, Thu Jun 14 16:49:22 PST 2001
//    Added SetColorTable method.
//
//    Kathleen Bonnell, Wed Aug 29 16:44:31 PDT 2001 
//    Added avtLookupTable and previousMode. 
//
//    Kathleen Bonnell, Thu Oct  4 16:28:16 PDT 2001 
//    Added SetLimitsMode.  Removed SetMin, SetMax, SetMinOff, SetMaxOff.
//
//    Kathleen Bonnell, Wed Mar 13 12:04:53 PST 2002 
//    Added private method SetLegendRanges.  
//
//    Kathleen Bonnell, Thu Mar 28 14:03:19 PST 2002 
//    Removed previousMode, no longer needed. 
//
//    Hank Childs, Sun Jun 23 12:19:23 PDT 2002
//    Add support for point meshes.
//
//    Kathleen Bonnell, Tue Oct 22 08:33:26 PDT 2002
//    Added ApplyRenderingTransformation. 
//    
//    Jeremy Meredith, Tue Dec 10 09:06:18 PST 2002
//    Added GetSmoothingLevel to allow smoothing inside avtPlot.
//
// ****************************************************************************

class avtPseudocolorPlot : public avtSurfaceDataPlot
{
  public:
                                avtPseudocolorPlot();
    virtual                    ~avtPseudocolorPlot();

    static avtPlot             *Create();

    virtual const char         *GetName(void) { return "PseudocolorPlot"; };

    virtual void                SetAtts(const AttributeGroup*);
    virtual bool                SetColorTable(const char *ctName);
    virtual void                ReleaseData(void);

    void                        SetLegend(bool);
    void                        SetLighting(bool);

    void                        SetLimitsMode(int);

    void                        SetOpacity(float);
    void                        SetScaling(int, double);

  protected:
    avtVariableMapper          *varMapper;
    avtVariableLegend          *varLegend;
    avtLegend_p                 varLegendRefPtr;
    PseudocolorAttributes       atts;
    avtShiftCenteringFilter    *filter;
    avtPointToGlyphFilter      *glyphPoints;
    avtSmoothPolyDataFilter    *smooth;
    bool                        colorsInitialized;
    avtLookupTable             *avtLUT;

    virtual avtMapper          *GetMapper(void);
    virtual avtDataObject_p     ApplyOperators(avtDataObject_p);
    virtual avtDataObject_p     ApplyRenderingTransformation(avtDataObject_p);
    virtual void                CustomizeBehavior(void);
    virtual int                 GetSmoothingLevel();

    virtual avtLegend_p         GetLegend(void) { return varLegendRefPtr; };

private:
    void                        SetLegendRanges(void);
};


#endif


