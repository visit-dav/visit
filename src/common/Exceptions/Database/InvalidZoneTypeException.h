// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                         InvalidZoneTypeException.h                        //
// ************************************************************************* //

#ifndef INVALID_ZONE_TYPE_EXCEPTION_H
#define INVALID_ZONE_TYPE_EXCEPTION_H
#include <avtexception_exports.h>


#include <DatabaseException.h>


// ****************************************************************************
//  Class: InvalidZoneTypeException
//
//  Purpose:
//      The exception thrown when the zone type desired is not supported by
//      VTK.
//
//  Programmer: Hank Childs
//  Creation:   August 15, 2000
//
// ****************************************************************************

class AVTEXCEPTION_API InvalidZoneTypeException : public DatabaseException
{
  public:
                          InvalidZoneTypeException(int);
    virtual              ~InvalidZoneTypeException() VISIT_THROW_NOTHING {;};
};


#endif


