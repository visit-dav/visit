// ************************************************************************* //
//                           VisWindowException.h                            //
// ************************************************************************* //

#ifndef VIS_WINDOW_EXCEPTION_H
#define VIS_WINDOW_EXCEPTION_H
#include <avtexception_exports.h>


#include <VisItException.h>


// ****************************************************************************
//  Class: VisWindowException
//
//  Purpose:
//      A base exception type that all exceptions thrown by the VisWindow 
//      should be derived from.  This allows clients to the VisWindow to catch
//      this type without worrying about the specific exception thrown.
//
//  Programmer: Hank Childs
//  Creation:   June 15, 2000
//
// ****************************************************************************

class AVTEXCEPTION_API VisWindowException : public VisItException
{
  public:
                          ~VisWindowException() VISIT_THROW_NOTHING {;};
};


#endif


