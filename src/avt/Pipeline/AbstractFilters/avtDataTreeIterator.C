// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

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
