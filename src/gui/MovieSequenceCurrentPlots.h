// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#ifndef MOVIE_SEQUENCE_CURRENT_PLOTS_H
#define MOVIE_SEQUENCE_CURRENT_PLOTS_H

#include <MovieSequence.h>

// ****************************************************************************
// Class: MovieSequenceCurrentPlots
//
// Purpose:
//   A movie sequence that tells the movie maker to use the plots from
//   the current window.
//
// Programmer: Brad Whitlock
// Creation:   Fri Oct 6 14:56:28 PST 2006
//
// Modifications:
//   Brad Whitlock, Tue Apr  8 09:07:31 PDT 2008
//   Changed SequenceMenuName so it returns QString.
//
// ****************************************************************************

class MovieSequenceCurrentPlots : public MovieSequence
{
public:
    MovieSequenceCurrentPlots();
    virtual ~MovieSequenceCurrentPlots();

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
    virtual bool ReadOnly() const;

    // Returns the pixmap associated with this sequence object type.
    virtual QPixmap Pixmap();
};

#endif
