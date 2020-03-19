// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                                   avtPlot.h                               //
// ************************************************************************* //

#ifndef AVT_PLOT_H
#define AVT_PLOT_H

#include <plotter_exports.h>

#include <ref_ptr.h>

#include <avtActor.h>
#include <avtDataObjectReader.h>
#include <avtDataObjectWriter.h>
#include <avtDecorationsMapper.h>
#include <avtImage.h>
#include <avtLegend.h>
#include <avtMapperBase.h>
#include <avtBehavior.h>
#include <avtDatasetToDatasetFilter.h>

#include <enumtypes.h>
#include <string>
#include <vector>

class     avtCondenseDatasetFilter;
class     avtDatasetToDatasetFilter;
class     avtCompactTreeFilter;
class     avtSmoothPolyDataFilter;
class     avtVertexNormalsFilter;
class     avtMeshLogFilter;
class     AttributeGroup;
class     AttributeSubject;
class     PlotInfoAttributes;
class     WindowAttributes;
class     RenderingAttributes;
class     MapNode;

// ****************************************************************************
//  Class: avtPlot
//
//  Purpose:
//    This is the base type for all plots in VisIt.  A plot is something that
//    defines a color map, adds behavior, and can potentially add operators.
//    The derived types need to do that.  This base type sets up the
//    infrastructure for the rest of the plots.  It must be instantiated for
//    each plot on both the viewer and the engine and allows for either
//    geometry or images to be sent across the network.
//
//  Programmer: Hank Childs
//  Creation:   December 27, 2000
//
//  Modifications:
//    Kathleen Bonnell, Fri Jan 12 09:38:17 PST 2001
//    Added member avtDrawer.
//
//    Jeremy Meredith, Thu Mar  1 13:52:16 PST 2001
//    Added a relevantPointsFilter and a facelistFilter directly to this class.
//
//    Jeremy Meredith, Fri Mar  2 13:10:02 PST 2001
//    Made a pure virtual SetAtts method.
//
//    Hank Childs, Tue Mar 27 14:32:54 PST 2001
//    Added avtSurfaceDataPlot, avtVolumeDataPlot, and avtImageDataPlot.
//    Also added pure virtual function GetName.
//
//    Jeremy Meredith, Tue Jun  5 20:38:48 PDT 2001
//    Added a flag to allow determination of whether a plot needs
//    recalculation.  Added NeedsRecalculation() to get the value.
//
//    Brad Whitlock, Thu Jun 14 16:42:27 PST 2001
//    Added the SetColorTable method that gets overridden in certain subclasses.
//
//    Kathleen Bonnell, Wed Jun 20 13:35:27 PDT 2001
//    Added avtGhostZoneFilter.
//
//    Kathleen Bonnell, Fri Jul 20 07:26:55 PDT 2001
//    Added ghostZoneAndFacelistFilter, removed facelistFilter, ghostZoneFilter.
//
//    Eric Brugger, Fri Aug 31 11:13:48 PDT 2001
//    I added a definition for avtPlot_p
//
//    Eric Brugger, Fri Sep  7 13:36:15 PDT 2001
//    Removed the SIL restriction argument from NeedsRecalculation.  Added
//    the method GetCurrentSILRestriction.
//
//    Kathleen Bonnell, Wed Sep 19 12:55:57 PDT 200
//    Added method CompactTree, and filter avtCompactTreeFilter.
//
//    Kathleen Bonnell, Wed Sep 26 15:36:14 PDT 2001
//    Added virtual methods SetBackgroundColor, SetForegroundColor.
//
//    Kathleen Bonnell, Thu Oct  4 16:28:16 PDT 2001
//    Added method SetCurrentExtents, member currentExtentFilter.
//
//    Jeremy Meredith, Thu Nov  8 11:07:22 PST 2001
//    Added Equivalent and ReleaseData.
//    Added WindowAttributes to the engine's Execute method.
//
//    Hank Childs, Tue Nov 20 14:48:56 PST 2001
//    Added plot index.
//
//    Kathleen Bonnell, Mon Nov 26 9:16:32 PST 2001
//    Added hook to retrieve flag specifiying whether points were tranformed.
//
//    Hank Childs, Mon Dec 31 11:36:28 PST 2001
//    Added vertex normals.
//
//    Kathleen Bonnell, Fri Jul 12 18:42:11 PDT 2002
//    Added hook to retrieve a plot's decorations mapper.
//
//    Kathleen Bonnell, Mon Sep 30 14:38:33 PDT 2002
//    Added intermediateDataObject, which is a copy of the data in the pipeline
//    before "ReduceGeometry" occurs.   Used by Queries.
//
//    Kathleen Bonnell, Tue Oct 22 08:41:29 PDT 2002
//    Added method ApplyRenderingTransformation, in order to split
//    ApplyOperators into two methods.  If a plot should be query-able, the
//    output from ApplyOperators will yield a queryable object.
//
//    Jeremy Meredith, Tue Dec 10 09:05:11 PST 2002
//    Added virtual GetSmoothingLevel function.
//
//    Mark C. Miller, Tue Feb 11 08:33:07 PST 2003
//    Added a CombinedExecute method to support external rendering.
//    Added protected member functions for both Execute methods that
//    actually implement the Execute methods. The public methods are unchanged
//    externally but have become wrappers that call the protected equivalents.
//
//    Kathleen Bonnell, Tue Mar 25 11:18:43 PST 2003
//    Renamed GetTransformedPoints to RequiresReExecuteForQuery.
//
//    Jeremy Meredith, Wed Aug 13 16:33:21 PDT 2003
//    Made the vertex normals filter be a pointer to the subclass so I could
//    access methods not in avtDatasetToDatasetFilter.
//
//    Kathleen Bonnell, Wed Aug 27 15:45:45 PDT 2003
//    Added SetOpaqueMeshIsAppropriate.
//
//    Eric Brugger, Fri Mar 19 15:46:52 PST 2004
//    Added Set/GetDataExtents.
//
//    Mark C. Miller, Wed Mar 24 19:23:21 PST 2004
//    Added AttributesDependOnDatabaseMetaData
//
//    Kathleen Bonnell, Tue Jun  1 15:08:30 PDT 2004
//    Added bool args to RequiresReExecuteForQuery.
//
//    Brad Whitlock, Tue Jul 20 16:10:25 PST 2004
//    Added variable units.
//
//    Mark C. Miller, Wed Aug 11 23:42:18 PDT 2004
//    Added GetCellCountMultiplierForSRThreshold
//
//    Mark C. Miller, Mon Aug 23 20:27:17 PDT 2004
//    Made GetCellCountMultiplierForSRThreshold non-virtual
//    Added SetCellCountMultiplierForSRThreshold
//
//    Kathleen Bonnell, Tue Aug 24 16:12:03 PDT 2004
//    Added avtMeshType arg to SetOpaqueMeshIsAppropriate.
//
//    Kathleen Bonnell, Tue Nov  2 10:18:16 PST 2004
//    Added meshType as a member of this class, removed MeshType arg from
//    SetOpaqueMeshIsAppropriate.
//
//    Kathleen Bonnell, Wed Nov  3 16:51:24 PST 2004
//    Removed meshType, added topologicalDim and SpatialDim.
//
//    Hank Childs, Wed Nov 24 16:39:52 PST 2004
//    Added virtual functions PlotIsImageBased and ImageExecute.
//    This is for plots such as the volume plot which cannot play well with
//    volume rendering.
//
//    Brad Whitlock, Tue Dec 14 15:29:22 PST 2004
//    I made SetVarName be virtual.
//
//    Hank Childs, Sun Dec  4 17:52:16 PST 2005
//    Added GetNumberOfStagesForImageBasedPlots.
//
//    Kathleen Bonnell Tue Jun 20 16:02:38 PDT 2006
//    Added GetPlotInfoAtts.
//
//    Kathleen Bonnell, Thu Mar 22 15:45:21 PDT 2007
//    To facilitate log-views, added avtMeshLogFilter, SetScaleMode,
//    xScaleMode, yScaleMode, havePerformedLogX, havePerformedLogY.
//
//    Kathleen Bonnell,  Tue Apr  3 16:06:54 PDT 2007
//    Added bool return to SetScaleMode. Added virtual method
//    CanDoCurveViewScaling.
//
//    Gunther H. Weber, Thu Apr 12 10:48:17 PDT 2007
//    Added GetFilterForTopOfPipeline()
//
//    Kathleen Bonnell,  Wed May  9 16:58:50 PDT 2007
//    Added WindowMode arg to SetScaleMode. Added virtual method
//    CanDo2DViewScaling.
//
//    Kathleen Bonnell, Tue Sep 25 07:57:01 PDT 2007
//    Removed unused method GetScaleMode, added separate scale modes for 2D
//    and curve.  Added ScaleModeRequiresUpdate.
//
//    Hank Childs, Fri Feb  1 13:14:21 PST 2008
//    Add new virtual method "UtilizeRenderingFilters".
//
//    Tom Fogal, Fri Jun 13 11:19:01 EDT 2008
//    I made GetNumberOfStagesForImageBasedPlot a const method, to allow
//    calling it from a const method in NetworkManager.
//
//    Hank Childs, Wed Aug 13 10:36:40 PDT 2008
//    Add new virtual method "NeedZBufferToCompositeEvenIn2D".
//
//    Brad Whitlock, Wed Jan  7 14:24:40 PST 2009
//    I renamed a method that returns the plot info atts.
//
//    Kathleen Bonnell, Mon Mar  2 16:29:09 PST 2009
//    Removed CanDoCurveViewScaling, CanDo2DViewScaling (moved into
//    ViewerPlotPluginInfo)
//
//    Hank Childs, Mon Apr  6 14:05:10 PDT 2009
//    Add support for registering named selections.
//
//    Brad Whitlock, Wed Sep  9 16:17:55 PDT 2009
//    I made ReduceGeometry, CompactTree, SetCurrentExtents virtual.
//
//    Hank Childs, Sun Feb 21 10:03:53 CST 2010
//    Add method ManagesOwnTransparency.
//
//    Hank Childs, Thu Aug 26 13:47:30 PDT 2010
//    Rename SetCurrentExtents to SetActualExtents.
//
//    Kathleen Biagas, Wed Feb 29 07:36:59 PST 2012
//    Added GetExtraInfoForPick.
//
//    Brad Whitlock, Thu Sep 19 11:55:28 PDT 2013
//    Added AugmentAtts and ModifyContract.
//    Work partially supported by DOE Grant SC0007548.
//
//    Kathleen Biagas, Mon Apr 21 11:30:37 PDT 2014
//    Removed no longer used CombinedExecute and protected Execute methods.
//
//    Kathleen Biagas, Thu Apr 13 10:42:01 PDT 2017
//    Make GetMapper return an avtMapperBase.
//
//    Kathleen Biagas, Thu Oct 31 12:23:09 PDT 2019
//    Added PlotHasBeenGlyphed, default return false so derived types don't
//    have to define.
//
// ****************************************************************************

