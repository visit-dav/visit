// ************************************************************************* //
//                                avtVectorPlot.h                            //
// ************************************************************************* //

#ifndef AVT_VECTOR_PLOT_H
#define AVT_VECTOR_PLOT_H

#include <avtLegend.h>
#include <avtPlot.h>

#include <VectorAttributes.h>

class     vtkVectorGlyph;

class     avtGhostZoneFilter;
class     avtVectorGlyphMapper;
class     avtVariableLegend;
class     avtVectorFilter;
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
    virtual bool                SetColorTable(const char *ctName);


  protected:
    VectorAttributes            atts;
    bool                        colorsInitialized;

    avtVectorGlyphMapper       *glyphMapper;
    avtVariableLegend          *varLegend;
    avtLegend_p                 varLegendRefPtr;
    avtVectorFilter            *vectorFilter;
    avtGhostZoneFilter         *ghostFilter;
    avtLookupTable             *avtLUT;

    vtkVectorGlyph             *glyph;

    virtual avtMapper          *GetMapper(void);
    virtual avtDataObject_p     ApplyOperators(avtDataObject_p);
    virtual avtDataObject_p     ApplyRenderingTransformation(avtDataObject_p);
    virtual void                CustomizeBehavior(void);
    virtual void                CustomizeMapper(avtDataObjectInformation &);

    virtual avtLegend_p         GetLegend(void) { return varLegendRefPtr; };
    void                        SetLegend(bool);
    void                        SetLegendRanges();
    void                        ComputeMagVarName(const std::string &);
    void                        SetMapperColors(void);
    std::string                 magVarName;

    virtual void                SetCellCountMultiplierForSRThreshold(const avtDataObject_p);
};


#endif


