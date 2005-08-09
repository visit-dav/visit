// ************************************************************************* //
//                                 avtLabelPlot.h                            //
// ************************************************************************* //

#ifndef AVT_Label_PLOT_H
#define AVT_Label_PLOT_H
#include <LabelAttributes.h>

#include <avtLegend.h>
#include <avtPlot.h>
#include <avtLabelRenderer.h>

class avtGhostZoneAndFacelistFilter;
class avtCondenseDatasetFilter;
class avtLabelFilter;
class avtLabelsMapper;
class avtLabelSubsetsFilter;
class avtUserDefinedMapper;
class avtVariableLegend;
class avtVertexNormalsFilter;

// ****************************************************************************
//  Class:  avtLabelPlot
//
//  Purpose:
//      A concrete type of avtPlot, this is the Label plot.
//
//  Programmer: Brad Whitlock
//  Creation:   Wed Jan 7 14:58:26 PST 2004
//
//  Modifications:
//    Brad Whitlock, Wed Dec 15 10:48:35 PDT 2004
//    Removed SetVarName.
//
//    Brad Whitlock, Wed Aug 3 18:04:15 PST 2005
//    I made it use a avtLabelsMapper so we can label subsets.
//
// ****************************************************************************

class avtLabelPlot : public avtSurfaceDataPlot
{
  public:
                                avtLabelPlot();
    virtual                    ~avtLabelPlot();

    static avtPlot             *Create();

    virtual const char         *GetName(void) { return "LabelPlot"; };
    virtual void                SetAtts(const AttributeGroup*);
    virtual void                ReleaseData(void);

    virtual bool                SetForegroundColor(const double *);
    void                        SetLegend(bool);

    virtual avtPipelineSpecification_p EnhanceSpecification(avtPipelineSpecification_p spec);

  protected:
    LabelAttributes                atts;
    avtGhostZoneAndFacelistFilter *ghostAndFaceFilter;
    avtCondenseDatasetFilter      *condenseFilter;
    avtVertexNormalsFilter        *normalFilter;
    avtLabelSubsetsFilter         *labelSubsetsFilter;
    avtLabelFilter                *labelFilter;
    avtLabelRenderer_p             renderer;
    avtLabelsMapper               *labelMapper; 
    avtVariableLegend             *varLegend;
    avtLegend_p                    varLegendRefPtr;

    virtual avtMapper          *GetMapper(void);
    virtual avtDataObject_p     ApplyOperators(avtDataObject_p);
    virtual avtDataObject_p     ApplyRenderingTransformation(avtDataObject_p);
    virtual void                CustomizeBehavior(void);
    virtual void                CustomizeMapper(avtDataObjectInformation &);

    virtual avtLegend_p         GetLegend(void) { return varLegendRefPtr; };
};


#endif
