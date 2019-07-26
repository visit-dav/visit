// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                           ViewerCommandFromSimObserver.C                  //
// ************************************************************************* //

#include <ViewerCommandFromSimObserver.h>
#include <SimulationCommand.h>

using std::string;

// ****************************************************************************
//  Method: ViewerCommandFromSimObserver constructor
//
//  Programmer: Brad Whitlock
//  Creation:   Thu Jan 25 14:08:09 PST 2007
//
//  Modifications:
//
// ****************************************************************************

ViewerCommandFromSimObserver::ViewerCommandFromSimObserver(Subject *subj,
                                               const EngineKey &k,
                                               const string &f)
    : Observer(subj), key(k), file(f)
{
}

// ****************************************************************************
//  Method: ViewerCommandFromSimObserver destructor
//
//  Programmer: Brad Whitlock
//  Creation:   Thu Jan 25 14:08:09 PST 2007
//
//  Modifications:
//
// ****************************************************************************

ViewerCommandFromSimObserver::~ViewerCommandFromSimObserver()
{
}

// ****************************************************************************
//  Method: ViewerCommandFromSimObserver::Update
//
//  Purpose:
//      Update MetaData in the subject.
//
//  Arguments:
//      s       The subject which contains the metadata information.
//
//  Programmer: Brad Whitlock
//  Creation:   Thu Jan 25 14:08:09 PST 2007
//
//  Modifications:
//
// ****************************************************************************

void
ViewerCommandFromSimObserver::Update(Subject *s)
{
    // Tell the ViewerSubject the simulation command was updated
    SimulationCommand *command = (SimulationCommand*)s;
    emit execute(key, file, command->GetCommand());
}

