// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                                avtVectorPlot.h                            //
// ************************************************************************* //

#ifndef AVT_VECTOR_PLOT_H
#define AVT_VECTOR_PLOT_H

#include <avtLegend.h>
#include <avtPlot.h>

#include <VectorAttributes.h>

#include <string>

class     vtkVectorGlyph;

class     avtVectorFilter;
class     avtVectorGlyphMapper;
class     avtGhostZoneFilter;
class     avtResampleFilter;
class     avtVariableLegend;
class     avtLookupTable;

// ****************************************************************************
//  Class:  avtVectorPlot
//
//  Purpose:
//      A concrete type of avtPlot, this is the standard vector plot.
//
//  Programmer: Hank Childs
//  Creation:   March 21, 2001
//
//  Modifications:
//
//    Hank Childs, Tue Mar 27 14:47:03 PST 2001
//    Inherited from avtPointDataPlot instead of avtPlot and added GetName.
//
//    Brad Whitlock, Fri Jun 15 15:11:25 PST 2001
//    Added override of SetColorTable method.
//
//    Kathleen Bonnell, Thu Aug 30 10:47:07 PDT 2001 
//    Added avtLookupTable as member.
//
//    Kathleen Bonnell, Tue Oct 22 08:33:26 PDT 2002
//    Added ApplyRenderingTransformation. 
//
//    Brad Whitlock, Mon Dec 2 11:57:57 PDT 2002
//    I added a legend.
//
//    Mark C. Miller, Wed Aug 11 23:42:18 PDT 2004
//    Added GetCellCountMultiplierForSRThreshold()
//
//    Kathleen Bonnell, Mon Aug  9 14:33:26 PDT 2004 
//    Added magVarName, ComputeMagVarName, and SetMapperColors.
//
//    Mark C. Miller, Mon Aug 23 20:24:31 PDT 2004
//    Changed GetCellCountMultiplierForSRThreshold to Set...
//
//    Kathleen Bonnell, Wed Dec 22 17:01:09 PST 2004 
//    Added SetLimitsMode. 
//
//    Hank Childs, Wed Aug 13 11:40:52 PDT 2008
//    Turn on NeedZBufferToCompositeEvenIn2D, as vector glyphs can bleed
//    into other processor's portion of image space.
//
//    Hank Childs, Tue Aug 24 22:31:43 PDT 2010
//    Add resample operator for uniform glyph placement.
//
//    Kathleen Biagas, Wed Feb 29 13:10:11 MST 2012
//    Add GetExtraInfoForPick.
//
// ****************************************************************************

class avtVectorPlot : public avtPointDataPlot
{
  public:
                                avtVectorPlot();
    virtual                    ~avtVectorPlot();

    virtual const char         *GetName(void) { return "VectorPlot"; };
    virtual void                ReleaseData(void);

    static avtPlot             *Create();

    virtual void                SetAtts(const AttributeGroup*);
    virtual bool                NeedZBufferToCompositeEvenIn2D(void)
                                                          { return true; };

    virtual const MapNode      &GetExtraInfoForPick(void);

  protected:
    VectorAttributes            atts;
    bool                        colorsInitialized;

    avtVectorGlyphMapper       *vectorMapper;
    avtVariableLegend          *varLegend;
    avtLegend_p                 varLegendRefPtr;
    avtVectorFilter            *vectorFilter;
    avtGhostZoneFilter         *ghostFilter;
    avtResampleFilter          *resampleFilter;
    avtLookupTable             *avtLUT;

    vtkVectorGlyph             *vectorGlyph;

    virtual avtMapperBase      *GetMapper(void);
    virtual avtDataObject_p     ApplyOperators(avtDataObject_p);
    virtual avtDataObject_p     ApplyRenderingTransformation(avtDataObject_p);
    virtual void                CustomizeBehavior(void);
    virtual void                CustomizeMapper(avtDataObjectInformation &);

    bool                        SetColorTable(const char *);
    void                        SetLimitsMode(int);
    void                        SetMapperColors(void);

    virtual avtLegend_p         GetLegend(void) { return varLegendRefPtr; };
    void                        SetLegend(bool);
    void                        SetLegendRanges();

    virtual void                SetCellCountMultiplierForSRThreshold(const avtDataObject_p); 

    void                        ComputeMagVarName(const std::string &);
    std::string                 magVarName;
};


#endif
