// ************************************************************************* //
//                          avtDataAttributes.h                              //
// ************************************************************************* //

#ifndef AVT_DATA_ATTRIBUTES_H
#define AVT_DATA_ATTRIBUTES_H
#include <pipeline_exports.h>

#include <VisWindowTypes.h>

#include <avtTypes.h>
#include <vector>
#include <string>
#include <avtMatrix.h>

class     avtDataObjectString;
class     avtDataObjectWriter;
class     avtExtents;


// ****************************************************************************
//  Method: avtDataAttributes
//
//  Purpose:
//      Contains the data attributes about a data object.  This includes
//      things like extents, dimension, etc.
//
//  Notes:
//      The extents have gotten a bit out of hand, so this an effort at trying
//      to categorize them:
//      True...Extents:    The extents from the original dataset.  However
//                         these extents may be transformed etc.  These are,
//                         for the most part, display extents.
//
//                         The 'true' extents are the extents of the dataset
//                         regardless of its parallel decomposotion, if any.
//                         That is, upon syncronization of parallel processes,
//                         all processors should be forced to agree on the true
//                         extents.
//
//                         In theory the 'true' extents are the extents of the
//                         dataset regardless of whether only some of it has been
//                         read in. However, for databases that don't support
//                         the auxiliary extents data, it would be necessary to
//                         read data that wasn't needed in a pipeline just to
//                         get the 'true' extents set right. So we don't do that.
//
//      Current...Extents: The extents at the bottom of the pipeline for what
//                         is really there.  Used for re-mapping the color to
//                         what actually exists in what is being rendered, etc.
//
//                         The 'current' extents are the extents of what is left,
//                          sort of, after various operations, which may have reduced
//                         the data, such as thresholding, slicing, etc.
//
//      Effective...Extents: Like the current extents, but sometimes maintained
//                         in the middle of a pipeline.  They are used for
//                         things like resampling onto an area smaller than
//                         the true extents to get more bang for our buck with
//                         splatting, etc.
//      Cumulative Variants: If we are doing dynamic load balancing, we can't
//                         what know some extents until we are done executing.
//                         Then we can take all of the pieces and unify them.
//                         This is where the pieces are stored.
//
//                         The cummulative variants are used as places to store
//                         extents information on a per-processor basis *before*
//                         that information is merged and unified across all
//                         processors. Think of the cummulative variants as 
//                         "what this processor has seen so far."
//
//  Programmer: Hank Childs
//  Creation:   March 24, 2001
//
//  Modifications:
//
//    Hank Childs, Mon Aug 13 17:47:07 PDT 2001
//    Added merge routines for extents.
//
//    Hank Childs, Tue Sep  4 13:27:02 PDT 2001
//    Changed extents to use avtExtents.
//
//    Kathleen Bonnell, Wed Sep 19 14:52:10 PDT 2001 
//    Added labels, and associated methods. 
//
//    Kathleen Bonnell, Tue Oct  2 15:25:23 PDT 2001 
//    Added current spatial and data extents, to hold the values
//    at the end of pipeline execution. 
//
//    Hank Childs, Sun Jun 16 19:36:34 PDT 2002
//    Added cellOrigin (origin of the cells within one block).
//
//    Kathleen Bonnell, Wed Mar 26 13:03:54 PST 2003 
//    Added containsOriginalCells, and Set/Get methods. 
//
//    Kathleen Bonnell, Thu Apr 10 10:29:29 PDT 2003  
//    Added transform and Set/Get/Merge/Read/Write/Copy methods. 
//
//    Mark C. Miller, 15Jul03
//    Added Set/GetCanUseCummulativeAsTrueOrCurrent
//    
//    Eric Brugger, Wed Aug 20 09:27:54 PDT 2003
//    Added GetWindowMode and SetWindowMode.
//
//    Mark C. Miller, Thu Jan 29 17:31:20 PST 2004
//    Added bools to Merge method to control ignoring certain attributes
//
//    Mark C. Miller, Sun Feb 29 18:35:00 PST 2004
//    Added GetAnySpatialExtents method
//
// ****************************************************************************

class PIPELINE_API avtDataAttributes
{
  public:
                             avtDataAttributes();
    virtual                 ~avtDataAttributes();

    void                     Copy(const avtDataAttributes &);
    void                     Merge(const avtDataAttributes &,
                                 bool ignoreThisOk = false,
                                 bool ignoreThatOk = false);

    void                     Write(avtDataObjectString &, 
                                   const avtDataObjectWriter *);
    int                      Read(char *);

    int                      GetCycle(void) { return cycle; };
    void                     SetCycle(int);
    bool                     CycleIsAccurate(void) { return cycleIsAccurate; };
    double                   GetTime(void) { return dtime; };
    void                     SetTime(double);
    bool                     TimeIsAccurate(void) { return timeIsAccurate; };

    avtExtents              *GetTrueSpatialExtents(void)
                                    { return trueSpatial; };
    avtExtents              *GetTrueDataExtents(void)
                                    { return trueData; };

    avtExtents              *GetCumulativeTrueSpatialExtents(void)
                                    { return cumulativeTrueSpatial; };
    avtExtents              *GetCumulativeTrueDataExtents(void)
                                    { return cumulativeTrueData; };

    avtExtents              *GetEffectiveSpatialExtents(void)
                                    { return effectiveSpatial; };
    avtExtents              *GetEffectiveDataExtents(void)
                                    { return effectiveData; };

