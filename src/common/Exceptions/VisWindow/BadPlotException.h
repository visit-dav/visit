// ************************************************************************* //
//                             BadPlotException.h                            //
// ************************************************************************* //

#ifndef BAD_PLOT_EXCEPTION_H
#define BAD_PLOT_EXCEPTION_H
#include <avtexception_exports.h>

#include <VisWindowException.h>


// ****************************************************************************
//  Class: PlotDimensionalityException
//
//  Purpose:
//      The exception that should be thrown when a bad plot is specified.
//
//  Programmer: Hank Childs
//  Creation:   May 17, 2000
//
// ****************************************************************************

class AVTEXCEPTION_API BadPlotException : public VisWindowException
{
  public:
                    BadPlotException();
    virtual        ~BadPlotException() VISIT_THROW_NOTHING {;};
};


#endif


