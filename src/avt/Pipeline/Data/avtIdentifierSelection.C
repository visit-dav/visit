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

// ************************************************************************* //
//                           avtIdentifierSelection.C                        //
// ************************************************************************* //

#include <avtIdentifierSelection.h>

#include <snprintf.h>


// ****************************************************************************
//  Method: avtIdentifierSelection constructor
//
//  Programmer: Hank Childs
//  Creation:   March 5, 2008
//
//  Modifications:
//    Brad Whitlock, Thu Mar 15 14:17:11 PDT 2012
//    Initialize ids, idVar.
//
// ****************************************************************************

avtIdentifierSelection::avtIdentifierSelection() : avtDataSelection(), 
    ids(), idVar()
{
}


// ****************************************************************************
//  Method: avtIdentifierSelection destructor
//
//  Programmer: Hank Childs
//  Creation:   March 5, 2008
//
// ****************************************************************************

avtIdentifierSelection::~avtIdentifierSelection()
{
}


// ****************************************************************************
//  Method: operator== 
//
//  Programmer: Hank Childs
//  Creation:   March 5, 2008
//
// ****************************************************************************

bool
avtIdentifierSelection::operator==(const avtIdentifierSelection &sel) const
{
    return (ids == sel.ids) && (idVar == sel.idVar);
}


// ****************************************************************************
//  Method: avtIdentifierSelection::Destruct
//
//  Purpose:
//      Calls the destructor for an identifier selection.
//
//  Programmer: Hank Childs
//  Creation:   March 5, 2008
//
// ****************************************************************************

void
avtIdentifierSelection::Destruct(void *i)
{
    avtIdentifierSelection *ids = (avtIdentifierSelection *) i;
    delete ids;
}


// ****************************************************************************
//  Method: avtIdentifierRangeSelection::DescriptionString
//
//  Purpose:
//      Creates a string (used as a key for caching) that describes this
//      selection.
//
//  Programmmer: Hank Childs
//  Creation:    December 20, 2011
//
// ****************************************************************************

std::string
avtIdentifierSelection::DescriptionString(void)
{
    // This one is going to take some thought about how to build the
    // string in a way that doesn't get too long.  Apologies to the next
    // developer, but it isn't the problem I'm trying to solve right now.
    // The short cut is just to have file formats that do these selections
    // to just declare they can't do caching.
    EXCEPTION1(VisItException, "Exception due to unimplemented code: avtIdentifiedRangeSelection");
}


