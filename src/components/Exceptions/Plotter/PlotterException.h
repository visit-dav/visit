// ************************************************************************* //
//                              PlotterException.h                           //
// ************************************************************************* //

#ifndef PLOTTER_EXCEPTION_H
#define PLOTTER_EXCEPTION_H
#include <avtexception_exports.h>


#include <VisItException.h>


// ****************************************************************************
//  Class: PlotterException
//
//  Purpose:
//      A base class for all plotter exceptions.  Meant to make logic easier
//      on catches.
//
//  Programmer: Mark C. Miller 
//  Creation:   April 14, 2004 
//
// ****************************************************************************

class AVTEXCEPTION_API PlotterException : public VisItException
{
  public:
                         ~PlotterException() VISIT_THROW_NOTHING {;};
};


#endif


