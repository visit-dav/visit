// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                            avtTruecolorPlot.h                             //
// ************************************************************************* //

#ifndef AVT_Truecolor_PLOT_H
#define AVT_Truecolor_PLOT_H

 
#include <avtLegend.h>
#include <avtPlot.h>
#include <avtVariableMapper.h>

#include <TruecolorAttributes.h>

class     avtTruecolorFilter;


// ****************************************************************************
//  Class:  avtTruecolorPlot
//
//  Purpose:
//      A concrete type of avtPlot, this is the Truecolor plot.
//
//  Programmer: Chris Wojtan
//  Creation:   Monday, June 15 2004
//
//  Modifications:
//
//    Hank Childs, Wed Nov 10 11:32:31 PST 2004
//    Add ReleaseData to address memory leaks.
//
//    Kathleen Bonnell, Mon Jul 25 15:27:06 PDT 2005 
//    Added SetLighting method.
//
// ****************************************************************************

class avtTruecolorPlot : public avtSurfaceDataPlot
{
  public:
                                avtTruecolorPlot();
    virtual                    ~avtTruecolorPlot();

    virtual const char         *GetName(void) { return "TruecolorPlot"; };

    static avtPlot             *Create();

    virtual void                SetAtts(const AttributeGroup*);
    virtual void                ReleaseData();
    
    void                        SetOpacity(float);
    void                        SetLighting(bool);

  protected:
    avtVariableMapper          *myMapper;
    avtTruecolorFilter         *TruecolorFilter;
    TruecolorAttributes         atts;

    virtual avtMapperBase      *GetMapper(void);
    virtual avtDataObject_p     ApplyOperators(avtDataObject_p);
    virtual avtDataObject_p     ApplyRenderingTransformation(avtDataObject_p);
    virtual void                CustomizeBehavior(void);
    virtual void                CustomizeMapper(avtDataObjectInformation &);

    virtual avtLegend_p         GetLegend(void) { return NULL; };
};


#endif
