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
//                                  avtFilter.C                              //
// ************************************************************************* //

#include <avtFilter.h>

#include <avtDynamicAttribute.h>
#include <avtExtents.h>
#include <avtMetaData.h>
#include <avtTerminatingSink.h>
#include <avtParallel.h>
#include <avtOriginatingSource.h>
#include <avtWebpage.h>

#include <vtkSystemIncludes.h> // for VTK_FLOAT

#include <DebugStream.h>
#include <ImproperUseException.h>
#include <NoInputException.h>
#include <TimingsManager.h>

#include <string>
#include <vector>

using std::string;
using std::vector;


bool avtFilter::debugDump    = false;
int  avtFilter::numInExecute = 0;


// ****************************************************************************
//  Method: avtFilter constructor
//
//  Programmer: Hank Childs
//  Creation:   May 30, 2001
//
//  Modifications:
//
//    Hank Childs, Fri Dec 22 07:55:21 PST 2006
//    Initialize webpage.
//
// ****************************************************************************

avtFilter::avtFilter()
{
    modified    = true;
    inExecute   = false;
    webpage     = NULL;
}


// ****************************************************************************
//  Method: avtFilter destructor
//
//  Purpose:
//      Defines the destructor.  Note: this should not be inlined in the header
//      because it causes problems for certain compilers.
//
//  Programmer: Hank Childs
//  Creation:   February 5, 2004
//
//  Modifications:
//
//    Hank Childs, Fri Dec 22 07:55:21 PST 2006
//    Delete the webpage.
//
// ****************************************************************************

avtFilter::~avtFilter()
{
    if (webpage != NULL)
        delete webpage;;
}


// ****************************************************************************
//  Method: avtFilter::UpdateProgress
//
//  Purpose:
//      Updates the progress of the filter through a callback.
//
//  Arguments:
//      current       The current node of a data tree that was just finished.
//      total         The total number of trees.
//
//  Notes:            This method is also used to indicate that a filter is
//                    totally done.  When that happens, the callback is called
//                    with total == 0.  Also, the name of description can be
//                    NULL.
//
//  Programmer:       Hank Childs
//  Creation:         June 20, 2001
//
// ****************************************************************************

void
avtFilter::UpdateProgress(int current, int total)
{
    avtDataObjectSource::UpdateProgress(current, total, GetType(),
                                        GetDescription());
}


// ****************************************************************************
//  Method: avtFilter::Update
//
//  Purpose: 
//      Walks up a pipeline and determine what needs to re-execute and what
//      part of the data specification can be reduced through the use of
//      meta-data.
//
//  Arguments:
//      spec    The pipeline specification.
//
//  Returns:    A Boolean indicating if anything upstream was modified.
//
//  Programmer: Hank Childs
//  Creation:   May 30, 2001
//
//  Modifications:
//
//    Hank Childs, Wed Jun 20 16:29:15 PDT 2001
//    Added hooks for aborting, and progress.
// 
//    Jeremy Meredith, Fri Jun 29 15:15:39 PDT 2001
//    Made two hooks for progress -- one at start and one at finish.
//
//    Hank Childs, Wed Sep 12 10:16:47 PDT 2001
//    Use UpdateInput over Update, since there might be multiple inputs and
//    we want to abstract that into the data object sink classes.
//
//    Jeremy Meredith, Wed Sep 19 13:58:14 PDT 2001
//    Removed an unnecessary CheckAbort since UpdateProgress now checks for it.
//
//    Hank Childs, Wed Oct 24 14:21:18 PDT 2001
//    Added calls to PreExecute and PostExecute.
//
//    Hank Childs, Fri Mar 15 15:30:11 PST 2002
//    Accounted for dynamic attributes.
//
//    Hank Childs, Fri May 16 10:37:09 PDT 2003
//    Do a better job of catching exceptions.
//
//    Hank Childs, Sun Feb 27 13:04:32 PST 2005
//    If we are doing dynamic load balancing, clean up our input as we go.
//    Moved timings code from avtDataTreeStreamer to this routine so that
//    all filters are timed.
//
//    Hank Childs, Thu May 25 16:45:41 PDT 2006
//    Release data for all transient data when we are done executing, 
//    regardless of whether or not we're streaming.
//
//    Hank Childs, Thu Dec 21 09:15:00 PST 2006
//    Add support for the "-dump" option.
//
//    Hank Childs, Fri Jun 15 11:37:59 PDT 2007
//    Maintain "numInExecute" to keep track of how many nested executions
//    we have.  Also dump info for pipeline specifications.
//
//    Hank Childs, Mon Dec 10 14:17:47 PST 2007
//    Added debug statement for when the filter decides *not* to execute.
//
// ****************************************************************************

