// ************************************************************************* //
//                         BadWindowModeException.h                          //
// ************************************************************************* //

#ifndef BAD_WINDOW_MODE_EXCEPTION_H
#define BAD_WINDOW_MODE_EXCEPTION_H
#include <avtexception_exports.h>

#include <VisWindowException.h>
#include <VisWindowTypes.h>


// ****************************************************************************
//  Method: BadWindowModeException
//
//  Purpose:
//      The exception that should be thrown when an unexpected window mode is
//      encountered.
//
//  Programmer: Hank Childs
//  Creation:   May 19, 2000
//
// ****************************************************************************

class AVTEXCEPTION_API BadWindowModeException : public VisWindowException
{
  public:
                    BadWindowModeException(WINDOW_MODE);
    virtual        ~BadWindowModeException() VISIT_THROW_NOTHING {;};
};


#endif


