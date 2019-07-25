// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                       avtFilledBoundaryFilter.h                           //
// ************************************************************************* //

#ifndef AVT_FILLED_BOUNDARY_FILTER_H
#define AVT_FILLED_BOUNDARY_FILTER_H

#include <avtSIMODataTreeIterator.h>

#include <FilledBoundaryAttributes.h>

#include <string>


// ****************************************************************************
//  Class: avtFilledBoundaryFilter
//
//  Purpose:  Ensures that the correct boundary names are passed along
//            as labels.
//
//  Programmer: Jeremy Meredith
//  Creation:   May  9, 2003
//
//  Note:  taken almost verbatim from the Subset plot
//
//  Modifications:
//    Kathleen Bonnell, Fri Nov 12 10:51:59 PST 2004
//    Moved constructor to source code, added keepNodeZone data member.
//
//    Eric Brugger, Tue Aug 19 10:14:45 PDT 2014
//    Modified the class to work with avtDataRepresentation.
//
// ****************************************************************************

class avtFilledBoundaryFilter : public avtSIMODataTreeIterator
{
  public:
                          avtFilledBoundaryFilter();
    virtual              ~avtFilledBoundaryFilter(){}; 

    virtual const char   *GetType(void) {return "avtFilledBoundaryFilter";};
    virtual const char   *GetDescription(void) 
                              { return "Setting boundary names"; };

    void                  SetPlotAtts(const FilledBoundaryAttributes *);

  protected:
    FilledBoundaryAttributes    plotAtts;

    virtual avtDataTree_p ExecuteDataTree(avtDataRepresentation *);
    virtual void          UpdateDataObjectInfo(void);
    virtual avtContract_p
                          ModifyContract(avtContract_p);

    virtual void          PostExecute(void);

  private:
    bool                  keepNodeZone;
};


#endif


