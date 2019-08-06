// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                        PointerNotInCacheException.C                       //
// ************************************************************************* //


#include <PointerNotInCacheException.h>


// ****************************************************************************
//  Method: PointerNotInCacheException constructor
//
//  Programmer: Mark C. Miller 
//  Creation:   November 30, 2006 
//
//  Modifications:
//    Jeremy Meredith, Thu Aug  7 16:19:35 EDT 2008
//    Use %p format for pointers, not %x.
//
// ****************************************************************************

PointerNotInCacheException::PointerNotInCacheException(const void *p)
{
    char str[1024];
    snprintf(str, sizeof(str), "Unable to find object with pointer %p in cache", p);
    msg = str;
}
