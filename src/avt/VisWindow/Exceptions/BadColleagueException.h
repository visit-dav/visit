// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                          BadColleagueException.h                          //
// ************************************************************************* //

#ifndef BAD_COLLEAGUE_EXCEPTION_H
#define BAD_COLLEAGUE_EXCEPTION_H
#include <viswindow_exports.h>


#include <VisWindowException.h>


// ****************************************************************************
//  Class: BadColleagueException
//
//  Purpose:
//      The appropriate exception when colleague specified could not be found.
//
//  Programmer: Hank Childs
//  Creation:   June 8, 2000
//
// ****************************************************************************

class VISWINDOW_API2 BadColleagueException : public VisWindowException
{
  public:
                    BadColleagueException();
   virtual         ~BadColleagueException() VISIT_THROW_NOTHING {;};
};


#endif


