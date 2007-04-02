/*****************************************************************************
*
* Copyright (c) 2000 - 2006, The Regents of the University of California
* Produced at the Lawrence Livermore National Laboratory
* All rights reserved.
*
* This file is part of VisIt. For details, see http://www.llnl.gov/visit/. The
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
*    documentation and/or materials provided with the distribution.
*  - Neither the name of the UC/LLNL nor  the names of its contributors may be
*    used to  endorse or  promote products derived from  this software without
*    specific prior written permission.
*
* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT  HOLDERS AND CONTRIBUTORS "AS IS"
* AND ANY EXPRESS OR  IMPLIED WARRANTIES, INCLUDING,  BUT NOT  LIMITED TO, THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND  FITNESS FOR A PARTICULAR  PURPOSE
* ARE  DISCLAIMED.  IN  NO  EVENT  SHALL  THE  REGENTS  OF  THE  UNIVERSITY OF
* CALIFORNIA, THE U.S.  DEPARTMENT  OF  ENERGY OR CONTRIBUTORS BE  LIABLE  FOR
* ANY  DIRECT,  INDIRECT,  INCIDENTAL,  SPECIAL,  EXEMPLARY,  OR CONSEQUENTIAL
* DAMAGES (INCLUDING, BUT NOT  LIMITED TO, PROCUREMENT OF  SUBSTITUTE GOODS OR
* SERVICES; LOSS OF  USE, DATA, OR PROFITS; OR  BUSINESS INTERRUPTION) HOWEVER
* CAUSED  AND  ON  ANY  THEORY  OF  LIABILITY,  WHETHER  IN  CONTRACT,  STRICT
* LIABILITY, OR TORT  (INCLUDING NEGLIGENCE OR OTHERWISE)  ARISING IN ANY  WAY
* OUT OF THE  USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH
* DAMAGE.
*
*****************************************************************************/

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

  protected:
    bool                     zonesPreserved;
    bool                     originalZonesIntact;
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
    bool                     hasEverOwnedAnyDomain;

    bool                     errorOccurred;
    std::string              errorString;
};


#endif