class PLOTTER_API avtPlot
{
  public:
    static int                 instanceCount;

                               avtPlot();
    virtual                   ~avtPlot();

    virtual const char        *GetName(void) = 0;

    avtDataObjectWriter_p      Execute(avtDataObject_p,
                                       avtContract_p,
                                       const WindowAttributes*);
    avtActor_p                 Execute(avtDataObjectReader_p);
    avtActor_p                 Execute(avtDataObjectReader_p, avtDataObject_p dob);

    virtual bool               PlotIsImageBased(void) { return false; }
    virtual int                GetNumberOfStagesForImageBasedPlot(
                                       const WindowAttributes &) const
                                       { return 0; }
    virtual avtImage_p         ImageExecute(avtImage_p img,
                                            const WindowAttributes &)
                                    { return img; }

    virtual bool               Equivalent(const AttributeGroup *)
                                            { return false; }
    virtual void               ReleaseData(void);

    virtual void               SetAtts(const AttributeGroup*) = 0;
    virtual bool               AugmentAtts(AttributeGroup*);

    void                       SetDataExtents(const std::vector<double> &);
    virtual void               GetDataExtents(std::vector<double> &);

    virtual void               SetPlotTitle(const char *title);
    virtual void               SetVarName(const char *name);
    void                       SetVarUnits(const char *units);
    void                       SetMeshType(const avtMeshType);

