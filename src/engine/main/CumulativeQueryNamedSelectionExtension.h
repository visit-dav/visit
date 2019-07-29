// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#ifndef CUMULATIVE_QUERY_NAMED_SELECTION_EXTENSION_H
#define CUMULATIVE_QUERY_NAMED_SELECTION_EXTENSION_H

#include <avtNamedSelectionExtension.h>
#include <SelectionSummary.h>

// ****************************************************************************
// Class: CumulativeQueryNamedSelectionExtension
//
// Purpose:
//   This class implements cumulative query named selections as an extension
//   to named selections.
//
// Notes:      This class is implemented in the engine instead of in AVT because
//             putting it in AVT would cause some library tangling since this
//             class uses some avt filters under the hood.
//
// Programmer: Brad Whitlock
// Creation:   Mon Dec 13 15:56:56 PST 2010
//
// Modifications:
//   Brad Whitlock, Tue Sep  6 15:27:34 PDT 2011
//   I changed the API so we could add caching support that lets setting
//   query attributes sometimes bypass filter execution.
//
//   Brad Whitlock, Fri Oct 28 11:48:05 PDT 2011
//   Change the API.
//
// ****************************************************************************

class CumulativeQueryNamedSelectionExtension : public avtNamedSelectionExtension
{
public:
    CumulativeQueryNamedSelectionExtension();
    virtual ~CumulativeQueryNamedSelectionExtension();

    virtual avtNamedSelection *GetSelection(avtDataObject_p dob, 
                                            const SelectionProperties &props,
                                            avtNamedSelectionCache &cache);

    const SelectionSummary &GetSelectionSummary() const;
private:
    SelectionSummary summary;
};

#endif
