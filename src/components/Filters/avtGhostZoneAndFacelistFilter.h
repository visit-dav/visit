/*****************************************************************************
*
* Copyright (c) 2000 - 2007, The Regents of the University of California
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
//                        avtGhostZoneAndFacelistFilter.h                    //
// ************************************************************************* //

#ifndef AVT_GHOSTZONE_AND_FACELIST_FILTER_H
#define AVT_GHOSTZONE_AND_FACELIST_FILTER_H

#include <filters_exports.h>

#include <avtDatasetToDatasetFilter.h>

class   avtGhostZoneFilter;
class   avtFacelistFilter;


// ****************************************************************************
//  Class: avtGhostZoneAndFacelistFilter
//
//  Purpose:
//    Applies the ghostZone and facelist filters, after
//    determining which should come first. 
//
//  Programmer: Kathleen Bonnell 
//  Creation:   July 20, 2001 
//
//  Modifications:
//
//    Hank Childs, Fri Jul 27 14:54:13 PDT 2001
//    Added RefashionDataObjectInfo.
//
//    Hank Childs, Thu Sep  6 11:14:38 PDT 2001
//    Allowed for dynamic load balancing.
//
//    Hank Childs, Wed Aug 11 09:46:53 PDT 2004
//    Added PerformRestriction.
//
//    Hank Childs, Fri Sep 23 10:38:31 PDT 2005
//    Add support for edge lists.
//
//    Hank Childs, Wed Dec 20 09:34:20 PST 2006
//    Add support for new methods with facelist filter and ghost zone filter
//    to ensure that ghosts are removed and polydata is produced.
//
//    Jeremy Meredith, Thu Feb 15 11:44:28 EST 2007
//    Added support for rectilinear grids with an inherent transform.
//
// ****************************************************************************

class AVTFILTERS_API avtGhostZoneAndFacelistFilter :
                                               public avtDatasetToDatasetFilter
{
  public:
                          avtGhostZoneAndFacelistFilter();
    virtual              ~avtGhostZoneAndFacelistFilter(); 

    virtual const char   *GetType(void)
                                   { return "avtGhostZoneAndFacelistFilter"; };
    virtual const char   *GetDescription(void)
                                   { return "Finding visible triangles"; };

    void                  SetUseFaceFilter(bool val) { useFaceFilter = val; };
    void                  SetCreate3DCellNumbers(bool val);
    void                  SetForceFaceConsolidation(bool);
    void                  SetCreateEdgeListFor2DDatasets(bool val);
    void                  GhostDataMustBeRemoved();
    void                  MustCreatePolyData();

    virtual void          ReleaseData(void);

  protected:
    avtGhostZoneFilter   *ghostFilter;
    avtFacelistFilter    *faceFilter;
    bool                  useFaceFilter;
    bool                  useGhostFilter;

    virtual void          Execute(void);
    virtual void          RefashionDataObjectInfo(void);
    virtual int           AdditionalPipelineFilters(void);
    virtual void          ChangedInput(void);
    virtual avtPipelineSpecification_p
                          PerformRestriction(avtPipelineSpecification_p);
    virtual bool          FilterUnderstandsTransformedRectMesh();
};


#endif