    bool                       NeedsRecalculation(void);

    virtual bool               AttributesDependOnDatabaseMetaData(void)
                                   { return false; }

    virtual bool               SetColorTable(const char *ctName);
    void                       SetCurrentSILRestriction(avtSILRestriction_p);
    avtSILRestriction_p        GetCurrentSILRestriction(void);

    virtual bool               SetBackgroundColor(const double *);
    virtual bool               SetForegroundColor(const double *);

    void                       SetIndex(int ind) { index = ind; }

    bool                       RequiresReExecuteForQuery(const bool, const bool);
    avtDataObject_p            GetIntermediateDataObject(void)
                                  { return intermediateDataObject; }
    virtual avtMapperBase     *GetMapper(void) = 0;
    virtual bool               CanCacheWriterExternally(void) { return true; }
    virtual bool               ManagesOwnTransparency(void) { return false; }
    virtual const AttributeSubject
                              *SetOpaqueMeshIsAppropriate(bool)
                                   { return NULL; }
    float                      GetCellCountMultiplierForSRThreshold() const;
    const PlotInfoAttributes  &GetPlotInformation() const;

    bool                       SetScaleMode(ScaleMode, ScaleMode, WINDOW_MODE);
    bool                       ScaleModeRequiresUpdate(WINDOW_MODE, ScaleMode,
                                                       ScaleMode rs);
    virtual bool               NeedZBufferToCompositeEvenIn2D(void)
                                                          { return false; }
    virtual void               RegisterNamedSelection(const std::string &) {;}

