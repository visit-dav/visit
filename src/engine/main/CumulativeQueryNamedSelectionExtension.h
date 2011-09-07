/*****************************************************************************
*
* Copyright (c) 2000 - 2011, Lawrence Livermore National Security, LLC
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
#ifndef CUMULATIVE_QUERY_NAMED_SELECTION_EXTENSION_H
#define CUMULATIVE_QUERY_NAMED_SELECTION_EXTENSION_H

#include <avtNamedSelectionExtension.h>
#include <SelectionSummary.h>

class avtThresholdFilter;
class CQHistogramCalculationFilter;
class CQFilter;

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
// ****************************************************************************

class CumulativeQueryNamedSelectionExtension : public avtNamedSelectionExtension
{
public:
    CumulativeQueryNamedSelectionExtension();
    virtual ~CumulativeQueryNamedSelectionExtension();

    virtual void GetSelection(avtDataObject_p dob, const SelectionProperties &props,
                              avtNamedSelectionCache &cache, 
                              std::vector<int> &doms, std::vector<int> &zones);

    const SelectionSummary &GetSelectionSummary() const;
private:
    std::string      CreateSelectionKey(const SelectionProperties &props) const;
    bool             CheckProperties(const SelectionProperties &newProps, 
                                     const SelectionProperties &oldProps) const;
    avtDataObject_p  AddFilters(avtDataObject_p dob, const SelectionProperties &props);
    SelectionSummary BuildSummary();

    CQHistogramCalculationFilter *hist;
    CQFilter                     *cqFilter;
    avtThresholdFilter           *threshold;
    SelectionSummary              summary;
    int                           nts;
};

#endif