    avtExtents              *GetCurrentSpatialExtents(void)
                                    { return currentSpatial; };
    avtExtents              *GetCurrentDataExtents(void)
                                    { return currentData; };

    avtExtents              *GetCumulativeCurrentSpatialExtents(void)
                                    { return cumulativeCurrentSpatial; };
    avtExtents              *GetCumulativeCurrentDataExtents(void)
                                    { return cumulativeCurrentData; };
    void                     SetCanUseCummulativeAsTrueOrCurrent(bool canUse)
                                { canUseCummulativeAsTrueOrCurrent = canUse; }
    bool                     GetCanUseCummulativeAsTrueOrCurrent(void)
                                { return canUseCummulativeAsTrueOrCurrent; }

    void                     SetTopologicalDimension(int);
    int                      GetTopologicalDimension(void) const
                                { return topologicalDimension; };

    void                     SetSpatialDimension(int);
    int                      GetSpatialDimension(void) const
                                   { return spatialDimension; };

    void                     SetVariableDimension(int);
    int                      GetVariableDimension(void) const
                                   { return variableDimension; };

    avtCentering             GetCentering(void) const
                                   { return centering; };
    void                     SetCentering(avtCentering);

    int                      GetCellOrigin(void) const
                                   { return cellOrigin; };
    void                     SetCellOrigin(int);

    int                      GetBlockOrigin(void) const
                                   { return blockOrigin; };
    void                     SetBlockOrigin(int);

    avtGhostType             GetContainsGhostZones(void) const
                                   { return containsGhostZones; };
    void                     SetContainsGhostZones(avtGhostType v)
                                   { containsGhostZones = v; };

    bool                     GetContainsOriginalCells(void) const
                                   { return containsOriginalCells; };
    void                     SetContainsOriginalCells(bool c)
                                   { containsOriginalCells= c; };

    bool                     GetDataExtents(double *);
    bool                     GetCurrentDataExtents(double *);
    bool                     GetSpatialExtents(double *);
    bool                     GetCurrentSpatialExtents(double *);
    bool                     GetAnySpatialExtents(double *);

    void                     SetLabels(const std::vector<std::string> &l);
    void                     GetLabels(std::vector<std::string> &l);
            
    const std::string       &GetVariableName(void) const { return varname; };
    void                     SetVariableName(const std::string &s)
                                 { varname = s; };
 
    const std::string       &GetFilename(void) const { return filename; };
    void                     SetFilename(const std::string &s) { filename=s; };

    const std::string       &GetXUnits(void) const { return xUnits; };
    void                     SetXUnits(const std::string &s) { xUnits=s; };
    const std::string       &GetYUnits(void) const { return yUnits; };
    void                     SetYUnits(const std::string &s) { yUnits=s; };
    const std::string       &GetZUnits(void) const { return zUnits; };
    void                     SetZUnits(const std::string &s) { zUnits=s; };

    const std::string       &GetXLabel(void) const { return xLabel; };
    void                     SetXLabel(const std::string &s) { xLabel=s; };
    const std::string       &GetYLabel(void) const { return yLabel; };
    void                     SetYLabel(const std::string &s) { yLabel=s; };
    const std::string       &GetZLabel(void) const { return zLabel; };
    void                     SetZLabel(const std::string &s) { zLabel=s; };

    bool                     HasTransform(void); 
    void                     SetTransform(const double *);
    const avtMatrix         *GetTransform(void) { return transform;};
    bool                     GetCanUseTransform(void) { return canUseTransform;};
    void                     SetCanUseTransform(bool b) { canUseTransform = b;};

    WINDOW_MODE              GetWindowMode(void) const { return windowMode;} ;
    void                     SetWindowMode(WINDOW_MODE m) { windowMode = m;} ;

  protected:
    int                      spatialDimension;
    int                      topologicalDimension;
    int                      variableDimension;
    avtCentering             centering;
    int                      cellOrigin;
    int                      blockOrigin;
    double                   dtime;
    bool                     timeIsAccurate;
    int                      cycle;
    bool                     cycleIsAccurate;
    avtGhostType             containsGhostZones;
    bool                     containsOriginalCells;

    avtExtents              *trueSpatial;
    avtExtents              *cumulativeTrueSpatial;
    avtExtents              *effectiveSpatial;
    avtExtents              *currentSpatial;
    avtExtents              *cumulativeCurrentSpatial;
    avtExtents              *trueData;
    avtExtents              *cumulativeTrueData;
    avtExtents              *effectiveData;
    avtExtents              *currentData;
    avtExtents              *cumulativeCurrentData;
    bool                     canUseCummulativeAsTrueOrCurrent;

    avtMatrix               *transform;
    bool                     canUseTransform;
  
    WINDOW_MODE              windowMode;

    std::vector<std::string> labels;
    std::string              varname;
    std::string              filename;
    std::string              xUnits;
    std::string              yUnits;
    std::string              zUnits;
    std::string              xLabel;
    std::string              yLabel;
    std::string              zLabel;

    void                     WriteLabels(avtDataObjectString &,
                                         const avtDataObjectWriter *);
    int                      ReadLabels(char *);
    void                     MergeLabels(const std::vector<std::string> &);
    void                     WriteTransform(avtDataObjectString &,
                                            const avtDataObjectWriter *);
    int                      ReadTransform(char *);
    void                     MergeTransform(const avtMatrix *);
    void                     CopyTransform(const avtMatrix *);
};


#endif

