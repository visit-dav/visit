// ************************************************************************* //
//                             InvalidColortableException.h                  //
// ************************************************************************* //

#ifndef INVALID_COLORTABLE_EXCEPTION_H
#define INVALID_COLORTABLE_EXCEPTION_H
#include <avtexception_exports.h>

#include <PlotterException.h>

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
//  Modifications:
//    Mark C. Miller, Wed Apr 14 18:18:30 PDT 2004
//    Modified to inherent from new PlotterException base class
//
// ****************************************************************************

class AVTEXCEPTION_API InvalidColortableException : public PlotterException 
{
  public:
                  InvalidColortableException(const std::string &ct);
    virtual      ~InvalidColortableException() VISIT_THROW_NOTHING {;};
};


#endif


