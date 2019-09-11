// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                    avtConnectedComponentsVolumeQuery.h                    //
// ************************************************************************* //

#ifndef AVT_CONN_COMPONENTS_VOLUME_QUERY_H
#define AVT_CONN_COMPONENTS_VOLUME_QUERY_H
#include <query_exports.h>

#include <avtDatasetQuery.h>
#include <avtConnComponentsQuery.h>

#include <vector>

class avtVMetricVolume;
class avtRevolvedVolume;

class vtkDataSet;


// ****************************************************************************
//  Class: avtConnComponentsVolumeQuery
//
//  Purpose:
//      Obtains the volume of each connected component. 
//      Requires a 3D or revolved volume dataset. 
//
//  Programmer: Cyrus Harrison
//  Creation:   February 8, 2007 
//
// ****************************************************************************

class QUERY_API avtConnComponentsVolumeQuery : public avtConnComponentsQuery
{
  public:
                                    avtConnComponentsVolumeQuery();
    virtual                        ~avtConnComponentsVolumeQuery();

    virtual const char             *GetType(void)
                                  { return "avtConnComponentsVolumeQuery"; };
    virtual const char             *GetDescription(void)
                                  { return "Finding per component volume."; };

  protected:

    avtRevolvedVolume              *revolvedVolumeFilter;
    avtVMetricVolume               *volumeFilter;

    std::vector<double>             volPerComp;

    virtual void                    Execute(vtkDataSet *, const int);
    virtual void                    PreExecute(void);
    virtual void                    PostExecute(void);
    virtual avtDataObject_p         ApplyFilters(avtDataObject_p);
    virtual void                    VerifyInput();
};


#endif



