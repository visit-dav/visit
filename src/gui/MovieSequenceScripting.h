// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#ifndef MOVIE_SEQUENCE_SCRIPTING_H
#define MOVIE_SEQUENCE_SCRIPTING_H

#include <MovieSequence.h>

// ****************************************************************************
// Class: MovieSequenceScripting
//
// Purpose:
//   Movie sequence object that represents custom scripting.
//
// Notes:      
//
// Programmer: Brad Whitlock
// Creation:   Tue Nov 14 10:21:10 PDT 2006
//
// Modifications:
//   Brad Whitlock, Tue Apr  8 09:15:10 PDT 2008
//   Made SequenceMenuName return QString.
//
// ****************************************************************************

class MovieSequenceScripting : public MovieSequence
{
public:
    MovieSequenceScripting();
    virtual ~MovieSequenceScripting();

    // Creates a new instance of the movie sequence object.
    virtual MovieSequence *NewInstance() const;

    // Returns the sequence Id that will be used to identify the
    // type of sequence object this is.
    virtual int SequenceId() const;

    // Returns the name of the sequence as it will appear in the XML file.
    virtual std::string SequenceName() const;

    // Returns the name of the sequence as it will appear in menus.
    virtual QString SequenceMenuName() const;

    virtual bool ProvidesMenu() const;

    // Returns the pixmap associated with this sequence object type.
    virtual QPixmap Pixmap();

    // Returns whether the sequence supports a custom UI.
    virtual bool SupportsCustomUI() const;
};

#endif
