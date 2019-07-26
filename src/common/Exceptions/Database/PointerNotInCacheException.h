// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                         PointerNotInCacheException.h                      //
// ************************************************************************* //

#ifndef POINTER_NOT_IN_CACHE_EXCEPTION_H
#define POINTER_NOT_IN_CACHE_EXCEPTION_H
#include <avtexception_exports.h>

#include <DatabaseException.h>

// ****************************************************************************
//  Class: PointerNotInCacheException
//
//  Purpose:
//      The exception that should be thrown whenever transform manager is
//      unable to find items in variable cache given object pointer
//
//  Programmer: Mark C. Miller
//  Creation:   November 30, 2006 
//
// ****************************************************************************

class AVTEXCEPTION_API PointerNotInCacheException : public DatabaseException
{
  public:
                    PointerNotInCacheException(const void *);
    virtual        ~PointerNotInCacheException() VISIT_THROW_NOTHING {;};
};

#endif


