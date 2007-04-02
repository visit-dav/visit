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
//                  avtGhostZoneAndFacelistFilter.h                          //
// ************************************************************************* //

#include <avtGhostZoneAndFacelistFilter.h>

#include <avtFacelistFilter.h>
#include <avtGhostZoneFilter.h>
#include <avtSourceFromAVTDataset.h>

#include <DebugStream.h>
#include <TimingsManager.h>


// ****************************************************************************
//  Method: avtGhostZoneAndFacelistFilter constructor
//
//  Programmer: Kathleen Bonnell
//  Creation:   July 20, 2001
//
// ****************************************************************************

avtGhostZoneAndFacelistFilter::avtGhostZoneAndFacelistFilter()
{
    ghostFilter = new avtGhostZoneFilter;
    faceFilter  = new avtFacelistFilter;
    useFaceFilter = false;
    useGhostFilter = true;
}


// ****************************************************************************
//  Method: avtGhostZoneAndFacelistFilter destructor
//
//  Programmer: Kathleen Bonnell
//  Creation:   July 20, 2001
//
// ****************************************************************************

avtGhostZoneAndFacelistFilter::~avtGhostZoneAndFacelistFilter()
{
    if (ghostFilter != NULL)
    {
        delete ghostFilter;
        ghostFilter = NULL;
    }
    if (faceFilter != NULL)
    {
        delete faceFilter;
        faceFilter = NULL;
    }
}


// ****************************************************************************
//  Method: avtGhostZoneAndFacelistFilter::AdditionalPipelineFilters
//
//  Purpose:
//      Returns how many pipeline filters will be executing.
//
//  Programmer: Hank Childs
//  Creation:   September 30, 2002
//
// ****************************************************************************

int
avtGhostZoneAndFacelistFilter::AdditionalPipelineFilters(void)
{
    int vals = 0;

    if (useFaceFilter)
        vals++;
    if (useGhostFilter)
        vals++;

    return vals;
}


// ****************************************************************************
//  Method: avtGhostZoneAndFacelistFilter::SetCreateEdgeListFor2DDatasets
//
//  Purpose:
//      This is a Boolean that is passed directly to the facelist filter.  
//      It is used to tell the facelist filter to create an external edge list
//      when dealing with 2D data sets.
//
//  Programmer: Hank Childs
//  Creation:   September 23, 2005
//
// ****************************************************************************

void
avtGhostZoneAndFacelistFilter::SetCreateEdgeListFor2DDatasets(bool val)
{
    faceFilter->SetCreateEdgeListFor2DDatasets(val);
}


// ****************************************************************************
//  Method: avtGhostZoneAndFacelistFilter::SetCreate3DCellNumbers
//
//  Purpose:
//      This is a Boolean that is passed directly to the facelist filter.  It
//      is used when trying to reconstruct mesh edges after the fact.
//
//  Programmer: Hank Childs
//  Creation:   July 19, 2002
//
// ****************************************************************************

void
avtGhostZoneAndFacelistFilter::SetCreate3DCellNumbers(bool val)
{
    faceFilter->SetCreate3DCellNumbers(val);
}


// ****************************************************************************
//  Method: avtGhostZoneAndFacelistFilter::SetForceFaceConsolidation
//
//  Purpose:
//      Used to tell the facelist filter to consolidate faces.
//
//  Programmer: Hank Childs
//  Creation:   October 15, 2003
//
// ****************************************************************************

void
avtGhostZoneAndFacelistFilter::SetForceFaceConsolidation(bool val)
{
    faceFilter->SetForceFaceConsolidation(val);
}


// ****************************************************************************
//  Method: avtGhostZoneAndFacelistFilter::GhostDataMustBeRemoved
//
//  Purpose:
//      Tell the ghost zone filter that ghost data must be removed.
//
//  Programmer: Hank Childs
//  Creation:   December 20, 2006
//
// ****************************************************************************

void
avtGhostZoneAndFacelistFilter::GhostDataMustBeRemoved(void)
{
    ghostFilter->GhostDataMustBeRemoved();
}


// ****************************************************************************
//  Method: avtGhostZoneAndFacelistFilter::MustCreatePolyData
//
//  Purpose:
//      Tell the facelist filter that it must produce poly data.
//
//  Programmer: Hank Childs
//  Creation:   December 20, 2006
//
// ****************************************************************************

void
avtGhostZoneAndFacelistFilter::MustCreatePolyData(void)
{
    faceFilter->MustCreatePolyData();
}


