// ************************************************************************* //
//                       avtFilledBoundaryFilter.h                           //
// ************************************************************************* //

#ifndef AVT_FILLED_BOUNDARY_FILTER_H
#define AVT_FILLED_BOUNDARY_FILTER_H


#include <avtDataTreeStreamer.h>
#include <FilledBoundaryAttributes.h>


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
// ****************************************************************************

class avtFilledBoundaryFilter : public avtDataTreeStreamer
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

    virtual avtDataTree_p ExecuteDataTree(vtkDataSet *, int, string);
    virtual void          RefashionDataObjectInfo(void);
    virtual avtPipelineSpecification_p
                          PerformRestriction(avtPipelineSpecification_p);

    virtual void          PostExecute(void);

  private:
    bool                  keepNodeZone;
};


#endif


