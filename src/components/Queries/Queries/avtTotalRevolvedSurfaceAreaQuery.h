// ************************************************************************* //
//                    avtTotalRevolvedSurfaceAreaQuery.h                     //
// ************************************************************************* //

#ifndef AVT_TOTALREVOLVED_SURFACE_AREA_QUERY_H
#define AVT_TOTALREVOLVED_SURFACE_AREA_QUERY_H

#include <query_exports.h>

#include <avtSummationQuery.h>


class     avtRevolvedSurfaceArea;


// ****************************************************************************
//  Class: avtTotalRevolvedSurfaceAreaQuery
//
//  Purpose:
//      A query for TotalRevolvedVolume.
//      A query that calculates the total surface area when revolving lines
//      around the X-axis.
//  Notes:
//  Programmer: Hank Childs
//  Creation:   March 18, 2003
//
//  Modifications:
//    Kathleen Bonnell, Wed Jul 28 08:35:22 PDT 2004
//    Added VerifyInput.
//
// ****************************************************************************

class QUERY_API avtTotalRevolvedSurfaceAreaQuery : public avtSummationQuery
{
  public:
                         avtTotalRevolvedSurfaceAreaQuery();
    virtual             ~avtTotalRevolvedSurfaceAreaQuery();

    virtual const char  *GetType(void)  
                             { return "avtTotalRevolvedSurfaceAreaQuery"; };
    virtual const char  *GetDescription(void)
                             { return "TotalRevolvedSurfaceArea"; };

  protected:
    virtual avtDataObject_p       ApplyFilters(avtDataObject_p);
    virtual void                  VerifyInput(void);
    avtRevolvedSurfaceArea       *surface_area;
    virtual int                   GetNFilters() { return 1; };

};


#endif


