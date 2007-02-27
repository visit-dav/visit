/*****************************************************************************
*
* Copyright (c) 2000 - 2007, The Regents of the University of California
* Produced at the Lawrence Livermore National Laboratory
* All rights reserved.
*
* This file is part of VisIt. For details, see http://www.llnl.gov/visit/. The
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
*    documentation and/or materials provided with the distribution.
*  - Neither the name of the UC/LLNL nor  the names of its contributors may be
*    used to  endorse or  promote products derived from  this software without
*    specific prior written permission.
*
* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT  HOLDERS AND CONTRIBUTORS "AS IS"
* AND ANY EXPRESS OR  IMPLIED WARRANTIES, INCLUDING,  BUT NOT  LIMITED TO, THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND  FITNESS FOR A PARTICULAR  PURPOSE
* ARE  DISCLAIMED.  IN  NO  EVENT  SHALL  THE  REGENTS  OF  THE  UNIVERSITY OF
* CALIFORNIA, THE U.S.  DEPARTMENT  OF  ENERGY OR CONTRIBUTORS BE  LIABLE  FOR
* ANY  DIRECT,  INDIRECT,  INCIDENTAL,  SPECIAL,  EXEMPLARY,  OR CONSEQUENTIAL
* DAMAGES (INCLUDING, BUT NOT  LIMITED TO, PROCUREMENT OF  SUBSTITUTE GOODS OR
* SERVICES; LOSS OF  USE, DATA, OR PROFITS; OR  BUSINESS INTERRUPTION) HOWEVER
* CAUSED  AND  ON  ANY  THEORY  OF  LIABILITY,  WHETHER  IN  CONTRACT,  STRICT
* LIABILITY, OR TORT  (INCLUDING NEGLIGENCE OR OTHERWISE)  ARISING IN ANY  WAY
* OUT OF THE  USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH
* DAMAGE.
*
*****************************************************************************/

// ************************************************************************* //
//                               avtSILNamespace.C                           //
// ************************************************************************* //

#include <avtSILEnumeratedNamespace.h>
#include <avtSILRangeNamespace.h>

#include <NamespaceAttributes.h>

#include <DebugStream.h>


using  std::vector;


// ****************************************************************************
//  Method: avtSILNamespace::GetNamespace
//
//  Purpose:
//      Takes in a NamespaceAttributes object and creates the correct derived
//      type of avtSILNamespace.
//
//  Arguments:
//      atts      The attribute subject.
//
//  Returns:      The new namespace object.
//
//  Programmer:   Hank Childs
//  Creation:     March 29, 2001
//
// ****************************************************************************

avtSILNamespace *
avtSILNamespace::GetNamespace(const NamespaceAttributes *atts)
{
    int type = atts->GetType();
    if (type == 0)
    {
        return new avtSILEnumeratedNamespace(atts->GetSubsets());
    }
    else if (type == 1)
    {
        const vector<int> &ss = atts->GetSubsets();
        int  subset = ss[0];
        int  min = atts->GetMin();
        int  max = atts->GetMax();
        return new avtSILRangeNamespace(subset, min, max);
    }

    //
    // Don't know what to do, so flag it.
    //
    debug1 << "Invalid namespace type " << type << endl;
    return NULL;
}


