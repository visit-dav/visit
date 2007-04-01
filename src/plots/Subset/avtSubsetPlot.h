// ************************************************************************* //
//                             avtSubsetPlot.h                             //
// ************************************************************************* //

#ifndef AVT_SUBSET_PLOT_H
#define AVT_SUBSET_PLOT_H


#include <avtPlot.h>
#include <SubsetAttributes.h>

class     avtLevelsLegend;
class     avtLevelsPointGlyphMapper;
class     avtLookupTable;

class     avtFeatureEdgesFilter;
class     avtGhostZoneAndFacelistFilter;
class     avtGhostZoneFilter;
class     avtFacelistFilter;
class     avtSubsetFilter;
class     avtSmoothPolyDataFilter;


// ****************************************************************************
//  Method: avtSubsetPlot
//
//  Purpose:
//      A concrete type of avtPlot for subsets.
//
//  Programmer: Kathleen Bonnell 
//  Creation:   October 16, 2001 
//
//  Modifications:
//    Eric Brugger, Fri Dec 14 13:03:02 PST 2001
//    I modified the class so that it now inherits from avtVolumeDataPlot,
//    since the plot will now take care of reducing the topology from 3 to 2.
//
//    Jeremy Meredith, Tue Mar 12 17:23:11 PST 2002
//    Added a line style.
//
//    Kathleen Bonnell, Wed Apr 10 09:45:43 PDT 2002 
//    Added SetColors. 
//
//    Kathleen Bonnell, Tue Oct 22 08:33:26 PDT 2002
//    Added ApplyRenderingTransformation. 
//
//    Jeremy Meredith, Tue Dec 10 10:04:18 PST 2002
//    Added smooth poly data filter.
//
//    Brad Whitlock, Tue Nov 26 11:03:31 PDT 2002
//    Added the SetColorTable method.
//
//    Kathleen Bonnell, Thu Dec 19 12:27:09 PST 2002 
//    Added the SortLabels method.
//
//    Mark C. Miller, Wed Mar 24 19:23:21 PST 2004
//    Added AttributesDependOnDatabaseMetaData
//
//    Kathleen Bonnell, Fri Nov 12 11:47:49 PST 2004 
//    Changed mapper type to avtLevelsPointGlyphMapper. 
//
// ****************************************************************************

class
avtSubsetPlot : public avtVolumeDataPlot
{
  public:
                              avtSubsetPlot();
    virtual                  ~avtSubsetPlot();

    static avtPlot           *Create();

    virtual const char       *GetName(void) { return "SubsetPlot"; };


    virtual void              SetAtts(const AttributeGroup*);
    virtual void              ReleaseData(void);
    virtual bool              SetColorTable(const char *ctName);

    virtual bool              AttributesDependOnDatabaseMetaData(void)
                                  { return true; };

    void                      SetLegend(bool);
    void                      SetLineWidth(int);
    void                      SetLineStyle(int);

  protected:
    avtFeatureEdgesFilter           *wf;
    avtGhostZoneAndFacelistFilter   *gzfl;
    avtGhostZoneFilter              *gz;
    avtFacelistFilter               *fl;
    avtSubsetFilter                 *sub;
    avtSmoothPolyDataFilter         *smooth;

    SubsetAttributes          atts;
    avtLevelsPointGlyphMapper *levelsMapper;
    avtLevelsLegend          *levelsLegend;
    avtLegend_p               levLegendRefPtr;
    avtLookupTable           *avtLUT;

    void                      SetColors(void); 
    void                      SortLabels(void); 
    virtual avtMapper        *GetMapper(void);
    virtual avtDataObject_p   ApplyOperators(avtDataObject_p);
    virtual avtDataObject_p   ApplyRenderingTransformation(avtDataObject_p);
    virtual void              CustomizeBehavior(void);

    virtual avtLegend_p       GetLegend(void) { return levLegendRefPtr; };
};


#endif


