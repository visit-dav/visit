// ************************************************************************* //
//                         avtBoundaryFilter.h                               //
// ************************************************************************* //

#ifndef AVT_BOUNDARY_FILTER_H
#define AVT_BOUNDARY_FILTER_H


#include <avtDataTreeStreamer.h>
#include <BoundaryAttributes.h>


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
// ****************************************************************************

class avtBoundaryFilter : public avtDataTreeStreamer
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

    virtual avtDataTree_p ExecuteDataTree(vtkDataSet *, int, string);
    virtual void          RefashionDataObjectInfo(void);
    virtual avtPipelineSpecification_p
                          PerformRestriction(avtPipelineSpecification_p);

    virtual void          PostExecute(void);

  private:
    bool                  keepNodeZone;
};


#endif


