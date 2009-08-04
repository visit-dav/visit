/*****************************************************************************
*
* Copyright (c) 2000 - 2009, Lawrence Livermore National Security, LLC
* Produced at the Lawrence Livermore National Laboratory
* LLNL-CODE-400124
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
//                       avtMultipleInputExpressionFilter.h                  //
// ************************************************************************* //

#include <avtMultipleInputExpressionFilter.h>

#include <string.h>

#include <avtDatasetExaminer.h>
#include <avtTypes.h>

#include <cstdlib>

// ****************************************************************************
//  Method: avtMultipleInputExpressionFilter destructor
//
//  Programmer: Hank Childs
//  Creation:   September 19, 2003
//
// ****************************************************************************

avtMultipleInputExpressionFilter::~avtMultipleInputExpressionFilter()
{
    for (int i = 0 ; i < varnames.size() ; i++)
    {
        //
        // We should use the C-style de-allocation, since this string was
        // created by strdup (which uses malloc).  If we don't, purify
        // complains.
        //
        free(varnames[i]);
    }
}


// ****************************************************************************
//  Method: avtMultipleInputExpressionFilter::AddInputVariableName
//
//  Purpose:
//      Sets one of the variables that will be needed for this filter.
//
//  Programmer: Sean Ahern
//  Creation:   June 14, 2002
//
// ****************************************************************************

void
avtMultipleInputExpressionFilter::AddInputVariableName(const char *var)
{
    if (varnames.size() == 0)
        SetActiveVariable(var);
    else
        AddSecondaryVariable(var);

    varnames.push_back(strdup(var));
}


// ****************************************************************************
//  Method: avtMultipleInputExpressionFilter::IsPointVariable
//
//  Purpose:
//      Determines whether or not this is a point variable.
//
//  Note:   This routine can only determine whether or not this is a point
//          variable once the data has been read.  However, this function
//          is sometimes called before that.  So the best we can do is defer
//          to the base class.  We need a better long term solution, but this
//          is better than what we are doing now.
//
//  Programmer: Hank Childs
//  Creation:   August 15, 2003
//
//  Modifications:
//
//    Hank Childs, Wed Feb 25 14:44:19 PST 2004
//    Updated to account for multiple variables in data attributes.
//
//    Kathleen Bonnell, Mon Mar 29 11:28:25 PST 2004 
//    Make sure that varnames is not empty before attempting to index. 
//
//    Hank Childs, Thu Jan  6 10:42:13 PST 2005
//    Do a better job when there are mixed centerings.
//
// ****************************************************************************

bool
avtMultipleInputExpressionFilter::IsPointVariable(void)
{
    avtDataAttributes &atts = GetInput()->GetInfo().GetAttributes();
    bool hasNodal = false;
    bool hasZonal = false;
    for (int i = 0 ; i < varnames.size() ; i++)
    {
        if (!atts.ValidVariable(varnames[i]))
            return avtExpressionFilter::IsPointVariable();

        if (atts.GetCentering(varnames[i]) == AVT_ZONECENT)
            hasZonal = true;
        else
            hasNodal = true;
    }

    if (hasZonal && hasNodal)
        return !(MixedCenteringYieldsZonal());

    return hasNodal;
}


// ****************************************************************************
//  Method: avtMultipleInputExpressionFilter::ClearInputVariableNames
//
//  Purpose:
//      Clears the input variable names.  This allows a filter to be used
//      repeatedly.
//
//  Programmer: Hank Childs
//  Creation:   February 4, 2004
//
// ****************************************************************************

void
avtMultipleInputExpressionFilter::ClearInputVariableNames(void)
{
    varnames.clear();
}


