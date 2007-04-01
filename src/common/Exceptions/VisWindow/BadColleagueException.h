// ************************************************************************* //
//                          BadColleagueException.h                          //
// ************************************************************************* //

#ifndef BAD_COLLEAGUE_EXCEPTION_H
#define BAD_COLLEAGUE_EXCEPTION_H
#include <avtexception_exports.h>


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

class AVTEXCEPTION_API BadColleagueException : public VisWindowException
{
  public:
                    BadColleagueException();
   virtual         ~BadColleagueException() VISIT_THROW_NOTHING {;};
};


#endif


