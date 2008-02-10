/*****************************************************************************
*
* Copyright (c) 2000 - 2008, Lawrence Livermore National Security, LLC
* Produced at the Lawrence Livermore National Laboratory
* LLNL-CODE-400142
* All rights reserved.
*
* This file is  part of VisIt. For  details, see https://visit.llnl.gov/.  The
* full copyright notice is contained in the file COPYRIGHT located at the root
* of the VisIt distribution or at http://www.llnl.gov/visit/copyright.html.
*
* Redistribution  and  use  in  source  and  binary  forms,  with  or  without
* modification, are permitted provided that the following conditions are met:
*
*  - Redistributions of  source code must  retain the above  copyright notice,
*    this list of conditions and the disclaimer below.
*  - Redistributions in binary form must reproduce the above copyright notice,
*    this  list of  conditions  and  the  disclaimer (as noted below)  in  the
*    documentation and/or other materials provided with the distribution.
*  - Neither the name of  the LLNS/LLNL nor the names of  its contributors may
*    be used to endorse or promote products derived from this software without
*    specific prior written permission.
*
* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT  HOLDERS AND CONTRIBUTORS "AS IS"
* AND ANY EXPRESS OR  IMPLIED WARRANTIES, INCLUDING,  BUT NOT  LIMITED TO, THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND  FITNESS FOR A PARTICULAR  PURPOSE
* ARE  DISCLAIMED. IN  NO EVENT  SHALL LAWRENCE  LIVERMORE NATIONAL  SECURITY,
* LLC, THE  U.S.  DEPARTMENT OF  ENERGY  OR  CONTRIBUTORS BE  LIABLE  FOR  ANY
* DIRECT,  INDIRECT,   INCIDENTAL,   SPECIAL,   EXEMPLARY,  OR   CONSEQUENTIAL
* DAMAGES (INCLUDING, BUT NOT  LIMITED TO, PROCUREMENT OF  SUBSTITUTE GOODS OR
* SERVICES; LOSS OF  USE, DATA, OR PROFITS; OR  BUSINESS INTERRUPTION) HOWEVER
* CAUSED  AND  ON  ANY  THEORY  OF  LIABILITY,  WHETHER  IN  CONTRACT,  STRICT
* LIABILITY, OR TORT  (INCLUDING NEGLIGENCE OR OTHERWISE)  ARISING IN ANY  WAY
* OUT OF THE  USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH
* DAMAGE.
*
*****************************************************************************/

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
class     avtWebpage;
class     PlotInfoAttributes;


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
//                         dataset regardless of whether only some of it has
//                         been read in. However, for databases that don't 
//                         support the auxiliary extents data, it would be
//                         necessary to read data that wasn't needed in a 
//                         pipeline just to get the 'true' extents set right. 
//                         So we don't do that.
//
//      Current...Extents: The extents at the bottom of the pipeline for what
//                         is really there.  Used for re-mapping the color to
//                         what actually exists in what is being rendered, etc.
//
//                         The 'current' extents are the extents of what is
//                         left, sort of, after various operations, which may 
//                         have reduced the data, such as thresholding, 
//                         slicing, etc.
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
//                         The cumulative variants are used as places to store
//                         extents information on a per-processor basis *before*
//                         that information is merged and unified across all
//                         processors. Think of the cumulative variants as 
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
//    Added Set/GetCanUseCumulativeAsTrueOrCurrent
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
//    Kathleen Bonnell, Tue Jun  1 15:08:30 PDT 2004 
//    Added containsOriginalNodes, invTransform and Set/Get methods. 
//
//    Kathleen Bonnell, Thu Jul 22 12:10:19 PDT 2004 
//    Added treatAsASCII to VarInfo struct, and Set/Get methods. 
//
//    Brad Whitlock, Tue Jul 20 12:19:38 PDT 2004
//    Added support for units on variables.
//
//    Mark C. Miller, Tue Sep 28 19:57:42 PDT 2004
//    Added support for data selections that are applied by plugins
//
//    Kathleen Bonnell, Tue Oct 12 16:11:15 PDT 2004 
//    Added keepNodeZoneArrays and Set/Get methods. 
//
//    Jeremy Meredith, Wed Nov  3 12:13:39 PST 2004
//    Changed a typedef anonymous struct to a normal struct.  xlC was not
//    generating methods (e.g. copy constructor) using the C-style declaration.
//
//    Kathleen Bonnell, Thu Dec  9 16:12:33 PST 2004 
//    Added containsGlobalNode/ZoneIds and Set/Get methods. 
//
//    Hank Childs, Sat Jan  1 11:23:50 PST 2005
//    Set the name of the mesh.
//
//    Hank Childs, Thu Jan 20 09:40:25 PST 2005
//    Added TransformSpatialExtents.
//
//    Kathleen Bonnell, Thu Jan 27 09:14:35 PST 2005 
//    Added numStates and Set/Get methods. 
//
//    Kathleen Bonnell, Thu Feb  3 09:27:22 PST 2005 
//    Added mirOccurred and Set/Get methods. 
//
//    Hank Childs, Thu Aug  4 09:21:34 PDT 2005
//    Added type and variable names (for arrays).
//
//    Kathleen Bonnell, Thu Aug  4 15:47:59 PDT 2005 
//    Added canUseOrigZones, origNodesRequiredForPick and Set/Get methods.
//
//    Jeremy Meredith, Thu Aug 25 11:06:41 PDT 2005
//    Added origin for groups.
//
//    Hank Childs, Fri Oct  7 08:31:30 PDT 2005
//    Added fullDBName.
//
//    Kathleen Bonnell, Fri Feb  3 10:32:12 PST 2006
//    Added meshCoordType.
//
//    Kathleen Bonnell, Mon May  1 08:57:41 PDT 2006 
//    Changed origNodesRequiredForPick to origElementsRequiredForPick.
//
//    Kathleen Bonnell, Tue Jun 20 16:02:38 PDT 2006
//    Added Set/Get/Read/Write PlotInfoAtts. 
//
//    Jeremy Meredith, Mon Aug 28 16:46:29 EDT 2006
//    Added nodesAreCritical.  Added unitCellVectors.
//
//    Hank Childs, Thu Dec 21 10:11:30 PST 2006
//    Add support for debug dumps.
//
//    Hank Childs, Fri Jan 12 13:00:31 PST 2007
//    Add bounds for array variables.
//
//    Jeremy Meredith, Thu Feb 15 11:44:28 EST 2007
//    Added support for rectilinear grids with an inherent transform.
//
//    Mark C. Miller, Tue Mar 27 08:39:55 PDT 2007
//    Added node origin
//
//    Kathleen Bonnell, Fri Jun 22 13:41:14 PDT 2007 
//    Added meshType.
//
//    Hank Childs, Fri Aug 31 08:48:40 PDT 2007
//    Added adaptsToAnyWindowMode.
//
//    Hank Childs, Sun Oct 28 09:23:27 PST 2007
//    Added containsExteriorBoundaryGhosts.
//
//    Hank Childs, Tue Dec 18 10:04:43 PST 2007
//    Define private copy constructor and assignment operator to prevent
//    accidental use of default, bitwise copy implementations.
//
//    Jeremy Meredith, Wed Jan 30 13:11:46 EST 2008
//    Added ability to specify that a variable is supposed to be used for
//    an axis (and which one), e.g. for a parallel coordinates plot.
//
//    Jeremy Meredith, Thu Feb  7 17:55:39 EST 2008
//    Added extents that can be associated with individual components of
//    array variables.
//
// ****************************************************************************

