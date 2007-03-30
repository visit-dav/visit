// ************************************************************************* //
//                   avtTotalRevolvedVolumeQuery.h                           //
// ************************************************************************* //

#ifndef AVT_TOTALREVOLVEDVOLUME_QUERY_H
#define AVT_TOTALREVOLVEDVOLUME_QUERY_H
#include <query_exports.h>

#include <avtSummationQuery.h>


class     avtRevolvedVolume;


// ****************************************************************************
//  Class: avtTotalRevolvedVolumeQuery
//
//  Purpose:
//      A query for TotalRevolvedVolume.
//
//  Notes:
//    Taken mostly from Hank Childs' avtTotalRevolvedVolumeFilter and reworked 
//    to fit into the Query hierarchy.  avtTotalRevolvedVolumeFilter is now 
//    deprecated.
//
//  Programmer: Kathleen Bonnell 
//  Creation:   September 30, 2002 
//
// ****************************************************************************

class QUERY_API avtTotalRevolvedVolumeQuery : public avtSummationQuery
{
  public:
                         avtTotalRevolvedVolumeQuery();
    virtual             ~avtTotalRevolvedVolumeQuery();

    virtual const char  *GetType(void)  
                             { return "avtTotalRevolvedVolumeQuery"; };
    virtual const char  *GetDescription(void)
                             { return "TotalRevolvedVolume"; };

  protected:
    virtual avtDataObject_p       ApplyFilters(avtDataObject_p);
    avtRevolvedVolume            *volume;
    virtual int                   GetNFilters() { return 1; };

};


#endif


