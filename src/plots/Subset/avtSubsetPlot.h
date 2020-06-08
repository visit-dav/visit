// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ****************************************************************************
//  avtSubsetPlot.h
// ****************************************************************************

#ifndef AVT_SUBSET_PLOT_H
#define AVT_SUBSET_PLOT_H


#include <avtPlot.h>
#include <SubsetAttributes.h>

class     avtLevelsLegend;
class     avtLookupTable;

class     avtFeatureEdgesFilter;
class     avtGhostZoneAndFacelistFilter;
class     avtGhostZoneFilter;
class     avtFacelistFilter;
class     avtSubsetFilter;
class     avtSubsetMapper;
class     avtSmoothPolyDataFilter;
class     avtSubsetBlockMergeFilter;
class     avtVertexExtractor;

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
//    Brad Whitlock, Thu Jul 21 15:38:31 PST 2005
//    Added SetPointGlyphSize.
//
//    Hank Childs, Fri Aug  3 13:46:26 PDT 2007
//    Added another ghost zone filter (gz2) for removing edges on the coarse
//    fine boundary of AMR grids.
//
//    Hank Childs, Wed Aug 13 11:42:07 PDT 2008
//    Add NeedZBufferToCompositeEvenIn2D because thick lines can bleed into
//    other processor's portion of image space.
//
//    Kevin Griffin, Mon Nov 3 12:47:05 PDT 2014
//    Added the avtSubsetBlockMergeFilter to merge the datasets of a block
//    into one dataset so the avtFeaturedEdgesFilter can properly outline
//    the block when the wireframe option is selected from the Subset plot
//    attributes dialog.
//
//    Kathleen Biagas, Tue Aug 23 11:20:32 PDT 2016
//    Added LevelsMapper as points and surfaces no longer handled by the
//    same mapper.
//
//    Kathleen Biagas, Fri Jun  5 08:18:16 PDT 2020
//    Add avtSurfaceMapper in place of levels and point glyph mapper.  Add
//    avtVertexExtractor.  Changes allows glyphing of points when topological
//    dimension not set to 0 and/or dataset contains mixed topology (including
//    vertex cells).
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
    virtual bool              NeedZBufferToCompositeEvenIn2D(void);

  protected:
    avtFeatureEdgesFilter           *wf;
    avtGhostZoneAndFacelistFilter   *gzfl;
    avtGhostZoneFilter              *gz;
    avtGhostZoneFilter              *gz2;
    avtFacelistFilter               *fl;
    avtSubsetFilter                 *sub;
    avtSmoothPolyDataFilter         *smooth;
    avtSubsetBlockMergeFilter       *sbmf;
    avtVertexExtractor              *vertexExtractor;

    SubsetAttributes          atts;
    avtSubsetMapper          *subsetMapper;
    avtLevelsLegend          *levelsLegend;
    avtLegend_p               levLegendRefPtr;
    avtLookupTable           *avtLUT;

    void                      SetColors(void);
    void                      SortLabels(void);
    void                      SetPointGlyphSize();
    virtual avtMapperBase    *GetMapper(void);
    virtual avtDataObject_p   ApplyOperators(avtDataObject_p);
    virtual avtDataObject_p   ApplyRenderingTransformation(avtDataObject_p);
    virtual void              CustomizeBehavior(void);

    virtual avtLegend_p       GetLegend(void) { return levLegendRefPtr; };
};


#endif