bool
avtFilter::Update(avtContract_p spec)
{
    debug1 << "Entered update for " << GetType() << endl;

    CheckAbort();

    //
    // If we don't have an input, there isn't much we can do.
    //
    avtDataObject_p input = GetInput();
    if (*input == NULL)
    {
        EXCEPTION0(NoInputException);
    }

    if (debugDump)
    {
        InitializeWebpage();
        DumpContract(spec, "input");
    }

    //
    // By using meta-data, we can potentially reduce the amount of data that
    // we are actually interested in using.  Give the derived types an
    // opportunity to reduce the data we are interested in.
    //
    avtContract_p newSpec =
                                      ModifyContractAndDoBookkeeping(spec);

    if (debugDump)
        DumpContract(newSpec, "output");

    bool modifiedUpstream = UpdateInput(newSpec);

    bool re_execute = modifiedUpstream || modified;
    if (re_execute)
    {
        inExecute = true;

        TRY
        {
            int timerHandle = visitTimer->StartTimer();

            //
            // If extents have changed, we need to copy that downstream.
            //
            PassOnDataObjectInfo();
    
            debug1 << "Executing " << GetType() << endl;
            UpdateProgress(0, 0);
            ResolveDynamicAttributes();
            if (debugDump)
                DumpDataObject(GetInput(), "input");
            numInExecute++;
            PreExecute();
            Execute();
            PostExecute();
            if (debugDump)
                DumpDataObject(GetOutput(), "output");
            UpdateProgress(1, 0);
            debug1 << "Done executing " << GetType() << endl;
            modified = false;

            visitTimer->StopTimer(timerHandle, GetType());
            visitTimer->DumpTimings();
        }
        CATCH2(VisItException, e)
        {
            debug1 << "Exception occurred in " << GetType() << endl;
            avtDataValidity &v = GetOutput()->GetInfo().GetValidity();
            v.ErrorOccurred();
            string tmp = e.Message(); // Otherwise there is a const problem.
            v.SetErrorMessage(tmp);
        }
        ENDTRY

        if (debugDump)
            FinalizeWebpage();
        inExecute = false;
        numInExecute--;
    }
    else
    {
        debug1 << "Decided to *not* re-execute " << GetType() << endl;
    }

    //
    // If we are doing dynamic load balancing, clean up as we go.
    //
    if (GetInput()->GetInfo().GetValidity().GetIsThisDynamic() ||
        GetInput()->IsTransient())
    {
        if (GetInput()->GetSource() != NULL)
            GetInput()->GetSource()->ReleaseData();
    }

    debug1 << "Done Updating " << GetType() << endl;
    return re_execute;
}


// ****************************************************************************
//  Method: avtFilter::ModifyContractAndDoBookkeeping
//
//  Purpose:
//      Calls perform restriction and does some bookkeeping (like keeping
//      track of how many total filters there are in the pipeline).
//
//  Arguments:
//      spec     The current pipeline specification.
//
//  Returns:     The new pipeline specification.
//
//  Programmer:  Hank Childs
//  Creation:    June 20, 2001
//
//  Modifications:
//
//    Hank Childs, Mon Jul 30 15:41:06 PDT 2001
//    Allow for filters that have their own execute cycle with additional
//    filters.
//
//    Hank Childs, Wed Oct 24 14:21:18 PDT 2001
//    Added call to examine specification.
//
//    Hank Childs, Fri Mar 15 15:36:50 PST 2002
//    Added support for dynamic attributes.
//
//    Hank Childs, Fri Jun 15 17:21:24 PDT 2007
//    Have filters inspect the original specification, not the new one.
//
// ****************************************************************************