class PIPELINE_API avtDataAttributes
{
  public:
                             avtDataAttributes();
    virtual                 ~avtDataAttributes();

    void                     Print(ostream &);

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
    avtExtents              *GetTrueDataExtents(const char * = NULL);

    avtExtents              *GetCumulativeTrueSpatialExtents(void)
                                    { return cumulativeTrueSpatial; };
    avtExtents              *GetCumulativeTrueDataExtents(const char * = NULL);

    avtExtents              *GetEffectiveSpatialExtents(void)
                                    { return effectiveSpatial; };
    avtExtents              *GetEffectiveDataExtents(const char * = NULL);

    avtExtents              *GetCurrentSpatialExtents(void)
                                    { return currentSpatial; };
    avtExtents              *GetCurrentDataExtents(const char * = NULL);

    avtExtents              *GetCumulativeCurrentSpatialExtents(void)
                                    { return cumulativeCurrentSpatial; };
    avtExtents              *GetCumulativeCurrentDataExtents(const char * 
                                                             = NULL);

    void                     SetCanUseCumulativeAsTrueOrCurrent(bool canUse)
                                { canUseCumulativeAsTrueOrCurrent = canUse; }
    bool                     GetCanUseCumulativeAsTrueOrCurrent(void)
                                { return canUseCumulativeAsTrueOrCurrent; }

