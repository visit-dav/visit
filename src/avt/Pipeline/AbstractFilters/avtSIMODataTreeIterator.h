// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                        avtSIMODataTreeIterator.h                          //
// ************************************************************************* //

#ifndef AVT_SIMO_DATA_TREE_ITERATOR_H
#define AVT_SIMO_DATA_TREE_ITERATOR_H

#include <pipeline_exports.h>

#include <avtDatasetToDatasetFilter.h>


class  avtExtents;


// ****************************************************************************
//  Class: avtSIMODataTreeIterator
//
//  Purpose:
//      A derived type of avtDatasetToDatasetFilter.  This will iterate over 
//      each avtDataRepresentation from an input avtDataTree.  This particular
//      flavor is "SIMO": Single Input Multiple Output, meaning that the
//      output for a single avtDataRepresentation can and likely will be
//      multiple avtDataRepresentation outputs.
//
//  Programmer: Hank Childs
//  Creation:   July 24, 2000
//
//  Modifications:
//
//    Jeremy Meredith, Thu Sep 28 13:05:02 PDT 2000
//    Nade ExecuteDomain take one vtkDataSet as input and and return
//    a new output one.
//
//    Kathleen Bonnell, Fri Feb  9 14:47:10 PST 2001 
//    Renamed this class from avtDataTreeIterator.  Made ExecuteDomain return
//    an avtDomainTree.
//
//    Kathleen Bonnell, Thu Apr 12 10:25:04 PDT 2001 
//    Renamed this class as avtSIMODataTreeIterator. Added recursive
//    Execute method to walk down input tree.
//
//    Hank Childs, Wed Jun 20 09:39:01 PDT 2001
//    Added support progress callback.
//
//    Kathleen Bonnell, Wed Sep 19 13:45:33 PDT 2001
//    Added string argument to ExecuteDataTree method. 
//
//    Hank Childs, Wed Oct 24 14:21:18 PDT 2001
//    Moved PreExecute and PostExecute to avtFilter.
//
//    Hank Childs, Tue Nov  6 11:46:10 PST 2001
//    Add support for overriding extents.
//
//    Hank Childs, Mon Dec 27 10:58:14 PST 2004
//    Made inheritance virtual.
//
//    Eric Brugger, Fri Jul 18 13:24:47 PDT 2014
//    Modified the class to work with avtDataRepresentation.
//
// **************************************************************************** 

class PIPELINE_API avtSIMODataTreeIterator : virtual public 
                                                      avtDatasetToDatasetFilter
{
   public:
                             avtSIMODataTreeIterator();
     virtual                ~avtSIMODataTreeIterator();

  protected:
    int                      currentNode;
    int                      totalNodes;

    virtual void             Execute(void);
    virtual void             Execute(avtDataTree_p inDT, avtDataTree_p &outDT);
    virtual avtDataTree_p    ExecuteDataTree(avtDataRepresentation *);
    virtual avtDataTree_p    ExecuteDataTree(vtkDataSet *,int,std::string);

    virtual bool             ThreadSafe(void) { return(false); };
    void                     FinishExecute(void);

    void                     OverrideOriginalSpatialExtents(void)
                                 { overrideOriginalSpatialExtents = true; };
    void                     OverrideOriginalDataExtents(void)
                                 { overrideOriginalDataExtents = true; };

  private:
    bool                     overrideOriginalSpatialExtents;
    bool                     overrideOriginalDataExtents;

    void                     UpdateExtents(avtDataTree_p);

    static void              ExecuteDataTreeOnThread(void *cbdata);
};


#endif


