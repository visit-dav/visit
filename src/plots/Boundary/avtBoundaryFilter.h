// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                         avtBoundaryFilter.h                               //
// ************************************************************************* //

#ifndef AVT_BOUNDARY_FILTER_H
#define AVT_BOUNDARY_FILTER_H

#include <avtSIMODataTreeIterator.h>

#include <BoundaryAttributes.h>

#include <string>


// ****************************************************************************
//  Class: avtBoundaryFilter
//
//  Purpose:  Ensures that the correct boundary names are passed along
//            as labels.
//
//  Programmer: Jeremy Meredith
//  Creation:   May  7, 2003
//
//  Note:  taken almost verbatim from the Subset plot
//
//  Modifications:
//    Kathleen Bonnell, Fri Nov 12 10:23:09 PST 2004
//    Moved constructor to source file, added keepNodeZone data member.
//
//    Eric Brugger, Tue Aug 19 09:57:11 PDT 2014
//    Modified the class to work with avtDataRepresentation.
//
// ****************************************************************************

class avtBoundaryFilter : public avtSIMODataTreeIterator
{
  public:
                          avtBoundaryFilter();
    virtual              ~avtBoundaryFilter(){}; 

    virtual const char   *GetType(void) {return "avtBoundaryFilter";};
    virtual const char   *GetDescription(void) 
                              { return "Setting boundary names"; };

    void                  SetPlotAtts(const BoundaryAttributes *);

  protected:
    BoundaryAttributes    plotAtts;

    virtual avtDataTree_p ExecuteDataTree(avtDataRepresentation *);
    virtual void          UpdateDataObjectInfo(void);
    virtual avtContract_p
                          ModifyContract(avtContract_p);

    virtual void          PostExecute(void);

  private:
    bool                  keepNodeZone;
};


#endif


