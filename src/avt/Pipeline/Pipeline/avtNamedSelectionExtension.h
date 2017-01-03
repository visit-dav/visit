/*****************************************************************************
*
* Copyright (c) 2000 - 2017, Lawrence Livermore National Security, LLC
* Produced at the Lawrence Livermore National Laboratory
* LLNL-CODE-442911
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
