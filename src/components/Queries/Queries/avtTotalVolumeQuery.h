// ************************************************************************* //
//                        avtTotalVolumeQuery.h                              //
// ************************************************************************* //

#ifndef AVT_TOTALVOLUME_QUERY_H
#define AVT_TOTALVOLUME_QUERY_H
#include <query_exports.h>
#include <avtSummationQuery.h>

class     avtVMetricVolume;


// ****************************************************************************
//  Class: avtTotalVolumeQuery
//
//  Purpose:
//      A query for TotalVolume.
//
//  Notes:
//    Taken mostly from Hank Childs' avtTotalVolumeFilter and reworked to
//    fit into the Query hierarchy.  avtTotalVolumeFilter is now deprecated.
//
//  Programmer: Kathleen Bonnell 
//  Creation:   September 30, 2002 
//
//  Modifications:
//    Kathleen Bonnell, Wed Oct 23 15:11:44 PDT 2002 
//    Add VerifyInput.
//
// ****************************************************************************

class QUERY_API avtTotalVolumeQuery : public avtSummationQuery
{
  public:
                         avtTotalVolumeQuery();
    virtual             ~avtTotalVolumeQuery();

    virtual const char  *GetType(void)  
                             { return "avtTotalVolumeQuery"; };
    virtual const char  *GetDescription(void)
                             { return "TotalVolume"; };

  protected:
    virtual void               VerifyInput(void);
    virtual avtDataObject_p    ApplyFilters(avtDataObject_p);
    avtVMetricVolume          *volume;
    virtual int                GetNFilters() { return 1; };

};


#endif


