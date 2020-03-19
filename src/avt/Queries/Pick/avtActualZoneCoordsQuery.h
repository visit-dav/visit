// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                       avtActualZoneCoordsQuery.h                          //
// ************************************************************************* //

#ifndef AVT_ACTUAL_ZONECOORDS_QUERY_H
#define AVT_ACTUAL_ZONECOORDS_QUERY_H
#include <query_exports.h>

#include <avtActualCoordsQuery.h>



// ****************************************************************************
//  Class: avtActualZoneCoordsQuery
//
//  Purpose:
//      This query determines the actual coordinates (center) for a given zone.
//
//  Programmer: Kathleen Bonnell 
//  Creation:   June 2, 2004 
//
//  Modifications:
//
// ****************************************************************************

class QUERY_API avtActualZoneCoordsQuery : public avtActualCoordsQuery
{
  public:
                                    avtActualZoneCoordsQuery();
    virtual                        ~avtActualZoneCoordsQuery();

    virtual const char             *GetType(void)
                                             { return "avtActualZoneCoordsQuery"; };
    virtual const char             *GetDescription(void)
                                             { return "Locating zone coords."; };

  protected:
    virtual void                    Execute(vtkDataSet *, const int);
};


#endif


