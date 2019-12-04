// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                        avtPickByZoneQuery.h                               //
// ************************************************************************* //

#ifndef AVT_PICKBYZONE_QUERY_H
#define AVT_PICKBYZONE_QUERY_H
#include <query_exports.h>

#include <avtPickQuery.h>



// ****************************************************************************
//  Class: avtPickByZoneQuery
//
//  Purpose:
//    This query gathers information about a cell picked by the user. 
//
//  Programmer: Kathleen Bonnell 
//  Creation:   November 15, 2002 
//
//  Modifications:
//    Kathleen Bonnell, Tue Nov  8 10:45:43 PST 2005
//    Added avtDataAttributes arg to Preparation.
//
//    Alister Maguire, Wed Dec  4 11:41:24 MST 2019
//    Added SetInvTransform.
//    
// ****************************************************************************

class QUERY_API avtPickByZoneQuery : public avtPickQuery
{
  public:
                                    avtPickByZoneQuery();
    virtual                        ~avtPickByZoneQuery();


    virtual const char             *GetType(void)
                                             { return "avtPickByZoneQuery"; };

    virtual void                    SetTransform(const avtMatrix *m);
    virtual void                    SetInvTransform(const avtMatrix *m);

  protected:

    virtual void                    Execute(vtkDataSet *, const int);
    virtual void                    Preparation(const avtDataAttributes &);
};


#endif