    virtual avtFilter         *GetFilterForTopOfPipeline() { return 0; }

    virtual bool               CompatibleWithCumulativeQuery() const { return true; }

    virtual const MapNode&     GetExtraInfoForPick(void);
    virtual avtContract_p      ModifyContract(avtContract_p c0) { return EnhanceSpecification(c0); }

    virtual bool               PlotHasBeenGlyphed() { return false; }

  protected:
    bool                       needsRecalculation;
    int                        index;
    int                        instanceIndex;
    int                        spatialDim;
    int                        topologicalDim;
    avtActor_p                 actor;
    avtBehavior_p              behavior;
    avtCondenseDatasetFilter  *condenseDatasetFilter;
    avtDatasetToDatasetFilter *ghostZoneAndFacelistFilter;
    avtCompactTreeFilter      *compactTreeFilter;
    avtDatasetToDatasetFilter *actualExtentsFilter;
    avtMeshLogFilter          *logMeshFilter;
    avtVertexNormalsFilter    *vertexNormalsFilter;
    avtSmoothPolyDataFilter   *smooth;
    avtSILRestriction_p        silr;
    char                      *varname;
    char                      *varunits;
    std::vector<double>        dataExtents;
    float                      cellCountMultiplierForSRThreshold;

    virtual avtDataObject_p    ApplyOperators(avtDataObject_p) = 0;
    virtual avtDataObject_p    ApplyRenderingTransformation(avtDataObject_p)=0;
    virtual void               CustomizeBehavior(void) = 0;
    virtual void               CustomizeMapper(avtDataObjectInformation &);
    virtual avtLegend_p        GetLegend(void) = 0;

    virtual int                GetSmoothingLevel() { return 0; }
    virtual bool               UtilizeRenderingFilters(void) { return true; }

    virtual int                TargetTopologicalDimension(void) = 0;

    virtual avtDataObject_p    ReduceGeometry(avtDataObject_p);
    virtual avtDataObject_p    CompactTree(avtDataObject_p, const RenderingAttributes &);
    virtual avtDataObject_p    SetActualExtents(avtDataObject_p);
    avtDataObject_p            SetScaleMode(avtDataObject_p,
                                            ScaleMode, ScaleMode,
                                            bool &, bool &);

    avtDataObject_p            intermediateDataObject;

    virtual avtContract_p
                               EnhanceSpecification(avtContract_p);
    virtual avtDecorationsMapper
                               *GetDecorationsMapper(void);

    virtual void               SetCellCountMultiplierForSRThreshold(
                                   const avtDataObject_p dob);

    ScaleMode xScaleMode2D;
    ScaleMode yScaleMode2D;
    bool havePerformedLogX2D;
    bool havePerformedLogY2D;
    ScaleMode xScaleModeCurve;
    ScaleMode yScaleModeCurve;
    bool havePerformedLogXCurve;
    bool havePerformedLogYCurve;

    MapNode extraPickInfo;
};

typedef ref_ptr<avtPlot> avtPlot_p;

class PLOTTER_API avtImageDataPlot : public avtPlot
{
  public:
    virtual int          TargetTopologicalDimension(void) { return -1; }
};


class PLOTTER_API avtPointDataPlot : public avtPlot
{
  public:
    virtual int          TargetTopologicalDimension(void) { return 0; }
};


class PLOTTER_API avtLineDataPlot : public avtPlot
{
  public:
    virtual int          TargetTopologicalDimension(void) { return 1; }
};


class PLOTTER_API avtSurfaceDataPlot : public avtPlot
{
  public:
    virtual int          TargetTopologicalDimension(void) { return 2; }
};


class PLOTTER_API avtVolumeDataPlot : public avtPlot
{
  public:
    virtual int          TargetTopologicalDimension(void) { return 3; }
};


#endif


