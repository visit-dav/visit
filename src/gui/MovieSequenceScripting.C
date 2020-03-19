// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#include <MovieSequenceScripting.h>
#include <icons/ms_scripting.xpm>

// ****************************************************************************
// Method: MovieSequenceScripting::MovieSequenceScripting
//
// Purpose: 
//   Constructor.
//
// Programmer: Brad Whitlock
// Creation:   Tue Nov 14 10:17:47 PDT 2006
//
// Modifications:
//   
// ****************************************************************************

MovieSequenceScripting::MovieSequenceScripting() : MovieSequence()
{
}

// ****************************************************************************
// Method: MovieSequenceScripting::~MovieSequenceScripting
//
// Purpose: 
//   Destructor.
//
// Programmer: Brad Whitlock
// Creation:   Tue Nov 14 10:18:02 PDT 2006
//
// Modifications:
//   
// ****************************************************************************

MovieSequenceScripting::~MovieSequenceScripting()
{
}

// ****************************************************************************
// Method: MovieSequenceScripting::NewInstance
//
// Purpose: 
//   Creates a new instance of the class.
//
// Returns:    A new instance of the class.
//
// Programmer: Brad Whitlock
// Creation:   Tue Nov 14 10:18:16 PDT 2006
//
// Modifications:
//   
// ****************************************************************************

MovieSequence *
MovieSequenceScripting::NewInstance() const
{
    return new MovieSequenceScripting;
}

// ****************************************************************************
// Method: MovieSequenceScripting::SequenceId
//
// Purpose: 
//   Returns the sequence id, which is an int that identifies the sequence type.
//
// Programmer: Brad Whitlock
// Creation:   Tue Nov 14 10:18:48 PDT 2006
//
// Modifications:
//   
// ****************************************************************************

int
MovieSequenceScripting::SequenceId() const
{
    return 1;
}

// ****************************************************************************
// Method: MovieSequenceScripting::SequenceName
//
// Purpose: 
//   Returns the sequence in as it appears in XML.
//
// Programmer: Brad Whitlock
// Creation:   Tue Nov 14 10:18:48 PDT 2006
//
// Modifications:
//   
// ****************************************************************************

std::string
MovieSequenceScripting::SequenceName() const
{
    return "Scripting";
}

// ****************************************************************************
// Method: MovieSequenceScripting::SequenceMenuName
//
// Purpose: 
//   Returns the name used for the sequuence in menus.
//
// Programmer: Brad Whitlock
// Creation:   Tue Nov 14 10:18:48 PDT 2006
//
// Modifications:
//   Brad Whitlock, Tue Apr  8 09:15:37 PDT 2008
//   Use tr() and return QString.
//
// ****************************************************************************

QString
MovieSequenceScripting::SequenceMenuName() const
{
    return tr("Scripting");
}

//
// Take it out of the menus for now.
//
bool
MovieSequenceScripting::ProvidesMenu() const
{
    return false;
}

// ****************************************************************************
// Method: MovieSequenceScripting::Pixmap
//
// Purpose: 
//   Returns the sequence's pixmap.
//
// Programmer: Brad Whitlock
// Creation:   Tue Nov 14 10:20:00 PDT 2006
//
// Modifications:
//   
// ****************************************************************************

QPixmap
MovieSequenceScripting::Pixmap()
{
    return QPixmap(ms_scripting_xpm);
}

// ****************************************************************************
// Method: MovieSequenceScripting::SupportsCustomUI
//
// Purpose: 
//   Tell VisIt that this sequence has custom user interfaces.
//
// Programmer: Brad Whitlock
// Creation:   Tue Nov 14 10:20:18 PDT 2006
//
// Modifications:
//   
// ****************************************************************************

bool
MovieSequenceScripting::SupportsCustomUI() const
{
    return true;
}

