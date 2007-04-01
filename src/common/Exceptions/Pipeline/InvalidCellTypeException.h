// ************************************************************************* //
//                         InvalidCellTypeException.h                        //
// ************************************************************************* //

#ifndef INVALID_CELL_TYPE_EXCEPTION_H
#define INVALID_CELL_TYPE_EXCEPTION_H
#include <avtexception_exports.h>


#include <PipelineException.h>


// ****************************************************************************
//  Class: InvalidCellTypeException
//
//  Purpose:
//      The exception that should be called when an unknown cell type is
//      encountered.
//
//  Programmer: Hank Childs
//  Creation:   December 12, 2000
//
//  Modifications:
//
//    Hank Childs, Mon Apr 15 15:31:53 PDT 2002
//    Make a more descriptive message when the exception is thrown.
//
// ****************************************************************************

class AVTEXCEPTION_API InvalidCellTypeException : public PipelineException
{
  public:
                    InvalidCellTypeException(int);
                    InvalidCellTypeException(const char *);
    virtual        ~InvalidCellTypeException() VISIT_THROW_NOTHING {;};
};


#endif


