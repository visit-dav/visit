// ************************************************************************* //
//                               avtDataValidity.h                           //
// ************************************************************************* //

#ifndef AVT_DATA_VALIDITY_H
#define AVT_DATA_VALIDITY_H

#include <pipeline_exports.h>

#include <string>

class     avtDataObjectString;
class     avtDataObjectWriter;


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
// ****************************************************************************

class PIPELINE_API avtDataValidity
{
  public:
                             avtDataValidity();
    virtual                 ~avtDataValidity() {;};

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
 
    void                     InvalidateZones(void)
                                   { zonesPreserved = false; };
    bool                     GetZonesPreserved(void) const
                                   { return zonesPreserved; };

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

    void                     SetIsThisDynamic(bool itd)
                                   { isThisDynamic = itd; };
    bool                     GetIsThisDynamic(void)
                                   { return isThisDynamic; };

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

    void                     ErrorOccurred(void) { errorOccurred = true; };
    void                     ResetErrorOccurred(void)
                                                 { errorOccurred = false; };
    bool                     HasErrorOccurred(void) { return errorOccurred; };
    void                     SetErrorMessage(std::string &s)
                                                          { errorString = s; };
    std::string             &GetErrorMessage(void) { return errorString; };

  protected:
    bool                     zonesPreserved;
    bool                     spatialMetaDataPreserved;
    bool                     dataMetaDataPreserved;
    bool                     operationFailed;
    bool                     usingAllData;
    bool                     usingAllDomains;
    bool                     isThisDynamic;
    bool                     pointsWereTransformed;
    bool                     wireframeRenderingIsInappropriate;
    bool                     normalsAreInappropriate;
    bool                     subdivisionOccurred;
    bool                     notAllCellsSubdivided;
    bool                     disjointElements;
    bool                     queryable;

    bool                     errorOccurred;
    std::string              errorString;
};


#endif


