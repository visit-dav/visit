// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                         avtLabelSubsetsFilter.h                           //
// ************************************************************************* //

#ifndef AVT_LABEL_SUBSETS_FILTER_H
#define AVT_LABEL_SUBSETS_FILTER_H

#include <avtSIMODataTreeIterator.h>

#include <string>


// ****************************************************************************
//  Class: avtLabelSubsetsFilter
//
//  Purpose:  Ensures that the correct subset names are passed along
//            as labels.
//
//  Programmer: Kathleen Bonnell 
//  Creation:   October 16, 2001 
//
//  Modifications:
//    Brad Whitlock, Wed Aug 3 17:58:50 PST 2005
//    Copied from the Subset plot and made its only job to split up materials
//    and pass along other subset variables.
//
//    Eric Brugger, Tue Aug 19 10:51:44 PDT 2014
//    Modified the class to work with avtDataRepresentation.
//
// ****************************************************************************

class avtLabelSubsetsFilter : public avtSIMODataTreeIterator
{
  public:
                          avtLabelSubsetsFilter();
    virtual              ~avtLabelSubsetsFilter(){}; 

    virtual const char   *GetType(void) {return "avtLabelSubsetsFilter";};
    virtual const char   *GetDescription(void) 
                              { return "Setting subset names"; };

    void                  SetNeedMIR(bool val) { needMIR = val; };
  protected:
    virtual avtDataTree_p ExecuteDataTree(avtDataRepresentation *);
    virtual avtContract_p
                          ModifyContract(avtContract_p);

    virtual void          PostExecute(void);

    bool needMIR;
};


#endif