avtContract_p
avtFilter::ModifyContractAndDoBookkeeping(avtContract_p spec)
{
    int   i;

    //
    // Some derived types need to examine a specification as it goes up.
    //
    ExamineContract(spec);

    avtContract_p newspec = ModifyContract(spec);
    newspec->AddFilter();
    int additionalFilters = AdditionalPipelineFilters();
    for (i = 0 ; i < additionalFilters ; i++)
    {
        newspec->AddFilter();
    }

    //
    // Allow our dynamic attributes to add any variables they may need, etc.
    //
    int nAttributes = dynamicAttributes.size();
    for (i = 0 ; i < nAttributes ; i++)
    {
        newspec = dynamicAttributes[i]->ModifyContract(newspec);
    }

    return newspec;
}


// ****************************************************************************
//  Method: avtFilter::ModifyContract
//
//  Purpose:
//      Meta-data can be used to reduce the amount of data that needs to be
//      considered.  This is the mechanism to reduce that data as an Update
//      goes up a pipeline.  This is defined in the base type so derived
//      types don't need to if the mechanism does not apply.
//
//  Arguments:
//      spec     The current pipeline specification.
//
//  Returns:     The new pipeline specification.
//
//  Programmer:  Hank Childs
//  Creation:    May 31, 2001
//
//  Modifications:
//
//    Mark C. Miller, Tue Apr  5 10:30:16 PDT 2005
//    Added code to set admissible types to float
//
// ****************************************************************************

avtContract_p
avtFilter::ModifyContract(avtContract_p spec)
{
    //
    // By default, all filters admit only VTK_FLOAT data
    //
    vector<int> dataTypes;
    dataTypes.push_back(VTK_FLOAT);
    spec->GetDataRequest()->UpdateAdmissibleDataTypes(dataTypes);

    return spec;
}


// ****************************************************************************
//  Method: avtFilter::GetOriginatingSource
//
//  Purpose:
//      Gets the terminating source at the top of the pipeline.
//
//  Returns:   A terminating source.  Throws an exception over returning NULL.
//             (so return value is always valid).
//
//  Programmer: Hank Childs
//  Creation:   May 30, 2001
//
// ****************************************************************************

avtOriginatingSource *
avtFilter::GetOriginatingSource(void)
{
    avtDataObject_p input = GetInput();
    if (*input == NULL)
    {
        EXCEPTION0(NoInputException);
    }

    return input->GetOriginatingSource();
}


// ****************************************************************************
//  Method: avtFilter::GetQueryableSource
//
//  Purpose:
//      Gets the queryable source for the pipeline.
//
//  Returns:    A queryable source.  Throws an exception over returning NULL.
//              (so return value is always valid).
//
//  Programmer: Hank Childs
//  Creation:   July 28, 2003
//
// ****************************************************************************

avtQueryableSource *
avtFilter::GetQueryableSource(void)
{
    avtDataObject_p input = GetInput();
    if (*input == NULL)
    {
        EXCEPTION0(NoInputException);
    }

    return input->GetQueryableSource();
}


// ****************************************************************************
//  Method: avtFilter::ChangedInput
//
//  Purpose:
//      Catches the hook from the base class that the input has changed.
//
//  Programmer: Hank Childs
//  Creation:   May 30, 2001
//
// ****************************************************************************

void
avtFilter::ChangedInput(void)
{
    //
    // Give the derived types an opportunity to throw an exception if they
    // don't like the input.
    //
    VerifyInput();

    PassOnDataObjectInfo();

    //
    // Some filters may want to do some special things to initialize
    // themselves.  Give them that chance.
    //
    InitializeFilter();

    modified = true;
}


// ****************************************************************************
//  Method: avtFilter::InitializeFilter
//
//  Purpose:
//      This is a chance for the derived types to initialize themselves when
//      a new input is provided.
//
//  Programmer: Hank Childs
//  Creation:   May 31, 2001
//
// ****************************************************************************

void
avtFilter::InitializeFilter(void)
{
    ;
}


// ****************************************************************************
//  Method: avtFilter::VerifyInput
//
//  Purpose:
//      This is a chance for the derived types to verify a new input.  This
//      does not include type checking, which should be done already.  An
//      example of a filter that may want to use this mechanism would be an
//      index select, which would want to confirm that the input is curvilinear
//      or rectilinear.
//
//  Programmer: Hank Childs
//  Creation:   May 31, 2001
//
// ****************************************************************************

void
avtFilter::VerifyInput(void)
{
    ;
}


