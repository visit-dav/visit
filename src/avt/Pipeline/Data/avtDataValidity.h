// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                               avtDataValidity.h                           //
// ************************************************************************* //

#ifndef AVT_DATA_VALIDITY_H
#define AVT_DATA_VALIDITY_H

#include <pipeline_exports.h>

#include <string>

class     avtDataObjectString;
class     avtDataObjectWriter;
class     avtWebpage;


// ****************************************************************************
//  Class: avtDataValidity
//
//  Purpose:
//      Contains many descriptions that may be useful in determining if
//      meta-data can still be applied to the class.
//
//  Programmer: Hank Childs
//  Creation:   March 24, 2001
// 
//  Modifications:
//
//    Hank Childs, Sun Jun 17 10:17:19 PDT 2001
//    Added usingAllData.
//
//    Hank Childs, Fri Sep 14 09:38:58 PDT 2001
//    Added usingAllDomains.
//
//    Kathleen Bonnell, Fri Oct 12 12:07:01 PDT 2001 
//    Added isThisDynamic.
//
//    Kathleen Bonnell, Wed Dec 12 10:50:01 PST 2001 
//    Added pointsWereTransformed.
//
//    Hank Childs, Sun Jun 23 23:05:44 PDT 2002
//    Added wireframeRenderingIsInappropriate.
//
//    Hank Childs, Tue Aug  6 10:57:20 PDT 2002
//    Added normalsAreInappropriate.
//
//    Jeremy Meredith, Tue Aug 13 09:51:46 PDT 2002
//    Added SubdivisionOccurred and NotAllCellsSubdivided.
//
//    Hank Childs, Sun Aug 18 10:58:23 PDT 2002
//    Added disjointElements.
//
//    Kathleen Bonnell, Wed Oct 23 15:11:44 PDT 2002  
//    Added queryable.
//
//    Hank Childs, Fri May 16 10:15:54 PDT 2003
//    Add error handling.
//
//    Mark C. Miller, Thu Jan 29 16:40:25 PST 2004
//    Added hasOwnedAnyDomain
//
//    Hank Childs, Thu Feb  5 17:11:06 PST 2004
//    Moved inlined constructor and destructor definitions to .C files
//    because certain compilers have problems with them.
//
//    Kathleen Bonnell, Thu Mar  2 14:04:06 PST 2006 
//    Added ZonesSplit() and GetOriginalZonesIntact().
//
//    Kathleen Bonnell, Thu Oct 26 09:17:08 PDT 2006 
//    Added InvalidateNodes and GetNodesPreserved.
//
//    Hank Childs, Thu Dec 21 10:11:30 PST 2006
//    Add support for debug dumps.
//
//    Hank Childs, Tue Dec 18 10:04:43 PST 2007
//    Define private copy constructor and assignment operator to prevent
//    accidental use of default, bitwise copy implementations.
//
//    Hank Childs, Tue Feb 19 19:45:43 PST 2008
//    Rename "dynamic" to "streaming", since we really care about whether we
//    are streaming, not about whether we are doing dynamic load balancing.
//    And the two are no longer synonymous.
//
//    Hank Childs, Sun Jun 21 15:29:01 PDT 2015
//    Add a bool to disable the downstream modules from attempting streaming.
//
// ****************************************************************************

class PIPELINE_API avtDataValidity
{
  public:
                             avtDataValidity();
    virtual                 ~avtDataValidity();
    void                     Reset(void);

    void                     Copy(const avtDataValidity &);
    void                     Merge(const avtDataValidity &);

    void                     Write(avtDataObjectString &,
                                   const avtDataObjectWriter *);
    int                      Read(char *);

    void                     InvalidateSpatialMetaData(void)
                                   { spatialMetaDataPreserved = false; };
    bool                     GetSpatialMetaDataPreserved(void) const
                                   { return spatialMetaDataPreserved; };
 
    void                     InvalidateDataMetaData(void)
                                   { dataMetaDataPreserved = false; };
    bool                     GetDataMetaDataPreserved(void) const
                                   { return dataMetaDataPreserved; };

