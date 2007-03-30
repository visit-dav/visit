// ************************************************************************* //
//                   avtTotalSurfaceAreaQuery.h                              //
// ************************************************************************* //

#ifndef AVT_TOTALSURFACEAREA_QUERY_H
#define AVT_TOTALSURFACEAREA_QUERY_H
#include <query_exports.h>

#include <avtSummationQuery.h>


class     avtFacelistFilter;
class     avtVMetricArea;


// ****************************************************************************
//  Class: avtTotalSurfaceAreaQuery
//
//  Purpose:
//      A query for TotalSurfaceArea.
//
//  Notes:
//    Taken mostly from Hank Childs' avtTotalSurfaceAreaFilter and reworked to
//    fit into the Query hierarchy.  avtTotalSurfaceAreaFilter is now deprecated.
//
//  Programmer: Kathleen Bonnell 
//  Creation:   September 30, 2002 
//
//  Modifications:
//    Kathleen Bonnell, Wed Oct 23 15:11:44 PDT 2002 
//    Added VerifyInput.
//
// ****************************************************************************

class QUERY_API avtTotalSurfaceAreaQuery : public avtSummationQuery
{
  public:
                         avtTotalSurfaceAreaQuery();
    virtual             ~avtTotalSurfaceAreaQuery();

    virtual const char  *GetType(void)  { return "avtTotalSurfaceAreaQuery"; };
    virtual const char  *GetDescription(void)
                             { return "TotalSurfaceArea"; };

  protected:
    virtual void               VerifyInput(void); 
    virtual avtDataObject_p    ApplyFilters(avtDataObject_p);
    avtVMetricArea            *area;
    avtFacelistFilter         *facelist;

    virtual int                GetNFilters() { return 2; };

};


#endif