// ****************************************************************************
//  Method: avtFilter::PassOnDataObjectInfo
//
//  Purpose:
//      Passes on the data object information from the input to the output.
//      This only copies that information and then calls a virtual function
//      that will change information for the derived type in an appropriate
//      way (maybe none).
//
//  Programmer: Hank Childs
//  Creation:   May 31, 2001
//
//  Modifications:
//
//    Hank Childs, Tue Sep 10 09:08:51 PDT 2002
//    Allow for NULL inputs.
//
// ****************************************************************************

void
avtFilter::PassOnDataObjectInfo(void)
{
    avtDataObject_p output = GetOutput();
    avtDataObject_p input  = GetInput();
    if (*input != NULL)
    {
        output->GetInfo().Copy(input->GetInfo());
    }

    UpdateDataObjectInfo();
}


// ****************************************************************************
//  Method: avtFilter::UpdateDataObjectInfo
//
//  Purpose:
//      If the derived type of filter modifies the data object information,
//      then this is the method where that can be indicated.  This is defined
//      for the derived types that don't refashion the data object information.
//
//  Programmer: Hank Childs
//  Creation:   May 31, 2001
//
// ****************************************************************************

void
avtFilter::UpdateDataObjectInfo(void)
{
    ;
}


// ****************************************************************************
//  Method: avtFilter::GetMetaData
//
//  Purpose:
//      Gets the meta data object (and does some error checking as well).
//
//  Programmer: Hank Childs
//  Creation:   June 6, 2001
//
// ****************************************************************************

avtMetaData *
avtFilter::GetMetaData(void)
{
    avtOriginatingSource *src = GetOriginatingSource();
    if (src == NULL)
    {
        EXCEPTION0(NoInputException);
    }
    avtMetaData *md = src->GetMetaData();
    if (md == NULL)
    {
        EXCEPTION0(NoInputException);
    }

    return md;
}


// ****************************************************************************
//  Method: avtFilter::GetGeneralContract
//
//  Purpose:
//      Gets a pipeline that the load balancer knows not to muck with.
//
//  Programmer: Hank Childs
//  Creation:   June 6, 2001
//
// ****************************************************************************

avtContract_p
avtFilter::GetGeneralContract(void)
{
    return GetOriginatingSource()->GetGeneralContract();
}


// ****************************************************************************
//  Method: avtFilter::AdditionalPipelineFilters
//
//  Purpose:
//      Some filters create their own network with additional filters.  These
//      filters all make progress calls, so it needs to be reported that they
//      exist, so the progress is still meaningful.  Since most filters do
//      not create their own networks, this just returns 0.
//
//  Returns:     The number of additional filters used in a pipeline that is 
//               created in this filter's Execute (typically 0).
//
//  Programmer:  Hank Childs
//  Creation:    July 30, 2001
//
// ****************************************************************************

int
avtFilter::AdditionalPipelineFilters(void)
{
    return 0;
}


// ****************************************************************************
//  Method: avtFilter::TryDataExtents
//
//  Purpose:
//      Tries to get the data extents, but only if they are available (does
//      not resort to parallel unification).
//
//  Programmer: Hank Childs
//  Creation:   September 4, 2001
//
//  Modifications:
//
//    Hank Childs, Fri Dec 21 07:48:40 PST 2001
//    Do not use extents if they don't correspond to this variable.
//
//    Hank Childs, Wed Apr 17 09:33:15 PDT 2002
//    Made argument const.
//
//    Hank Childs, Thu Feb 26 09:14:49 PST 2004
//    Better account for multiple variables.
//
// ****************************************************************************

bool
avtFilter::TryDataExtents(double *outexts, const char *varname)
{
    avtDataAttributes &atts = GetInput()->GetInfo().GetAttributes();

    if (varname != NULL && !atts.ValidVariable(varname))
    {
        // We are looking for the extents of a different variable.
        return false;
    }
    if (varname == NULL && !atts.ValidActiveVariable())
    {
        return false;
    }

    //
    // Our first preference is for the effective extents.
    //
    avtExtents *eff = atts.GetEffectiveDataExtents(varname);
    if (eff->HasExtents())
    {
        eff->CopyTo(outexts);
        return true;
    }
    else
    {
        //
        // If we had the extents in the meta-data, use that.
        //
        avtExtents *tr = atts.GetTrueDataExtents(varname);
        if (tr->HasExtents())
        {
            tr->CopyTo(outexts);
            return true;
        }
    }

    return false;
}