    // 
    //  This has more to do with zone-numbering than with
    //  the zones being subdivided, tetrahedralized, etc.
    // 
    void                     InvalidateZones(void)
                                   { zonesPreserved = false; };
    bool                     GetZonesPreserved(void) const
                                   { return zonesPreserved; };
    // 
    //  Indicates that node numberings have changed. 
    // 
    void                     InvalidateNodes(void)
                                   { nodesPreserved = false; };
    bool                     GetNodesPreserved(void) const
                                   { return nodesPreserved; };
    // 
    //  This has more to do with zone being split, by
    //  clipping, slicing, etc.
    // 
    void                     ZonesSplit(void)
                                   { originalZonesIntact = false; };
    bool                     GetOriginalZonesIntact(void) const
                                   { return originalZonesIntact; };

    void                     InvalidateOperation(void)
                                   { operationFailed = true; };
    bool                     GetOperationFailed(void) const
                                   { return operationFailed; };
 
    void                     SetUsingAllData(bool uad)
                                   { usingAllData = uad; };
    bool                     GetUsingAllData(void)
                                   { return usingAllData; };

    void                     SetUsingAllDomains(bool uad)
                                   { usingAllDomains = uad; };
    bool                     GetUsingAllDomains(void)
                                   { return usingAllDomains; };

    void                     SetWhetherStreaming(bool itd)
                                   { streaming = itd; };
    bool                     AreWeStreaming(void)
                                   { return streaming; };
    void                     SetWhetherStreamingPossible(bool sp)
                                   { streamingPossible = sp; };
    bool                     IsStreamingPossible(void)
                                   { return streamingPossible; };

    void                     SetPointsWereTransformed(bool pwt)
                                   { pointsWereTransformed = pwt; };
    bool                     GetPointsWereTransformed(void)
                                   { return pointsWereTransformed; };

    void                     SetWireframeRenderingIsInappropriate(bool wr)
                                   { wireframeRenderingIsInappropriate = wr; };
    bool                     GetWireframeRenderingIsInappropriate(void)
                                   {return wireframeRenderingIsInappropriate;};

    void                     SetNormalsAreInappropriate(bool n)
                                   { normalsAreInappropriate = n; };
    bool                     NormalsAreInappropriate(void)
                                   {return normalsAreInappropriate;};

    void                     SetSubdivisionOccurred(bool subdiv)
                                   { subdivisionOccurred = subdiv; }
    bool                     SubdivisionOccurred(void)
                                   { return subdivisionOccurred; }

    void                     SetNotAllCellsSubdivided(bool notall)
                                   { notAllCellsSubdivided = notall; }
    bool                     NotAllCellsSubdivided(void)
                                   { return notAllCellsSubdivided; }

    void                     SetDisjointElements(bool val)
                                   { disjointElements = val; }
    bool                     GetDisjointElements(void)
                                   { return disjointElements; }

    void                     SetQueryable(bool val)
                                   { queryable = val; }
    bool                     GetQueryable(void)
                                   { return queryable; }

    void                     SetHasEverOwnedAnyDomain(bool val)
                                   { hasEverOwnedAnyDomain = val; }
    bool                     GetHasEverOwnedAnyDomain(void) const
                                   { return hasEverOwnedAnyDomain; }


    void                     ErrorOccurred(void) { errorOccurred = true; };
    void                     ResetErrorOccurred(void)
                                                 { errorOccurred = false; };
    bool                     HasErrorOccurred(void) { return errorOccurred; };
    void                     SetErrorMessage(std::string &s)
                                                          { errorString = s; };
    std::string             &GetErrorMessage(void) { return errorString; };

    void                     DebugDump(avtWebpage *);

  protected:
    bool                     zonesPreserved;
    bool                     nodesPreserved;
    bool                     originalZonesIntact;
    bool                     spatialMetaDataPreserved;
    bool                     dataMetaDataPreserved;
    bool                     operationFailed;
    bool                     usingAllData;
    bool                     usingAllDomains;
    bool                     streaming;
    bool                     streamingPossible;
    bool                     pointsWereTransformed;
    bool                     wireframeRenderingIsInappropriate;
    bool                     normalsAreInappropriate;
    bool                     subdivisionOccurred;
    bool                     notAllCellsSubdivided;
    bool                     disjointElements;
    bool                     queryable;
    bool                     hasEverOwnedAnyDomain;

    bool                     errorOccurred;
    std::string              errorString;

  private:
    // These methods are defined to prevent accidental use of bitwise copy
    // implementations.  If you want to re-define them to do something
    // meaningful, that's fine.
                         avtDataValidity(const avtDataValidity &) {;};
    avtDataValidity     &operator=(const avtDataValidity &) { return *this; };
};


#endif


