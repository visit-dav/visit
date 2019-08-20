// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                         avtSubsetFilter.h                                 //
// ************************************************************************* //

#ifndef AVT_SUBSET_FILTER_H
#define AVT_SUBSET_FILTER_H

#include <avtSIMODataTreeIterator.h>

#include <SubsetAttributes.h>

#include <string>


// ****************************************************************************
//  Class: avtSubsetFilter
//
//  Purpose:  Ensures that the correct subset names are passed along
//            as labels.
//
//  Programmer: Kathleen Bonnell 
//  Creation:   October 16, 2001 
//
//  Modifications:
//    Eric Brugger, Fri Dec 14 13:08:12 PST 2001
//    I modified the class to inherit from avtSIMODataTreeIterator so that I
//    could implement an ExecuteDataTree method instead of the Execute
//    Method.
//
//    Kathleen Bonnell, Mon Apr 29 17:31:22 PDT 2002
//    I added PostExecute so that accurate labels could be passed via
//    the DataAttributes object. 
//
//    Kathleen Bonnell, Fri Nov 12 11:50:33 PST 2004
//    Moved constructor to source code, added keepNodeZone data member. 
//
//    Eric Brugger, Tue Aug 19 11:19:35 PDT 2014
//    Modified the class to work with avtDataRepresentation.
//
// ****************************************************************************

class avtSubsetFilter : public avtSIMODataTreeIterator
{
  public:
                          avtSubsetFilter();
    virtual              ~avtSubsetFilter(){}; 

    virtual const char   *GetType(void) {return "avtSubsetFilter";};
    virtual const char   *GetDescription(void) 
                              { return "Setting subset names"; };

    void                  SetPlotAtts(const SubsetAttributes *);

  protected:
    SubsetAttributes      plotAtts;

    virtual avtDataTree_p ExecuteDataTree(avtDataRepresentation *);
    virtual void          UpdateDataObjectInfo(void);
    virtual avtContract_p
                          ModifyContract(avtContract_p);

    virtual void          PostExecute(void);

  private:
    bool                  keepNodeZone;
};


#endif


