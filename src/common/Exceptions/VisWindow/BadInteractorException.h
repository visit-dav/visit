// ************************************************************************* //
//                        BadInteractorException.h                           //
// ************************************************************************* //

#ifndef BAD_INTERACTOR_EXCEPTION_H
#define BAD_INTERACTOR_EXCEPTION_H
#include <avtexception_exports.h>

#include <VisWindowException.h>
#include <VisWindowTypes.h>


// ****************************************************************************
//  Method: BadInteractorException
//
//  Purpose:
//      The exception thrown when no interactor seems appropriate given
//      the current mode.
//
//  Programmer: Hank Childs
//  Creation:   May 19, 2000
//
// ****************************************************************************

class AVTEXCEPTION_API BadInteractorException : public VisWindowException
{
  public:
                    BadInteractorException(INTERACTION_MODE);
    virtual        ~BadInteractorException() VISIT_THROW_NOTHING {;};
};


#endif