    void                     SetTopologicalDimension(int);
    int                      GetTopologicalDimension(void) const
                                { return topologicalDimension; };

    void                     SetSpatialDimension(int);
    int                      GetSpatialDimension(void) const
                                   { return spatialDimension; };

    void                     SetVariableDimension(int, const char * = NULL);
    int                      GetVariableDimension(const char * = NULL) const;

    void                     SetVariableType(avtVarType, const char * = NULL);
    avtVarType               GetVariableType(const char * = NULL) const;

    void                     SetVariableSubnames(
                                              const std::vector<std::string> &,
                                              const char * = NULL);
    const std::vector<std::string> &
                             GetVariableSubnames(const char * = NULL) const;
    void                     SetVariableBinRanges(
                                              const std::vector<double> &,
                                              const char * = NULL);
    const std::vector<double> &
                             GetVariableBinRanges(const char * = NULL) const;

    avtExtents              *GetVariableComponentExtents(const char * = NULL);

    avtCentering             GetCentering(const char * = NULL) const;
    void                     SetCentering(avtCentering, const char * = NULL);

    bool                     GetTreatAsASCII(const char * = NULL) const;
    void                     SetTreatAsASCII(const bool, const char * = NULL);

    int                      GetUseForAxis(const char * = NULL) const;
    void                     SetUseForAxis(const int, const char * = NULL);
    void                     ClearAllUseForAxis();

    int                      GetCellOrigin(void) const
                                   { return cellOrigin; };
    void                     SetCellOrigin(int);

    int                      GetNodeOrigin(void) const
                                   { return nodeOrigin; };
    void                     SetNodeOrigin(int);

    int                      GetBlockOrigin(void) const
                                   { return blockOrigin; };
    void                     SetBlockOrigin(int);

    int                      GetGroupOrigin(void) const
                                   { return groupOrigin; };
    void                     SetGroupOrigin(int);

    avtGhostType             GetContainsGhostZones(void) const
                                   { return containsGhostZones; };
    void                     SetContainsGhostZones(avtGhostType v)
                                   { containsGhostZones = v; };

    bool                     GetContainsExteriorBoundaryGhosts(void) const
                                   { return containsExteriorBoundaryGhosts; };
    void                     SetContainsExteriorBoundaryGhosts(bool v)
                                   { containsExteriorBoundaryGhosts= v; };

    bool                     GetContainsOriginalCells(void) const
                                   { return containsOriginalCells; };
    void                     SetContainsOriginalCells(bool c)
                                   { containsOriginalCells= c; };
    bool                     GetContainsOriginalNodes(void) const
                                   { return containsOriginalNodes; };
    void                     SetContainsOriginalNodes(bool c)
                                   { containsOriginalNodes= c; };
    bool                     GetKeepNodeZoneArrays(void) const
                                   { return keepNodeZoneArrays; };
    void                     SetKeepNodeZoneArrays(bool k)
                                   { keepNodeZoneArrays= k; };

    bool                     GetDataExtents(double *, const char * = NULL);
    bool                     GetCurrentDataExtents(double *,
                                                   const char * = NULL);
    bool                     GetSpatialExtents(double *);
    bool                     GetCurrentSpatialExtents(double *);
    bool                     GetAnySpatialExtents(double *);

    void                     SetLabels(const std::vector<std::string> &l);
    void                     GetLabels(std::vector<std::string> &l);
            
    const std::string       &GetVariableName(void) const;
    const std::string       &GetVariableName(int) const;
    const std::string       &GetVariableUnits(const char * = NULL) const;
    const std::string       &GetVariableUnits(int) const;
    int                      GetNumberOfVariables(void) const;
    void                     SetActiveVariable(const char *);
    void                     AddVariable(const std::string &s);
    void                     AddVariable(const std::string &s,
                                         const std::string &units);
    void                     RemoveVariable(const std::string &s);
    bool                     ValidVariable(const std::string &s) const;
    bool                     ValidActiveVariable(void) const;
 