// ****************************************************************************
//  Method: avtFilter::GetDataExtents
//
//  Purpose:
//      Gets the data extents -- parallel unification or not.
//
//  Programmer: Hank Childs
//  Creation:   September 4, 2001
//
//  Modifications:
//
//    Hank Childs, Fri Dec 21 07:48:40 PST 2001
//    Do not use extents if they don't correspond to this variable.
//
//    Hank Childs, Wed Apr 17 09:33:15 PDT 2002
//    Made argument const.
//
//    Hank Childs, Wed Jul  9 11:48:22 PDT 2003
//    Use the prescribed extents if the variable is named and that name is
//    the name of the default variable.
//
//    Mark C. Miller, Sat Jan 31 13:31:08 PST 2004
//    Added optional size of 18 for UnifyMinMax (max # comps = 9 for tensor)
//
//    Kathleen Bonnell, Thu Mar 11 10:14:20 PST 2004
//    DataExtents now restricted to only 2 components, regardless of variable
//    dimension. 
//
//    Hank Childs, Tue Jun 29 07:16:23 PDT 2004
//    Only use valid extents.
//
//    Hank Childs, Fri Dec  3 14:25:56 PST 2004
//    Add variable name to SearchDataForDataExtents.
//
//    Hank Childs, Sun Jan 30 14:25:00 PST 2005
//    Be leery of case where there is no input variable.
//
// ****************************************************************************

void
avtFilter::GetDataExtents(double *outexts, const char *varname)
{
    if (TryDataExtents(outexts, varname))
    {
         //
         // We had them lying around -- no parallel communication necessary.
         // 
         return;
    }

    bool hadThemAlready = false;
    avtDataAttributes &atts = GetInput()->GetInfo().GetAttributes();
    if (varname == NULL || 
        (atts.ValidActiveVariable() && atts.GetVariableName() == varname))
    {
        avtExtents *exts = atts.GetCumulativeTrueDataExtents();
        if (exts->HasExtents())
        {
            atts.GetCumulativeTrueDataExtents()->CopyTo(outexts);
            hadThemAlready = true;
        }
    }

    if (!hadThemAlready)
    {
        SearchDataForDataExtents(outexts, varname);
    }

    UnifyMinMax(outexts, 2, 2);

    if (varname == NULL)
    {
        //
        // We now have determined the true spatial extents, so we may as well 
        // set them back.
        //
        GetOutput()->GetInfo().GetAttributes().GetTrueDataExtents()
                                                                ->Set(outexts);
    }
}


// ****************************************************************************
//  Method: avtFilter::TrySpatialExtents
//
//  Purpose:
//      Tries to get the spatial extents, but only if they are available (does
//      not resort to parallel unification).
//
//  Programmer: Hank Childs
//  Creation:   September 4, 2001
//
// ****************************************************************************

bool
avtFilter::TrySpatialExtents(double *outexts)
{
    avtDataAttributes &atts = GetInput()->GetInfo().GetAttributes();

    //
    // Our first preference is for the effective extents.
    //
    avtExtents *eff = atts.GetEffectiveSpatialExtents();
    if (eff->HasExtents())
    {
        eff->CopyTo(outexts);
        return true;
    }
    else
    {
        //
        // If we had the extents in the meta-spatial, use that.
        //
        avtExtents *tr = atts.GetTrueSpatialExtents();
        if (tr->HasExtents())
        {
            tr->CopyTo(outexts);
            return true;
        }
    }

    return false;
}


// ****************************************************************************
//  Method: avtFilter::GetSpatialExtents
//
//  Purpose:
//      Gets the spatial extents -- parallel unification or not.
//
//  Programmer: Hank Childs
//  Creation:   September 4, 2001
//
//  Modifications:
//
//    Hank Childs, Mon Nov  3 09:49:11 PST 2003
//    Setting the extents with the output is a mistake -- the filter may even
//    modify those extents.  They should be set with the input.
//
//    Mark C. Miller, Sat Jan 31 13:31:08 PST 2004
//    Added altsize argument of 6 to UnifyMinMax call
// ****************************************************************************

void
avtFilter::GetSpatialExtents(double *newexts)
{
    if (TrySpatialExtents(newexts))
    {
         //
         // We had them lying around -- no parallel communication necessary.
         // 
         return;
    }

    avtDataAttributes &atts = GetInput()->GetInfo().GetAttributes();
    atts.GetCumulativeTrueSpatialExtents()->CopyTo(newexts);

    UnifyMinMax(newexts, atts.GetSpatialDimension()*2, 6);

    //
    // We now have determined the true spatial extents, so we may as well set
    // them back.
    //
    atts.GetTrueSpatialExtents()->Set(newexts);
}


