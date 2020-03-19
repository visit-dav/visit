// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                            avtZonePickQuery.h                             //
// ************************************************************************* //

#ifndef AVT_ZONEPICK_QUERY_H
#define AVT_ZONEPICK_QUERY_H
#include <query_exports.h>

#include <avtPickQuery.h>

#include <string>
#include <PickAttributes.h>


// ****************************************************************************
//  Class: avtZonePickQuery
//
//  Purpose:
//    This query gathers information about a cell picked by the user. 
//
//  Programmer: Kathleen Bonnell 
//  Creation:   May 20, 2004 
//
//  Modifications:
//    Kathleen Bonnell, Tue Nov  8 10:45:43 PST 2005
//    Added avtDataAttributes arg to Preparation.
//
//    Alister Maguire, Tue Oct 22 14:20:27 MST 2019
//    Added SetTransform. 
//    
// ****************************************************************************

class QUERY_API avtZonePickQuery : public avtPickQuery
{
  public:
                                    avtZonePickQuery();
    virtual                        ~avtZonePickQuery();

    virtual const char             *GetType(void)
                                             { return "avtZonePickQuery"; };

    virtual void                    SetInvTransform(const avtMatrix *m);
    virtual void                    SetTransform(const avtMatrix *m);

  protected:
    virtual void                    Execute(vtkDataSet *, const int);
    virtual void                    Preparation(const avtDataAttributes &);
};


#endif