    const std::string       &GetMeshname(void) const { return meshname; };
    void                     SetMeshname(const std::string &s) { meshname=s; };
    const std::string       &GetFilename(void) const { return filename; };
    void                     SetFilename(const std::string &s) { filename=s; };
    const std::string       &GetFullDBName(void) const { return fullDBName; };
    void                     SetFullDBName(const std::string &s) 
                                                     { fullDBName=s; };

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

    bool                     HasInvTransform(void); 
    void                     SetInvTransform(const double *);
    const avtMatrix         *GetInvTransform(void) { return invTransform;};
    bool                     GetCanUseInvTransform(void) 
                                                    { return canUseInvTransform;};
    void                     SetCanUseInvTransform(bool b) { canUseInvTransform =b;};

    bool                     HasTransform(void); 
    void                     SetTransform(const double *);
    const avtMatrix         *GetTransform(void) { return transform;};
    bool                     GetCanUseTransform(void) 
                                                    { return canUseTransform;};
    void                     SetCanUseTransform(bool b) { canUseTransform =b;};

    WINDOW_MODE              GetWindowMode(void) const { return windowMode;} ;
    void                     SetWindowMode(WINDOW_MODE m) { windowMode = m;} ;

    bool                     GetAdaptsToAnyWindowMode(void) const 
                                           { return adaptsToAnyWindowMode;} ;
    void                     SetAdaptsToAnyWindowMode(bool b) 
                                           { adaptsToAnyWindowMode = b; };

    void                     SetSelectionsApplied(std::vector<bool> &selsApplied);
    bool                     GetSelectionApplied(int selID) const;
    const std::vector<bool> &GetSelectionsApplied() const;

    bool                     GetContainsGlobalZoneIds(void) const
                                   { return containsGlobalZoneIds; };
    void                     SetContainsGlobalZoneIds(bool c)
                                   { containsGlobalZoneIds= c; };
    bool                     GetContainsGlobalNodeIds(void) const
                                   { return containsGlobalNodeIds; };
    void                     SetContainsGlobalNodeIds(bool c)
                                   { containsGlobalNodeIds= c; };

    void                     TransformSpatialExtents(avtDataAttributes &,
                                void (*)(const double *,double*,void*),void *);

    int                      GetNumStates(void) const
                                   { return numStates; };
    void                     SetNumStates(int n)
                                   { numStates = n; };

    bool                     MIROccurred(void) const
                                   { return mirOccurred; };
    void                     SetMIROccurred(bool mo)
                                   { mirOccurred = mo; };

    bool                     CanUseOrigZones(void) const
                                   { return canUseOrigZones; };
    void                     SetCanUseOrigZones(bool v)
                                   { canUseOrigZones = v; };

    bool                     OrigElementsRequiredForPick(void) const
                                   { return origElementsRequiredForPick; };
    void                     SetOrigElementsRequiredForPick(bool v)
                                   { origElementsRequiredForPick = v; };

    avtMeshType              GetMeshType(void) const
                                   { return meshType; };
    void                     SetMeshType(avtMeshType t)
                                   { meshType = t; };

    avtMeshCoordType         GetMeshCoordType(void) const
                                   { return meshCoordType; };
    void                     SetMeshCoordType(avtMeshCoordType t)
                                   { meshCoordType = t; };

    bool                     NodesAreCritical(void) const
                                   { return nodesAreCritical; };
    void                     SetNodesAreCritical(bool v)
                                   { nodesAreCritical = v; };

    const float             *GetUnitCellVectors(void) const
                                   { return unitCellVectors; }
    void                     SetUnitCellVectors(const float *v)
                             { for (int i=0;i<9;i++) unitCellVectors[i]=v[i]; }

    bool                     GetRectilinearGridHasTransform() const
                             { return rectilinearGridHasTransform; }
    void                     SetRectilinearGridHasTransform(bool v)
                             { rectilinearGridHasTransform = v; }

    const double            *GetRectilinearGridTransform(void) const
                                   { return rectilinearGridTransform; }
    void                     SetRectilinearGridTransform(const double *v)
                   { for (int i=0;i<16;i++) rectilinearGridTransform[i]=v[i]; }

    const PlotInfoAttributes *GetPlotInfoAtts(void) const
                                   { return plotInfoAtts; };
    void                     SetPlotInfoAtts(const PlotInfoAttributes *);