// ****************************************************************************
//  Method: avtFilter::TryCurrentDataExtents
//
//  Purpose:
//      Tries to get the current data extents, but only if they are available
//      (does not resort to parallel unification).
//
//  Returns:    True if the extents were succesfully retrieved.
//
//  Arguments:
//    outexts   A place to hold the retrieved extents.
//
//
//  Programmer: Kathleen Bonnell
//  Creation:   October 2, 2001
//
// ****************************************************************************

bool
avtFilter::TryCurrentDataExtents(double *outexts)
{
    avtDataAttributes &atts = GetInput()->GetInfo().GetAttributes();

    avtExtents *current = atts.GetCurrentDataExtents();
    if (current->HasExtents())
    {
        current->CopyTo(outexts);
        return true;
    }

    return false;
}


// ****************************************************************************
//  Method: avtFilter::GetCurrentDataExtents
//
//  Purpose:
//      Gets the data extents -- parallel unification or not.
//
//  Arguments:
//    outexts   A place to hold the retrieved extents.
//
//  Programmer: Kathleen Bonnell 
//  Creation:   October 2, 2001
//
//  Modifications:
//
//    Hank Childs, Mon Nov  3 09:49:11 PST 2003
//    Setting the extents with the output is a mistake -- the filter may even
//    modify those extents.  They should be set with the input.
//
//    Mark C. Miller, Sat Jan 31 13:31:08 PST 2004
//    Added optional size of 18 for UnifyMinMax (max # comps = 9 for tensor)
//
//    Kathleen Bonnell, Thu Mar 11 10:14:20 PST 2004
//    DataExtents now restricted to only 2 components, regardless of variable
//    dimension.
// 
// ****************************************************************************

void
avtFilter::GetCurrentDataExtents(double *newexts)
{
    if (TryCurrentDataExtents(newexts))
    {
         //
         // We had them lying around -- no parallel communication necessary.
         //
         return;
    }

    avtDataAttributes &atts = GetInput()->GetInfo().GetAttributes();
    atts.GetCumulativeCurrentDataExtents()->CopyTo(newexts);

    UnifyMinMax(newexts, 2, 2);

    //
    // We now have determined the current data extents, so we may as well set
    // them back.
    //
    atts.GetCurrentDataExtents()->Set(newexts);
}


// ****************************************************************************
//  Method: avtFilter::TryCurrentSpatialExtents
//
//  Purpose:
//    Tries to get the currentspatial extents, but only if they are available
//    (does not resort to parallel unification).
//
//  Returns:    True if the extents were succesfully retrieved.
//
//  Arguments:
//    outexts   A place to hold the retrieved extents.
//
//  Programmer: Kathleen Bonnell
//  Creation:   October 2, 2001
//
// ****************************************************************************

bool
avtFilter::TryCurrentSpatialExtents(double *outexts)
{
    avtDataAttributes &atts = GetInput()->GetInfo().GetAttributes();

    //
    // Our first preference is for the effective extents.
    //
    avtExtents *current = atts.GetCurrentSpatialExtents();
    if (current->HasExtents())
    {
        current->CopyTo(outexts);
        return true;
    }

    return false;
}


// ****************************************************************************
//  Method: avtFilter::GetCurrentSpatialExtents
//
//  Purpose:
//    Gets the current spatial extents -- parallel unification or not.
//
//  Arguments:
//    outexts   A place to hold the retrieved extents.
//
//  Programmer: Kathleen Bonnell
//  Creation:   October 2, 2001
//
//  Modificaitons:
//
//    Mark C. Miller, Sat Jan 31 13:31:08 PST 2004
//    Added altsize argument of 6 to UnifyMinMax call
//
// ****************************************************************************

void
avtFilter::GetCurrentSpatialExtents(double *outexts)
{
    if (TryCurrentSpatialExtents(outexts))
    {
         //
         // We had them lying around -- no parallel communication necessary.
         //
         return;
    }

    avtDataAttributes &atts = GetInput()->GetInfo().GetAttributes();
    atts.GetCumulativeCurrentSpatialExtents()->CopyTo(outexts);

    UnifyMinMax(outexts, atts.GetSpatialDimension()*2, 6);

    //
    // We now have determined the true spatial extents, so we may as well set
    // them back.
    //
    GetOutput()->GetInfo().GetAttributes().GetCurrentSpatialExtents()
                                                                ->Set(outexts);
}