// ****************************************************************************
//  Method: avtGhostZoneAndFacelistFilter::Execute
//
//  Purpose:
//    Applies ghostZone and facelist filters to the input.  Which
//    comes first is determined by whether or not the input is using all data.
//    Facelist filter only applied if user specified that it needed to be used.
//
//  Programmer: Kathleen Bonnell
//  Creation:   July 20, 2001
//
//  Modifications:
//
//    Hank Childs, Fri Sep 14 09:58:14 PDT 2001
//    Made use of more specialized routine that focuses on whether all of the
//    domains are being used, rather than all of the data is being used.  This
//    allows internal faces to be removed when are using all of the domains,
//    but have employed material selection.
//
//    Eric Brugger, Mon Nov  5 13:40:36 PST 2001
//    Modified to always compile the timing code.
//
//    Brad Whitlock, Thu Apr 4 14:46:27 PST 2002
//    Changed CopyTo so it is an inline template function.
//
//    Hank Childs, Mon Sep 30 09:58:10 PDT 2002
//    Do not apply ghost filter if we have no ghost zones.
//
//    Sean Ahern, Thu Jan 23 11:55:37 PST 2003
//    Fixed a problem with hooking up the input and output if we have
//    nothing to do.
//
//    Hank Childs, Thu May 15 16:31:41 PDT 2003
//    Fixed a problem where under bizarre circumstances, the ghost zone filter
//    is applied when it should not be ['3352].
//
//    Mark C. Miller, Thu Oct 16 05:46:06 PDT 2003
//    Added condition that if ghost were CREATED, apply facelist filter first
//
//    Hank Childs, Wed Feb  2 08:56:00 PST 2005
//    Make sure we are using the right variable in the spec, since that will
//    affect what the active variable is.
//
// ****************************************************************************

void
avtGhostZoneAndFacelistFilter::Execute(void)
{
    int  timingIndex = visitTimer->StartTimer();

    avtDataObject_p dObj = GetInput();
    avtDataValidity   &v = dObj->GetInfo().GetValidity();

    // Make sure this is the latest info.  This changes under very bizarre
    // circumstances.  ['3352].
    avtDataAttributes &a = dObj->GetInfo().GetAttributes();
    useGhostFilter = (a.GetContainsGhostZones()!=AVT_NO_GHOSTS ? true : false);

    avtDataset_p ds; 
    CopyTo(ds, dObj);
    avtSourceFromAVTDataset termsrc(ds);
    avtDataObject_p data = termsrc.GetOutput(); 

    avtPipelineSpecification_p specForDB = GetGeneralPipelineSpecification();
    avtDataSpecification_p wrongVar = specForDB->GetDataSpecification();
    avtDataSpecification_p correctVar = new avtDataSpecification(wrongVar,
                                                             pipelineVariable);
    avtPipelineSpecification_p goodSpec = new avtPipelineSpecification(
                                                       specForDB, correctVar);
    
    if (useFaceFilter && !useGhostFilter)
    {
        debug5 << "Using facelist filter only." << endl;
        faceFilter->SetInput(data);
        faceFilter->Update(goodSpec);
        GetOutput()->Copy(*(faceFilter->GetOutput()));
    }
    else if (useGhostFilter && !useFaceFilter)
    {
        debug5 << "Using ghostzone filter only." << endl;
        ghostFilter->SetInput(data);
        ghostFilter->Update(goodSpec);
        GetOutput()->Copy(*(ghostFilter->GetOutput()));
    }
    else if (!useGhostFilter && !useFaceFilter)
    {
        debug5 << "Not applying ghost zone or facelist filter." << endl;
        GetOutput()->Copy(*dObj);
    }
    else
    {
        // if we are using all the data, apply the facelist filter first.
        bool faceFirst = v.GetUsingAllDomains() ||
                         (a.GetContainsGhostZones() == AVT_CREATED_GHOSTS);

        if (faceFirst)
        {
            debug5 << "Using facelist filter before ghostzone filter." << endl;
            faceFilter->SetInput(data);
            ghostFilter->SetInput(faceFilter->GetOutput());
            ghostFilter->Update(goodSpec);
            GetOutput()->Copy(*(ghostFilter->GetOutput()));
        }
        else
        {
            debug5 << "Using ghostzone filter before facelist filter." << endl;
            ghostFilter->SetInput(data);
            faceFilter->SetInput(ghostFilter->GetOutput());
            faceFilter->Update(goodSpec);
            GetOutput()->Copy(*(faceFilter->GetOutput()));
        }
    }

    visitTimer->StopTimer(timingIndex, "GhostZone And Facelist Filter");
    visitTimer->DumpTimings();
}


