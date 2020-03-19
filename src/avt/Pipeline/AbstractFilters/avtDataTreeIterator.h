// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                          avtDataTreeIterator.h                            //
// ************************************************************************* //

#ifndef AVT_DATA_TREE_ITERATOR_H
#define AVT_DATA_TREE_ITERATOR_H

#include <pipeline_exports.h>

#include <avtSIMODataTreeIterator.h>


// ****************************************************************************
//  Class: avtDataTreeIterator
//
//  Purpose:
//      This is an abstract type.  Its purpose is to provide a service.  That
//      service is that it walks through an input avtDataTree and calls the
//      method "ExecuteData" once for each vtkDataSet in the tree.  In addition,
//      it assembles an avtDataTree output from the outputs of each ExecuteData
//      call.
//
//  Programmer: Hank Childs
//  Creation:   July 24, 2000
//
//  Modifications:
//
//    Jeremy Meredith, Thu Sep 28 13:05:02 PDT 2000
//    Made ExecuteDomain take one vtkDataSet as input and and return
//    a new output one.
//
//    Kathleen Bonnell, Feb  9 14:47:10 PST 2001  
//    Removed 'Execute' method, and made this class inherit from 
//    avtDomainTreeDataTreeIterator.  Allows for derived types to still
//    return vtkDataSet * from ExecuteDomain, but now wrapped in
//    avtDomainTree.
//
//    Kathleen Bonnell, Tue Apr 10 10:49:10 PDT 2001 
//    Change inheritance to avtSIMODataTreeIterator. 
//
//    Kathleen Bonnell, Wed Sep 19 13:35:35 PDT 200 
//    Added string argument to Execute method. 
//
//    Hank Childs, Fri Feb  1 14:48:15 PST 2002
//    Added mechanism for managing memory for derived types.
//
//    Hank Childs, Tue Sep 10 13:13:01 PDT 2002
//    Better support for releasing data.
//
//    Hank Childs, Mon Dec 27 10:58:14 PST 2004
//    Made inheritance virtual.
//
//    Hank Childs, Thu Dec 21 09:17:43 PST 2006
//    Remove support for debug dumps.
//
//    Eric Brugger, Fri Jul 18 14:00:24 PDT 2014
//    Modified the class to work with avtDataRepresentation.
//
//    Eric Brugger, Fri Sep 26 14:14:36 PDT 2014
//    I removed all the remaining traces of vtkDataSet since all the
//    filters have been converted to use avtDataRepresentation.
//
// **************************************************************************** 

class PIPELINE_API avtDataTreeIterator : virtual public avtSIMODataTreeIterator
{
  public:
                             avtDataTreeIterator();
    virtual                 ~avtDataTreeIterator();

    virtual void             ReleaseData(void);

  protected:
    virtual avtDataTree_p    ExecuteDataTree(avtDataRepresentation *);
    virtual avtDataRepresentation *ExecuteData(avtDataRepresentation *) = 0;
};


#endif


