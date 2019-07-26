// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                            avtOnionPeelFilter.h                           //
// ************************************************************************* //

#ifndef AVT_ONION_PEEL_FILTER_H
#define AVT_ONION_PEEL_FILTER_H

#include <avtPluginDataTreeIterator.h>

#include <OnionPeelAttributes.h>

class vtkOnionPeelFilter;
class vtkPolyDataOnionPeelFilter;


// ****************************************************************************
//  Class: avtOnionPeelFilter
//
//  Purpose:
//    A filter that turns dataset into unstructured grid comprised of
//    a seed cell and a number of layers surrounding the seed cell as
//    specified by the user.
//
//  Programmer: Kathleen Bonnell 
//  Creation:   October 09, 2000
//
//  Modifications:
//
//    Hank Childs, Fri Oct 27 10:23:52 PDT 2000
//    Added argument to ExecuteDomain to match interited interface.
//
//    Jeremy Meredith, Thu Mar  1 13:29:27 PST 2001
//    Made attributes be stored as an OnionPeelAttributes class.
//
//    Jeremy Meredith, Sun Mar  4 16:59:57 PST 2001
//    Added a static Create method.
//
//    Kathleen Bonnell, Tue Apr 10 10:49:10 PDT 2001 
//    Changed ExecuteDomain to ExecuteData. 
//
//    Hank Childs, Wed Jun  6 08:58:51 PDT 2001
//    Renamed some methods to fit changes in base class.
//
//    Kathleen bonnell, Tue Oct 16 14:55:41 PDT 2001
//    Added VerifyInput, ModifyContract. 
//
//    Hank Childs, Wed May 22 17:25:22 PDT 2002
//    Added BadSeedCell.
//
//    Kathleen Bonnell, Thu Aug 15 18:37:59 PDT 2002  
//    Added Pre/PostExecute methods. Added bool argument to BadSeedCell and
//    BadSeedCellCallback.  Added groupCategory, encounteredGhostSeed, and 
//    successfullyExecuted. 
//
//    Eric Brugger, Thu Jul 31 11:42:38 PDT 2014
//    Modified the class to work with avtDataRepresentation.
//  
//    Alister Maguire, Mon Oct 24 11:06:24 PDT 2016
//    Removed opf and poly_opf for thread safety. They are
//    now stack variables in ExecuteData. Added ThreadSafe
//    method to header file. 
//
// ****************************************************************************

class avtOnionPeelFilter : public avtPluginDataTreeIterator
{
  public:
                         avtOnionPeelFilter();
    virtual             ~avtOnionPeelFilter();

    static avtFilter    *Create();

    virtual const char  *GetType(void)  { return "avtOnionPeelFilter"; };
    virtual const char  *GetDescription(void)
                             { return "Selecting neighbors for onion peel"; };
    virtual void         ReleaseData(void);

    virtual bool         ThreadSafe(void) { return(true); };
    virtual void         SetAtts(const AttributeGroup*);
    virtual bool         Equivalent(const AttributeGroup*);

    void                 BadSeed(int, int, bool);
    static void          BadSeedCallback(void *, int, int, bool);


  protected:
    OnionPeelAttributes   atts;

    int                   badSeed;
    int                   maximumIds;
    bool                  encounteredBadSeed;
    bool                  encounteredGhostSeed;
    bool                  groupCategory;
    bool                  successfullyExecuted;

    virtual avtDataRepresentation *ExecuteData(avtDataRepresentation *);
    virtual void          PreExecute();
    virtual void          PostExecute();

    virtual void          VerifyInput(void);
    virtual void          UpdateDataObjectInfo(void);
    virtual avtContract_p
                          ModifyContract(avtContract_p);

};


#endif


