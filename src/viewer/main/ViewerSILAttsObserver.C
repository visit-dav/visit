// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                            ViewerSILAttsObserver.C                        //
// ************************************************************************* //

#include <ViewerSILAttsObserver.h>
#include <SILAttributes.h>

using std::string;

// ****************************************************************************
//  Method: ViewerSILAttsObserver constructor
//
//  Programmer: Jeremy Meredith
//  Creation:   April 22, 2004
//
//  Modifications:
//
// ****************************************************************************

ViewerSILAttsObserver::ViewerSILAttsObserver(Subject *subj,
                                             const string &h,
                                             const string &f)
    : Observer(subj), host(h), file(f)
{
}

// ****************************************************************************
//  Method: ViewerSILAttsObserver destructor
//
//  Programmer: Jeremy Meredith
//  Creation:   April 22, 2004
//
//  Modifications:
//
// ****************************************************************************

ViewerSILAttsObserver::~ViewerSILAttsObserver()
{
}

// ****************************************************************************
//  Method: ViewerSILAttsObserver::Update
//
//  Purpose:
//      Update SILAttributes in the subject.
//
//  Arguments:
//      s       The subject which contains the metadata information.
//
//  Programmer: Jeremy Meredith
//  Creation:   April 22, 2004
//
//  Modifications:
//
// ****************************************************************************

void
ViewerSILAttsObserver::Update(Subject *s)
{
    // Tell the ViewerSubject the meta data was updated
    SILAttributes *sa = (SILAttributes*)s;
    emit silAttsUpdated(host, file, sa);
}

