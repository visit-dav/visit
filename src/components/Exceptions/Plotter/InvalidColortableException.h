// ************************************************************************* //
//                             InvalidColortableException.h                  //
// ************************************************************************* //

#ifndef INVALID_COLORTABLE_EXCEPTION_H
#define INVALID_COLORTABLE_EXCEPTION_H
#include <avtexception_exports.h>

#include <VisItException.h>


// ****************************************************************************
//  Class: InvalidColortableException
//
//  Purpose:
//      The exception that should be thrown when a invalid color table
//      is specified.
//
//  Programmer: Brad Whitlock
//  Creation:   Fri Apr 25 10:22:30 PDT 2003
//
// ****************************************************************************

class AVTEXCEPTION_API InvalidColortableException : public VisItException
{
  public:
                  InvalidColortableException(const std::string &ct);
    virtual      ~InvalidColortableException() VISIT_THROW_NOTHING {;};
};


#endif


