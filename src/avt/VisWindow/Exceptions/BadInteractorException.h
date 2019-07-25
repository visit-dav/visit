// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                        BadInteractorException.h                           //
// ************************************************************************* //

#ifndef BAD_INTERACTOR_EXCEPTION_H
#define BAD_INTERACTOR_EXCEPTION_H
#include <viswindow_exports.h>

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

class VISWINDOW_API2 BadInteractorException : public VisWindowException
{
  public:
                    BadInteractorException(INTERACTION_MODE);
    virtual        ~BadInteractorException() VISIT_THROW_NOTHING {;};
};


#endif


