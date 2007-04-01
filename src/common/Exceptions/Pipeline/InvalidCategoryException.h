// ************************************************************************* //
//                       InvalidCategoryException.h                      //
// ************************************************************************* //

#ifndef INVALID_CATEGORY_EXCEPTION_H
#define INVALID_CATEGORY_EXCEPTION_H
#include <avtexception_exports.h>


#include <PipelineException.h>


// ****************************************************************************
//  Class: InvalidCategoryException
//
//  Purpose:
//      The exception that should be called when an invalid SIL category is
//      encountered.
//
//  Programmer: Kathleen Bonnell 
//  Creation:   August 12, 2002 
//
// ****************************************************************************

class AVTEXCEPTION_API InvalidCategoryException : public PipelineException
{
  public:
                    InvalidCategoryException(const char *);
    virtual        ~InvalidCategoryException() VISIT_THROW_NOTHING {;};
};


#endif