// ****************************************************************************
//  Method: avtFilter::PreExecute
//
//  Purpose:
//      Called just before execution.  This is a hook for derived types.
//
//  Programmer: Hank Childs
//  Creation:   October 24, 2001
//
// ****************************************************************************

void
avtFilter::PreExecute(void)
{
    ;
}


// ****************************************************************************
//  Method: avtFilter::PostExecute
//
//  Purpose:
//      Called just after execution.  This is a hook for derived types.
//
//  Programmer: Hank Childs
//  Creation:   October 24, 2001
//
// ****************************************************************************

void
avtFilter::PostExecute(void)
{
    ;
}


// ****************************************************************************
//  Method: avtFilter::ExamineContract
//
//  Purpose:
//      Allows the base types an opportunity to examine the pipeline
//      specification.  This is defined as a hook for derived types.
//
//  Programmer: Hank Childs
//  Creation:   October 24, 2001
//
// ****************************************************************************

void
avtFilter::ExamineContract(avtContract_p)
{
    ;
}


// ****************************************************************************
//  Method: avtFilter::ReleaseData
//
//  Purpose:
//      Makes the output release any data that it has as a memory savings.
//
//  Programmer: Hank Childs
//  Creation:   October 24, 2001
//
//  Modifications:
//
//    Hank Childs, Sat Feb 19 14:40:58 PST 2005
//    Release data is now defined in the base class.  Just call the base
//    class's method.  Left this one in place so that the debug statement
//    would stick around (very useful for debugging).
//
//    Hank Childs, Wed Jan 17 11:14:17 PST 2007
//    Tell the filter that it has been modified when it releases its data.
//
// ****************************************************************************

void
avtFilter::ReleaseData(void)
{
    debug3 << "Filter " << GetType() << " releasing its data" << endl;
    avtDataObjectSource::ReleaseData();
    modified = true;
}


// ****************************************************************************
//  Method: avtFilter::ResolveDynamicAttributes
//
//  Purpose:
//      Tells each of the dynamic attributes that it is time to resolve
//      themselves.
//
//  Programmer: Hank Childs
//  Creation:   March 15, 2002
//
// ****************************************************************************

void
avtFilter::ResolveDynamicAttributes(void)
{
    int nAttributes = dynamicAttributes.size();
    for (int i = 0 ; i < nAttributes ; i++)
    {
        dynamicAttributes[i]->ResolveAttributes(GetInput());
    }
}


// ****************************************************************************
//  Method: avtFilter::RegisterDynamicAttributes
//
//  Purpose:
//      Registers a dynamic attributes object with this base type.
//
//  Programmer: Hank Childs
//  Creation:   March 15, 2002
//
// ****************************************************************************

void
avtFilter::RegisterDynamicAttribute(avtDynamicAttribute *da)
{
    dynamicAttributes.push_back(da);
}


// ****************************************************************************
//  Method: avtFilter::SearchDataForDataExtents
//
//  Purpose:
//      This should search through the actual data to find the data extents.
//      Since this class does not know what type of data it has, it will
//      throw an exception.
//
//  Programmer: Hank Childs
//  Creation:   April 16, 2002
//
//  Modifications:
//
//    Hank Childs, Fri Dec  3 14:28:02 PST 2004
//    Added variable name.
//
// ****************************************************************************

void
avtFilter::SearchDataForDataExtents(double *, const char *)
{
    EXCEPTION0(ImproperUseException);
}


// ****************************************************************************
//  Method: avtFilter::InitializeWebpage
//
//  Purpose:
//      Initializes the webpage.
//
//  Programmer: Hank Childs
//  Creation:   June 15, 2007
//
// ****************************************************************************

