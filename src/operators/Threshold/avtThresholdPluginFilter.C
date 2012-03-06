/*****************************************************************************
*
* Copyright (c) 2000 - 2012, Lawrence Livermore National Security, LLC
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
