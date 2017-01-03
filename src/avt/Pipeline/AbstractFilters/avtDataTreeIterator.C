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
//                           avtDataTreeIterator.C                           //
// ************************************************************************* //

#include <avtDataRepresentation.h>
#include <avtDataTree.h>
#include <avtDataTreeIterator.h>

#include <ImproperUseException.h>

// ****************************************************************************
//  Method: avtDataTreeIterator constructor
//
//  Programmer: Hank Childs
//  Creation:   February 1, 2002
//
//  Modifications:
//    Eric Bruggger, Fri Sep 26 14:15:16 PDT 2014
//    I removed all the remaining traces of vtkDataSet since all the
//    filters have been converted to use avtDataRepresentation.
//
// ****************************************************************************

avtDataTreeIterator::avtDataTreeIterator()
{
}


// ****************************************************************************
//  Method: avtDataTreeIterator destructor
//
//  Programmer: Hank Childs
//  Creation:   February 1, 2002
//
//  Modifications:
//    Eric Bruggger, Fri Sep 26 14:15:16 PDT 2014
//    I removed all the remaining traces of vtkDataSet since all the
//    filters have been converted to use avtDataRepresentation.
//
// ****************************************************************************

avtDataTreeIterator::~avtDataTreeIterator()
{
}


// ****************************************************************************
//  Method: avtDataTreeIterator::ReleaseData
//
//  Purpose:
//      Releases any problem-size data associated with this filter.
//
//  Programmer: Hank Childs
//  Creation:   September 10, 2002
//
//  Modifications:
//    Eric Bruggger, Fri Sep 26 14:15:16 PDT 2014
//    I removed all the remaining traces of vtkDataSet since all the
//    filters have been converted to use avtDataRepresentation.
//
// ****************************************************************************

void
avtDataTreeIterator::ReleaseData(void)
{
    avtSIMODataTreeIterator::ReleaseData();
}


// ****************************************************************************
//  Method: avtDataTreeIterator::ExecuteDataTree
//
//  Purpose:
//      Defines the pure virtual function executedomaintree.  
//      Serves as a wrapper for the ExecuteDomain method.
//
//  Arguments:
//      in_dr   The data representation to pass to the derived type.
//
//  Programmer: Kathleen Bonnell 
//  Creation:   February 9, 2001
//
//  Modifications:
//
//    Kathleen Bonnell, Tue Apr 10 10:49:10 PDT 2001
//    Made this method return avtDataTree.
//
//    Kathleen Bonnell, Wed Sep 19 13:35:35 PDT 2001 
//    Added string argument so that labels will get passed on. 
// 
//    Hank Childs, Fri Oct 19 10:56:55 PDT 2001
//    Allow for derived types to return NULL.
//
//    Hank Childs, Wed Sep 11 09:17:46 PDT 2002
//    Pass the label down to the derived types as well.
//
//    Hank Childs, Mon Jun 27 10:02:55 PDT 2005
//    Choose better file names when doing a "-dump" in parallel.
//
//    Hank Childs, Tue Jul  5 09:41:28 PDT 2005
//    Fix cut-n-paste bug with last change.
//
//    Hank Childs, Wed Aug 31 09:10:11 PDT 2005
//    Make sure that -dump in parallel increments the dump index.
//
//    Hank Childs, Thu Dec 21 15:38:53 PST 2006
//    Removed -dump functionality, since it is now handled at a lower level.
//
//    Eric Brugger, Fri Jul 18 14:45:40 PDT 2014
//    Modified the class to work with avtDataRepresentation.
//
//    Brad Whitlock, Thu Sep  4 16:20:20 PDT 2014
//    I fixed memory leak with the data representation returned from
//    ExecuteData.
//
//    Eric Brugger, Fri Sep 26 08:42:24 PDT 2014
//    I modified the routine to create the same type of avtDataTree that it
//    did previous to modifiying it to work with avtDataRepresentation.
//
//    Eric Bruggger, Fri Sep 26 14:15:16 PDT 2014
//    I removed all the remaining traces of vtkDataSet since all the
//    filters have been converted to use avtDataRepresentation.
//
// ****************************************************************************

avtDataTree_p
avtDataTreeIterator::ExecuteDataTree(avtDataRepresentation *in_dr)
{
    avtDataRepresentation *out_dr = ExecuteData(in_dr);

    if (out_dr == NULL)
    {
        return NULL;
    }

    // This code ends up creating a copy of out_dr in the data tree.
    avtDataTree_p retval = new avtDataTree(out_dr);

    // If the derived type created a new avtDataRepresentation instance then
    // it's been copied in the avtDataTree. We need to remove the instance
    // that was returned or we'll end up with a VTK reference count leak.
    if(out_dr != in_dr)
    {
        delete out_dr;
    }

    return retval;
}
