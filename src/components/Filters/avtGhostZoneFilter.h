// ************************************************************************* //
//                            avtGhostZoneFilter.h                           //
// ************************************************************************* //

#ifndef AVT_GHOST_ZONE_FILTER_H
#define AVT_GHOST_ZONE_FILTER_H

#include <pipeline_exports.h>

#include <avtStreamer.h>

class     vtkDataSet;
class     vtkDataSetRemoveGhostCells;


// ****************************************************************************
//  Class: avtGhostZoneFilter
//
//  Purpose:
//
//  Programmer: Kathleen Bonnell 
//  Creation:   May 1, 2001 
//
//  Modifications:
//
//    Kathleen Bonnell, Wed Sep 19 12:55:57 PDT 2001
//    Added string argument to Execute method in order to match new interface.
//
//    Hank Childs, Tue Sep 10 12:51:33 PDT 2002
//    Inherited from avtStreamer instead of avtDataTreeStreamer.  Re-worked
//    the paradigm for memory management.
//
// ****************************************************************************

class PIPELINE_API avtGhostZoneFilter : public avtStreamer
{
  public:
                         avtGhostZoneFilter() {;};
    virtual             ~avtGhostZoneFilter() {;};

    virtual const char  *GetType(void) { return "avtGhostZoneFilter"; };
    virtual const char  *GetDescription(void) 
                             { return "Removing ghost cells"; };

  protected:
    virtual vtkDataSet         *ExecuteData(vtkDataSet *, int, std::string);
    virtual void                RefashionDataObjectInfo(void);
};


#endif


