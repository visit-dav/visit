// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#include <MovieSequenceCurrentPlots.h>
#include <icons/blankwindow.xpm>

// ****************************************************************************
// Method: MovieSequenceCurrentPlots::MovieSequenceCurrentPlots
//
// Purpose: 
//   Constructor
//
// Programmer: Brad Whitlock
// Creation:   Tue Nov 14 10:28:13 PDT 2006
//
// Modifications:
//   
// ****************************************************************************

MovieSequenceCurrentPlots::MovieSequenceCurrentPlots() : MovieSequence()
{
}

// ****************************************************************************
// Method: MovieSequenceCurrentPlots::~MovieSequenceCurrentPlots
//
// Purpose: 
//   Destructor
//
// Programmer: Brad Whitlock
// Creation:   Tue Nov 14 10:28:13 PDT 2006
//
// Modifications:
//   
// ****************************************************************************

MovieSequenceCurrentPlots::~MovieSequenceCurrentPlots()
{
}

// ****************************************************************************
// Method: MovieSequenceCurrentPlots::NewInstance
//
// Purpose: 
//   Creates a new instance of the class.
//
// Returns:    A new instance of the class.
//
// Programmer: Brad Whitlock
// Creation:   Tue Nov 14 10:28:36 PDT 2006
//
// Modifications:
//   
// ****************************************************************************

MovieSequence *
MovieSequenceCurrentPlots::NewInstance() const
{
    return new MovieSequenceCurrentPlots;
}

//
// Other methods.
//

int
MovieSequenceCurrentPlots::SequenceId() const
{
    return 0;
}

std::string
MovieSequenceCurrentPlots::SequenceName() const
{
    return "CurrentPlots";
}

QString
MovieSequenceCurrentPlots::SequenceMenuName() const
{
    return tr("Current plots");
}

bool
MovieSequenceCurrentPlots::ProvidesMenu() const
{
    return false;
}

bool
MovieSequenceCurrentPlots::ReadOnly() const
{
    return true;
}

QPixmap
MovieSequenceCurrentPlots::Pixmap()
{
    return QPixmap(blankwindow_xpm);
}