// ****************************************************************************
//  Method: avtGhostZoneAndFacelistFilter::RefashionDataObjectInfo
//
//  Purpose:
//      Copies the mutable metadata that is associated with individual 
//      datasets.  This is done by avtDatasetToDatasetFilter, but is redefined
//      here to indicate that the zones are invalidated after this operation.
//
//  Programmer: Hank Childs
//  Creation:   July 27, 2001
//
//  Modifications:
//
//    Hank Childs, Mon Jul 30 15:05:07 PDT 2001
//    Fixed up sloppiness from Friday - output dimension is 2 only if we are
//    using facelists.
//
//    Hank Childs, Tue Sep  4 15:12:40 PDT 2001
//    Reflect new data attributes interface.
//
//    Hank Childs, Sun Jun 23 13:06:44 PDT 2002
//    If we have something with topological dimension less than 2, then don't
//    say our output has dimension 2.
//
//    Hank Childs, Mon Sep 30 09:58:10 PDT 2002
//    Tell the output that it does not have ghost zones.
//
// ****************************************************************************

void
avtGhostZoneAndFacelistFilter::RefashionDataObjectInfo(void)
{
    avtDataObject_p output = GetOutput();
    output->GetInfo().GetValidity().InvalidateZones();
    if (useFaceFilter)
    {
        if (GetInput()->GetInfo().GetAttributes().GetTopologicalDimension() >2)
        {
            output->GetInfo().GetAttributes().SetTopologicalDimension(2);
        }
    }
    output->GetInfo().GetAttributes().SetContainsGhostZones(AVT_NO_GHOSTS);
}


// ****************************************************************************
//  Method: avtGhostZoneAndFacelistFilter::ReleaseData
//
//  Purpose:
//      Releases all the data -- it is not sufficient to rely on the definition
//      in the base class, since we must free up our derived types as well.
//
//  Programmer: Hank Childs
//  Creation:   September 10, 2002
//
//  Modifications:
//
//    Hank Childs, Sat Feb 19 14:55:03 PST 2005
//    Set the inputs of the filters as NULL, since we may not want to release
//    the data of their inputs.
//
// ****************************************************************************

void
avtGhostZoneAndFacelistFilter::ReleaseData(void)
{
    avtDatasetToDatasetFilter::ReleaseData();
    if (ghostFilter != NULL)
    {
        ghostFilter->ReleaseData();
        ghostFilter->SetInput(NULL);
    }
    if (faceFilter != NULL)
    {
        faceFilter->ReleaseData();
        faceFilter->SetInput(NULL);
    }
}


// ****************************************************************************
//  Method: avtGhostZoneAndFacelistFilter::ChangedInput
//
//  Purpose:
//      Called when the input to this filter is changed.  It will determine if
//      we should apply the ghost zone filter.
//
//  Programmer: Hank Childs
//  Creation:   September 30, 2002
//
// ****************************************************************************

void
avtGhostZoneAndFacelistFilter::ChangedInput(void)
{
    avtDatasetToDatasetFilter::ChangedInput();
    avtDataAttributes &a = GetInput()->GetInfo().GetAttributes();
    useGhostFilter = (a.GetContainsGhostZones()!=AVT_NO_GHOSTS ? true : false);
}


// ****************************************************************************
//  Method: avtGhostZoneAndFacelistFilter::PerformRestriction
//
//  Purpose:
//      Tell the database that we will need ghost nodes.
//
//  Programmer: Hank Childs
//  Creation:   August 11, 2004
//
// ****************************************************************************

avtPipelineSpecification_p
avtGhostZoneAndFacelistFilter::PerformRestriction(
                                            avtPipelineSpecification_p in_spec)
{
    avtPipelineSpecification_p spec = in_spec;

    //
    // Only declare that we want ghost nodes if someone downstream hasn't said
    // that they want ghost zones.  Also only declare this if we are actually
    // taking external faces.
    //
    if (useFaceFilter && 
          GetInput()->GetInfo().GetAttributes().GetTopologicalDimension() == 3)
    {
        spec = new avtPipelineSpecification(in_spec);
        if (spec->GetDataSpecification()->GetDesiredGhostDataType() !=
                                                               GHOST_ZONE_DATA)
            spec->GetDataSpecification()->SetDesiredGhostDataType(
                                                              GHOST_NODE_DATA);
    }

    return spec;
}