void
avtFilter::InitializeWebpage(void)
{
    if (webpage != NULL)
    {
        debug1 << "DUMP CODE: open file handle, exception previously?" 
               << endl;
        delete webpage;
    }

    static int filter_id = 0;
    char name[128];
    if (PAR_Size() > 1)
    {
        int rank = PAR_Rank();
        sprintf(name, "filt%d.%d.html", filter_id, rank);
    }
    else
        sprintf(name, "filt%d.html", filter_id);
    filter_id++;
    webpage = new avtWebpage(name);

    // Now set up our webpage.
    char pagename[128];
    sprintf(pagename, "%s dump info", GetType());
    webpage->InitializePage(pagename);
    webpage->WriteTitle(pagename);
    webpage->AddOnPageLink("input_contract", "Input contract");
    webpage->AddOnPageLink("output_contract", "Output contract");
    webpage->AddOnPageLink("input_data_object", "Input data object");
    webpage->AddOnPageLink("output_data_object", "Output data object");
}


// ****************************************************************************
//  Method: avtFilter::FinalizeWebpage
//
//  Purpose:
//      Finalizes the webpage.
//
//  Programmer: Hank Childs
//  Creation:   June 15, 2007
//
// ****************************************************************************

void
avtFilter::FinalizeWebpage(void)
{
    if (webpage == NULL)
    {
        debug1 << "DUMP CODE: webpage not open, error?" << endl;
        return;
    }
 
    webpage->FinalizePage();
    delete webpage;
    webpage = NULL;
}


// ****************************************************************************
//  Method: avtFilter::DumpDataObject
//
//  Purpose:
//      "Dumps" a data object.  This means put the data attributes into a web 
//      page.  The data object also can add to the web page, depending on 
//      whether or not it has reimplemented a virtual function on how to 
//      write itself out.
//
//  Programmer: Hank Childs
//  Creation:   December 21, 2006
//
//  Modifications:
//
//    Hank Childs, Fri Jun 15 12:41:41 PDT 2007
//    Moved some code to Initialize and FinalizeWebpage.
//
// ****************************************************************************

void
avtFilter::DumpDataObject(avtDataObject_p dob, const char *prefix)
{
    if (webpage == NULL)
    {
        debug1 << "Webpage not initialized ... shouldn't happen" << endl;
        return;
    }

    if (strcmp(prefix, "input") == 0)
    {
        // If we add the reference to the main web page right now, 
        // all of the filters will be in execution order.
        avtTerminatingSink::AddDumpReference(webpage->GetName(), GetType(), 
                                             numInExecute);

        std::string input_string;
        char prefix[128];
        sprintf(prefix, "before_%s", GetType());
        webpage->AddSectionForLinks("input_data_object");
        webpage->AddHeading("INPUT DATA OBJECT");
        dob->DebugDump(webpage, prefix);
    }
    else if (strcmp(prefix, "output") == 0)
    {
 
        std::string output_string;
        char prefix[128];
        sprintf(prefix, "after_%s", GetType());
        webpage->AddSectionForLinks("output_data_object");
        webpage->AddHeading("OUTPUT DATA OBJECT");
        dob->DebugDump(webpage, prefix);
    }
}


// ****************************************************************************
//  Method: avtFilter::DumpContract
//
//  Purpose:
//      "Dumps" a contract.  This means create a web page and put the
//      information about a pipeline specification into it.
//
//  Programmer: Hank Childs
//  Creation:   June 15, 2007
//
// ****************************************************************************

void
avtFilter::DumpContract(avtContract_p spec, const char *prefix)
{
    if (webpage == NULL)
    {
        debug1 << "Webpage not initialized ... shouldn't happen" << endl;
        return;
    }

    if (strcmp(prefix, "input") == 0)
    {
        webpage->AddSectionForLinks("input_contract");
        webpage->AddHeading("INPUT CONTRACT");
    }
    else if (strcmp(prefix, "output") == 0)
    {
        webpage->AddSectionForLinks("output_contract");
        webpage->AddHeading("OUTPUT CONTRACT");
    }
    spec->DebugDump(webpage);
}


// ****************************************************************************
//  Method:  avtFilter::FilterUnderstandsTransformedRectMesh
//
//  Purpose:
//    If this filter returns true, this means that it correctly deals
//    with rectilinear grids having an implied transform set in the
//    data attributes.  It can do this conditionally if desired.
//
//  Arguments:
//    none
//
//  Programmer:  Jeremy Meredith
//  Creation:    February 15, 2007
//
// ****************************************************************************
bool
avtFilter::FilterUnderstandsTransformedRectMesh(void)
{
    // This is the base class default, which reports "false".  This is
    // the safest behavior, allowing us to optimize those classes
    // that need it, and report "true" only for those known to exhibit
    // truly correct behavior.
    return false;
}
