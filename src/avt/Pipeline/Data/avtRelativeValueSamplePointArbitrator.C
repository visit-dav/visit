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
//                   avtRelativeValueSamplePointArbitrator.C                 //
// ************************************************************************* //

#include <avtRelativeValueSamplePointArbitrator.h>


// ****************************************************************************
//  Method: avtRelativeValueSamplePointArbitrator constructor
//
//  Arguments:
//      om      The opacity map.
//      av      The arbitration variable.
//
//  Programmer: Hank Childs
//  Creation:   January 29, 2005
//
// ****************************************************************************

avtRelativeValueSamplePointArbitrator::avtRelativeValueSamplePointArbitrator(
                                                     bool lt, int av)
   : avtSamplePointArbitrator(av)
{
    lessThan = lt;
}


// ****************************************************************************
//  Method: avtRelativeValueSamplePointArbitrator destructor
//
//  Purpose:
//      Defines the destructor.  Note: this should not be inlined in the header
//      because it causes problems for certain compilers.
//
//  Programmer: Hank Childs
//  Creation:   January 29, 2005
//
// ****************************************************************************

avtRelativeValueSamplePointArbitrator::~avtRelativeValueSamplePointArbitrator()
{
    ;
}


// ****************************************************************************
//  Method: avtRelativeValueSamplePointArbitrator::ShouldOverwrite
//
//  Purpose:
//      Decides whether the new sample point has a more important contribution
//      to the picture than the old sample point.  In this case, we define
//      important based on relative value.
//
//  Arguments:
//     whatIgot        The current value.
//     whatIcouldHave  The potential new value.
//
//  Returns:    true if the new value is better than the old.
//
//  Programmer: Hank Childs
//  Creation:   January 29, 2005
//
// ****************************************************************************

bool
avtRelativeValueSamplePointArbitrator::ShouldOverwrite(float whatIgot,
                                                       float whatIcouldHave)
{
    if (lessThan)
        return (whatIgot < whatIcouldHave);
    else
        return (whatIgot > whatIcouldHave);
}


