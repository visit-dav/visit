// ************************************************************************* //
//                                  avtFilter.C                              //
// ************************************************************************* //

#include <avtFilter.h>

#include <avtDynamicAttribute.h>
#include <avtExtents.h>
#include <avtMetaData.h>
#include <avtParallel.h>
#include <avtTerminatingSource.h>

#include <DebugStream.h>
#include <ImproperUseException.h>
#include <NoInputException.h>


using std::string;


// ****************************************************************************
//  Method: avtFilter constructor
//
//  Programmer: Hank Childs
//  Creation:   May 30, 2001
//
// ****************************************************************************

avtFilter::avtFilter()
{
    modified    = true;
    inExecute   = false;
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
// ****************************************************************************

bool
avtFilter::Update(avtPipelineSpecification_p spec)
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

    //
    // By using meta-data, we can potentially reduce the amount of data that
    // we are actually interested in using.  Give the derived types an
    // opportunity to reduce the data we are interested in.
    //
    avtPipelineSpecification_p newSpec =
                                      PerformRestrictionAndDoBookkeeping(spec);

    bool modifiedUpstream = UpdateInput(newSpec);

    bool re_execute = modifiedUpstream || modified;
    if (re_execute)
    {
        inExecute = true;

        TRY
        {
            //
            // If extents have changed, we need to copy that downstream.
            //
            PassOnDataObjectInfo();
    
            debug1 << "Executing " << GetType() << endl;
            UpdateProgress(0, 0);
            ResolveDynamicAttributes();
            PreExecute();
            Execute();
            PostExecute();
            UpdateProgress(1, 0);
            debug1 << "Done executing " << GetType() << endl;
            modified = false;
        }
        CATCH2(VisItException, e)
        {
            debug1 << "Exception occurred in " << GetType() << endl;
            avtDataValidity &v = GetOutput()->GetInfo().GetValidity();
            v.ErrorOccurred();
            string tmp = e.GetMessage(); // Otherwise there is a const problem.
            v.SetErrorMessage(tmp);
        }
        ENDTRY

        inExecute = false;
    }

    debug1 << "Done Updating " << GetType() << endl;
    return re_execute;
}


// ****************************************************************************
//  Method: avtFilter::PerformRestrictionAndDoBookkeeping
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
// ****************************************************************************

avtPipelineSpecification_p
avtFilter::PerformRestrictionAndDoBookkeeping(avtPipelineSpecification_p spec)
{
    int   i;

    avtPipelineSpecification_p newspec = PerformRestriction(spec);
    newspec->AddFilter();
    int additionalFilters = AdditionalPipelineFilters();
    for (i = 0 ; i < additionalFilters ; i++)
    {
        newspec->AddFilter();
    }

    //
    // Some derived types need to examine a specification as it goes up.
    //
    ExamineSpecification(newspec);

    //
    // Allow our dynamic attributes to add any variables they may need, etc.
    //
    int nAttributes = dynamicAttributes.size();
    for (i = 0 ; i < nAttributes ; i++)
    {
        newspec = dynamicAttributes[i]->PerformRestriction(newspec);
    }

    return newspec;
}


// ****************************************************************************
//  Method: avtFilter::PerformRestriction
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
// ****************************************************************************

avtPipelineSpecification_p
avtFilter::PerformRestriction(avtPipelineSpecification_p spec)
{
    return spec;
}


// ****************************************************************************
//  Method: avtFilter::GetTerminatingSource
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

avtTerminatingSource *
avtFilter::GetTerminatingSource(void)
{
    avtDataObject_p input = GetInput();
    if (*input == NULL)
    {
        EXCEPTION0(NoInputException);
    }

    return input->GetTerminatingSource();
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

    RefashionDataObjectInfo();
}


// ****************************************************************************
//  Method: avtFilter::RefashionDataObjectInfo
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
avtFilter::RefashionDataObjectInfo(void)
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
    avtTerminatingSource *src = GetTerminatingSource();
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
//  Method: avtFilter::GetGeneralPipelineSpecification
//
//  Purpose:
//      Gets a pipeline that the load balancer knows not to muck with.
//
//  Programmer: Hank Childs
//  Creation:   June 6, 2001
//
// ****************************************************************************

avtPipelineSpecification_p
avtFilter::GetGeneralPipelineSpecification(void)
{
    return GetTerminatingSource()->GetGeneralPipelineSpecification();
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
// ****************************************************************************

bool
avtFilter::TryDataExtents(double *outexts, const char *varname)
{
    avtDataAttributes &atts = GetInput()->GetInfo().GetAttributes();

    if (varname != NULL && !(atts.GetVariableName() == varname))
    {
        // We are looking for the extents of a different variable.
        return false;
    }

    //
    // Our first preference is for the effective extents.
    //
    avtExtents *eff = atts.GetEffectiveDataExtents();
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
        avtExtents *tr = atts.GetTrueDataExtents();
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

    avtDataAttributes &atts = GetInput()->GetInfo().GetAttributes();
    if (varname == NULL || (atts.GetVariableName() == varname))
    {
        atts.GetCumulativeTrueDataExtents()->CopyTo(outexts);
    }
    else
    {
        SearchDataForDataExtents(outexts);
    }

    UnifyMinMax(outexts, atts.GetVariableDimension()*2);

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
// ****************************************************************************

void
avtFilter::GetSpatialExtents(double *outexts)
{
    if (TrySpatialExtents(outexts))
    {
         //
         // We had them lying around -- no parallel communication necessary.
         // 
         return;
    }

    avtDataAttributes &atts = GetInput()->GetInfo().GetAttributes();
    atts.GetCumulativeTrueSpatialExtents()->CopyTo(outexts);

    UnifyMinMax(outexts, atts.GetSpatialDimension()*2);

    //
    // We now have determined the true spatial extents, so we may as well set
    // them back.
    //
    GetOutput()->GetInfo().GetAttributes().GetTrueSpatialExtents()
                                                                ->Set(outexts);
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
// ****************************************************************************

void
avtFilter::GetCurrentDataExtents(double *outexts)
{
    if (TryCurrentDataExtents(outexts))
    {
         //
         // We had them lying around -- no parallel communication necessary.
         //
         return;
    }

    avtDataAttributes &atts = GetInput()->GetInfo().GetAttributes();
    atts.GetCumulativeCurrentDataExtents()->CopyTo(outexts);

    UnifyMinMax(outexts, atts.GetVariableDimension()*2);

    //
    // We now have determined the current data extents, so we may as well set
    // them back.
    //
    GetOutput()->GetInfo().GetAttributes().GetCurrentDataExtents()
                                                               ->Set(outexts);
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

    UnifyMinMax(outexts, atts.GetSpatialDimension()*2);

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
//  Method: avtFilter::ExamineSpecification
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
avtFilter::ExamineSpecification(avtPipelineSpecification_p)
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
// ****************************************************************************

void
avtFilter::ReleaseData(void)
{
    debug3 << "Filter " << GetType() << " releasing its data" << endl;
    GetOutput()->ReleaseData();
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
// ****************************************************************************

void
avtFilter::SearchDataForDataExtents(double *)
{
    EXCEPTION0(ImproperUseException);
}


