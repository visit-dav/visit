// ************************************************************************* //
//                          avtDataAttributes.h                              //
// ************************************************************************* //

#ifndef AVT_DATA_ATTRIBUTES_H
#define AVT_DATA_ATTRIBUTES_H
#include <pipeline_exports.h>


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
//      Current...Extents: The extents at the bottom of the pipeline for what
//                         is really there.  Used for re-mapping the color to
//                         what actually exists in what is being rendered, etc.
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
// ****************************************************************************

class PIPELINE_API avtDataAttributes
{
  public:
                             avtDataAttributes();
    virtual                 ~avtDataAttributes();

    void                     Copy(const avtDataAttributes &);
    void                     Merge(const avtDataAttributes &);

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

    bool                     HasTransform(void); 
    void                     SetTransform(const double *);
    const avtMatrix         *GetTransform(void) { return transform;};
    bool                     GetCanUseTransform(void) { return canUseTransform;};
    void                     SetCanUseTransform(bool b) { canUseTransform = b;};


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

    avtMatrix               *transform;
    bool                     canUseTransform;
  
    std::vector<std::string> labels;
    std::string              varname;
    std::string              filename;
    std::string              xUnits;
    std::string              yUnits;
    std::string              zUnits;

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

