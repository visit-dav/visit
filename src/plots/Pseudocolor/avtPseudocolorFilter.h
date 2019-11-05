// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                           avtPseudocolorFilter.h                          //
// ************************************************************************* //

#ifndef AVT_PSEUDOCOLOR_FILTER_H
#define AVT_PSEUDOCOLOR_FILTER_H

#include <avtDataTreeIterator.h>

#include <PseudocolorAttributes.h>

#include <string>


// ****************************************************************************
//  Class: avtPseudocolorFilter
//
//  Purpose:  To set specific flags in the pipeline that may be necessary
//            when picking on point meshes.
//
//  Programmer: Kathleen Bonnell
//  Creation:   October 29, 2004
//
//  Modifications:
//    Kathleen Bonnell, Fri Nov 12 11:35:11 PST 2004
//    Added PlotAtts.
//
//    Kathleen Biagas, Fri Nov  2 10:25:25 PDT 2012
//    Added primaryVar.
//
//    Eric Brugger, Tue Aug 19 11:10:33 PDT 2014
//    Modified the class to work with avtDataRepresentation.
//
//    Alister Maguire, Tue Jul 16 14:12:20 PDT 2019
//    Added mustRemoveFacesBeforeGhosts.
//
// ****************************************************************************

class avtPseudocolorFilter : public avtDataTreeIterator
{
  public:
                              avtPseudocolorFilter();
    virtual                  ~avtPseudocolorFilter();

    virtual const char       *GetType(void)   { return "avtPseudocolorFilter"; };
    virtual const char       *GetDescription(void)
                                  { return "Preparing pipeline"; };
    void                      SetPlotAtts(const PseudocolorAttributes *);

  protected:
    bool                      keepNodeZone;
    bool                      mustRemoveFacesBeforeGhosts;

    virtual avtDataRepresentation *ExecuteData(avtDataRepresentation *);
    virtual void              UpdateDataObjectInfo(void);
    virtual avtContract_p
                              ModifyContract(avtContract_p);
  private:
    PseudocolorAttributes     plotAtts;
    std::string               primaryVar;
};


#endif