    void                     DebugDump(avtWebpage *);

  protected:
    int                      spatialDimension;
    int                      topologicalDimension;
    int                      cellOrigin;
    int                      nodeOrigin;
    int                      blockOrigin;
    int                      groupOrigin;
    double                   dtime;
    bool                     timeIsAccurate;
    int                      cycle;
    bool                     cycleIsAccurate;
    avtGhostType             containsGhostZones;
    bool                     containsExteriorBoundaryGhosts;
    bool                     containsOriginalCells;
    bool                     containsOriginalNodes;
    bool                     keepNodeZoneArrays;
    bool                     containsGlobalZoneIds;
    bool                     containsGlobalNodeIds;
    avtMatrix               *invTransform;
    bool                     canUseInvTransform;
    avtMatrix               *transform;
    bool                     canUseTransform;
    bool                     canUseCumulativeAsTrueOrCurrent;
    int                      numStates;
    bool                     mirOccurred;
    bool                     canUseOrigZones;
    bool                     origElementsRequiredForPick;
    avtMeshType              meshType;
    avtMeshCoordType         meshCoordType;
    bool                     nodesAreCritical;
    float                    unitCellVectors[9];
    bool                     rectilinearGridHasTransform;
    double                   rectilinearGridTransform[16];

    avtExtents              *trueSpatial;
    avtExtents              *cumulativeTrueSpatial;
    avtExtents              *effectiveSpatial;
    avtExtents              *currentSpatial;
    avtExtents              *cumulativeCurrentSpatial;

    struct VarInfo
    {
        std::string          varname;
        avtVarType           vartype;
        std::string          varunits;
        int                  dimension;
        avtCentering         centering;
        bool                 treatAsASCII;
        avtExtents          *trueData;
        avtExtents          *cumulativeTrueData;
        avtExtents          *effectiveData;
        avtExtents          *currentData;
        avtExtents          *cumulativeCurrentData;
        int                  useForAxis;
        std::vector<std::string>  subnames; // Only used for 'array' vars
                                            // at this point.
        std::vector<double>       binRange; // Only used for 'array' vars
                                            // at this point.
        avtExtents          *componentExtents; // Only used for 'array' vars
    };
    std::vector<VarInfo>     variables;
    int                      activeVariable;

    WINDOW_MODE              windowMode;
    bool                     adaptsToAnyWindowMode;

    std::vector<std::string> labels;
    std::string              meshname;

    // Filename: a short name appropriate for placing in a legend.
    std::string              filename;

    // FullDBName: a fully path qualified name for the whole database.  This
    // can even include virtual database strings.  This is the name used by
    // the network manager.
    std::string              fullDBName;

    std::string              xUnits;
    std::string              yUnits;
    std::string              zUnits;
    std::string              xLabel;
    std::string              yLabel;
    std::string              zLabel;

    std::vector<bool>        selectionsApplied;

    PlotInfoAttributes      *plotInfoAtts;

    void                     WriteLabels(avtDataObjectString &,
                                         const avtDataObjectWriter *);
    int                      ReadLabels(char *);
    void                     MergeLabels(const std::vector<std::string> &);
    void                     WriteInvTransform(avtDataObjectString &,
                                            const avtDataObjectWriter *);
    int                      ReadInvTransform(char *);
    void                     MergeInvTransform(const avtMatrix *);
    void                     CopyInvTransform(const avtMatrix *);

    void                     WriteTransform(avtDataObjectString &,
                                            const avtDataObjectWriter *);
    int                      ReadTransform(char *);
    void                     MergeTransform(const avtMatrix *);
    void                     CopyTransform(const avtMatrix *);

    int                      VariableNameToIndex(const char *) const;

    void                     DestructSelf(void);

    void                     WritePlotInfoAtts(avtDataObjectString &,
                                         const avtDataObjectWriter *);
    int                      ReadPlotInfoAtts(char *);

  private:
    // These methods are defined to prevent accidental use of bitwise copy
    // implementations.  If you want to re-define them to do something 
    // meaningful, that's fine.
                             avtDataAttributes(const avtDataAttributes &) {;};
    avtDataAttributes       &operator=(const avtDataAttributes &) 
                                                            { return *this; };

};


#endif


