// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//  File: avtThresholdPluginFilter.C
// ************************************************************************* //

#include <avtThresholdPluginFilter.h>

#include <avtThresholdFilter.h>


// ****************************************************************************
//  Method: avtThresholdPluginFilter constructor
//
//  Programmer: Brad Whitlock
//  Creation:   Fri Dec 10 16:30:57 PST 2010
//
// ****************************************************************************

avtThresholdPluginFilter::avtThresholdPluginFilter()
{
    tf = NULL;
}


// ****************************************************************************
//  Method: avtThresholdPluginFilter destructor
//
//  Programmer: Brad Whitlock
//  Creation:   Fri Dec 10 16:30:57 PST 2010
//
//  Modifications:
//
// ****************************************************************************

avtThresholdPluginFilter::~avtThresholdPluginFilter()
{
    if (tf != NULL)
    {
        delete tf;
        tf = NULL;
    }
}


// ****************************************************************************
//  Method:  avtThresholdPluginFilter::Create
//
//  Programmer: Brad Whitlock
//  Creation:   Fri Dec 10 16:30:57 PST 2010
//
// ****************************************************************************

avtFilter *
avtThresholdPluginFilter::Create()
{
    return new avtThresholdPluginFilter();
}


// ****************************************************************************
//  Method:      avtThresholdPluginFilter::SetAtts
//
//  Purpose:
//      Sets the state of the filter based on the attribute object.
//
//  Arguments:
//      a        The attributes to use.
//
//  Programmer: Brad Whitlock
//  Creation:   Fri Dec 10 16:30:57 PST 2010
//
// ****************************************************************************

void
avtThresholdPluginFilter::SetAtts(const AttributeGroup *a)
{
    atts = *(const ThresholdAttributes*)a;

    //
    // Construct a new threshold filter based on these attributes.
    //
    if (tf != NULL)
    {
        delete tf;
    }

    tf = new avtThresholdFilter();
    tf->SetAtts(a);
    tf->SetUpdateDataObjectInfoCallback(UpdateDataObjectInfoCB, (void*)this);
}


// ****************************************************************************
//  Method: avtThresholdPluginFilter::Equivalent
//
//  Purpose:
//      Returns true if creating a new avtThresholdPluginFilter with the given
//      parameters would result in an equivalent avtThresholdPluginFilter.
//
//  Programmer: Brad Whitlock
//  Creation:   Fri Dec 10 16:30:57 PST 2010
//
// ****************************************************************************

bool
avtThresholdPluginFilter::Equivalent(const AttributeGroup *a)
{
    return (atts == *(ThresholdAttributes*)a);
}

// ****************************************************************************
//  Method: avtThresholdPluginFilter::GetFacadedFilter
//
//  Purpose:
//      Gets the filter that the filter that we are a facade of.
//
//  Programmer: Hank Childs
//  Creation:   April 16, 2002
//
// ****************************************************************************

avtFilter *
avtThresholdPluginFilter::GetFacadedFilter(void)
{
    return tf;
}

// ****************************************************************************
//  Method: avtThresholdPluginFilter::GetFacadedFilter
//
//  Purpose:
//      Gets the filter that the filter that we are a facade of.
//
//  Programmer: Tom Fogal
//  Creation:   June 23, 2009
//
// ****************************************************************************

const avtFilter *
avtThresholdPluginFilter::GetFacadedFilter(void) const
{
    return tf;
}

// ****************************************************************************
// Method: avtThresholdPluginFilter::UpdateDataObjectInfoCB
//
// Purpose:
//   Update the data object information via a callback function.
//
// Programmer: Brad Whitlock
// Creation:   Tue Mar 18 10:53:05 PDT 2014
//
// Modifications:
//    Brad Whitlock, Mon Apr  7 15:55:02 PDT 2014
//    Add filter metadata used in export.
//    Work partially supported by DOE Grant SC0007548.
//
// ****************************************************************************

void
avtThresholdPluginFilter::UpdateDataObjectInfoCB(avtDataObject_p &input,
    avtDataObject_p &output, void *This)
{
    avtDataAttributes &outAtts = output->GetInfo().GetAttributes();
    const ThresholdAttributes &t = ((const avtThresholdPluginFilter *)This)->atts;

    char tmp[200];
    std::string params;
    size_t nvars = t.GetListedVarNames().size();
    for(size_t i = 0; i < nvars; ++i)
    {
        if(i < t.GetLowerBounds().size())
        {
            snprintf(tmp, 200, "%lg < ", t.GetLowerBounds()[i]);
            params += tmp;
        }

        if(i < t.GetLowerBounds().size() || i < t.GetUpperBounds().size())
        {
            if(t.GetListedVarNames()[i] == "default")
                params += t.GetDefaultVarName();
            else
                params += t.GetListedVarNames()[i];
        }

        if(i < t.GetUpperBounds().size())
        {
            snprintf(tmp, 200, " < %lg", t.GetUpperBounds()[i]);
            params += tmp;
        }

        if(i < nvars-1)
            params += ", ";
    }
    outAtts.AddFilterMetaData("Threshold", params);
}
