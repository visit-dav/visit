// ************************************************************************* //
//                      avtWeightedVariableSummationQuery.h                  //
// ************************************************************************* //

#ifndef AVT_WEIGHTED_VARIABLE_SUMMATION_QUERY_H
#define AVT_WEIGHTED_VARIABLE_SUMMATION_QUERY_H

#include <query_exports.h>

#include <avtSummationQuery.h>


class     avtBinaryMultiplyFilter;
class     avtRevolvedVolume;
class     avtVMetricArea;
class     avtVMetricVolume;


// ****************************************************************************
//  Class: avtWeightedVariableSummationQuery
//
//  Purpose:
//      A query that will sum all of one variables values.
//
//  Programmer: Hank Childs
//  Creation:   February 3, 2004
//
//  Modifications:
//    Kathleen Bonnell, Wed Jul 28 08:50:51 PDT 2004
//    Added VerifyInput.
//
//    Kathleen Bonnell, Fri Feb  3 10:32:12 PST 2006 
//    Added revolvedVolume. 
//
// ****************************************************************************

class QUERY_API avtWeightedVariableSummationQuery : public avtSummationQuery
{
  public:
                         avtWeightedVariableSummationQuery();
    virtual             ~avtWeightedVariableSummationQuery();

    virtual const char  *GetType(void)  
                             { return "avtWeightedVariableSummationQuery"; };

  protected:
    avtVMetricArea            *area;
    avtVMetricVolume          *volume;
    avtRevolvedVolume         *revolvedVolume;
    avtBinaryMultiplyFilter   *multiply;

    virtual avtDataObject_p    ApplyFilters(avtDataObject_p);
    virtual int                GetNFilters(void) { return 2; };
    virtual void               VerifyInput(void);
};


#endif


