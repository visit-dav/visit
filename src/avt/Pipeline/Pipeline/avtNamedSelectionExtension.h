// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#ifndef AVT_NAMED_SELECTION_EXTENSION_H
#define AVT_NAMED_SELECTION_EXTENSION_H

#include <pipeline_exports.h>

#include <avtContract.h>
#include <avtDataset.h>
#include <avtDataObject.h>
#include <avtNamedSelection.h>
#include <MRUCache.h>
#include <SelectionProperties.h>

// Base class for things we can stick in the cache.
class PIPELINE_API avtNamedSelectionCacheItem
{
public:
    avtNamedSelectionCacheItem();
    virtual ~avtNamedSelectionCacheItem();

    SelectionProperties properties;
};

typedef MRUCache<std::string, 
                 avtNamedSelectionCacheItem *,
                 MRUCache_Delete, 
                 10> avtNamedSelectionCache;

// ****************************************************************************
// Class: avtNamedSelectionExtension
//
// Purpose:
//   This is a base class for objects that can perform additional setup when
//   creating named selections. The extra setup can be used to influence how
//   the selection is created.
//
// Notes:      
//
// Programmer: Brad Whitlock
// Creation:   Mon Dec 13 16:09:53 PST 2010
//
// Modifications:
//   Brad Whitlock, Tue Sep  6 14:47:35 PDT 2011
//   I changed the API.
//
//   Brad Whitlock, Thu Oct 27 16:56:42 PDT 2011
//   I changed the API so we return a named selection from the extension.
//
// ****************************************************************************

class PIPELINE_API avtNamedSelectionExtension
{
public:
    avtNamedSelectionExtension();
    virtual ~avtNamedSelectionExtension();

    virtual avtNamedSelection *GetSelection(avtDataObject_p dob, const SelectionProperties &props,
                                            avtNamedSelectionCache &cache);

    virtual avtContract_p ModifyContract(avtContract_p c0, const SelectionProperties &props,
                                         bool &needsUpdate) const;

    static std::string        GetIdVariable(const SelectionProperties &props);
    static avtNamedSelection *GetSelectionFromDataset(avtDataset_p tree, const SelectionProperties &props);
};

#endif
