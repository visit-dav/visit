// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#ifndef MOVIE_SEQUENCE_COMPOSITE_TRANSITION_H
#define MOVIE_SEQUENCE_COMPOSITE_TRANSITION_H

#include <MovieSequence.h>

// ****************************************************************************
// Class: MovieSequenceCompositeTransition
//
// Purpose:
//   Base class for movie transitions.
//
// Notes:
//
// Programmer: Brad Whitlock
// Creation:   Mon Oct 16 10:40:43 PDT 2006
//
// Modifications:
//   Brad Whitlock, Tue Apr  8 09:03:10 PDT 2008
//   Made SequenceMenuName return QString and moved the implementation to the
//   .C file so we can more easily internationalize.
//
// ****************************************************************************

class MovieSequenceCompositeTransition : public MovieSequence
{
public:
    MovieSequenceCompositeTransition();
    virtual ~MovieSequenceCompositeTransition();

    virtual int SubMenuIndex() const { return 1; }

    virtual QWidget *CreateUI();
    virtual void ReadUIValues(QWidget *ui, DataNode *node);
    virtual void WriteUIValues(QWidget *ui, DataNode *node);
};

//
// Types of viewport fade transitions.
//

class MovieSequenceCompositeFadeIn : public MovieSequenceCompositeTransition
{
public:
    MovieSequenceCompositeFadeIn() : MovieSequenceCompositeTransition() { }
    virtual ~MovieSequenceCompositeFadeIn()                    { }

    virtual MovieSequence *NewInstance() const      { return new MovieSequenceCompositeFadeIn; }
    virtual int SequenceId() const                  { return 200; }
    virtual std::string SequenceName() const        { return "CompositeFadeIn"; }
    virtual QString SequenceMenuName() const;
    virtual QPixmap Pixmap();
};

class MovieSequenceCompositeFadeOut : public MovieSequenceCompositeTransition
{
public:
    MovieSequenceCompositeFadeOut() : MovieSequenceCompositeTransition() { }
    virtual ~MovieSequenceCompositeFadeOut()                    { }

    virtual MovieSequence *NewInstance() const      { return new MovieSequenceCompositeFadeOut; }
    virtual int SequenceId() const                  { return 201; }
    virtual std::string SequenceName() const        { return "CompositeFadeOut"; }
    virtual QString SequenceMenuName() const;
    virtual QPixmap Pixmap();
};


//
// Types of slide viewport transitions.
//

class MovieSequenceCompositeLRSlide : public MovieSequenceCompositeTransition
{
public:
    MovieSequenceCompositeLRSlide() : MovieSequenceCompositeTransition() { }
    virtual ~MovieSequenceCompositeLRSlide()                    { }

    virtual MovieSequence *NewInstance() const        { return new MovieSequenceCompositeLRSlide; }
    virtual int SequenceId() const                    { return 202; }
    virtual std::string SequenceName() const          { return "CompositeLRSlide"; }
    virtual QString SequenceMenuName() const;
    virtual QPixmap Pixmap();
};

class MovieSequenceCompositeRLSlide : public MovieSequenceCompositeTransition
{
public:
    MovieSequenceCompositeRLSlide() : MovieSequenceCompositeTransition() { }
    virtual ~MovieSequenceCompositeRLSlide()                    { }

    virtual MovieSequence *NewInstance() const        { return new MovieSequenceCompositeRLSlide; }
    virtual int SequenceId() const                    { return 203; }
    virtual std::string SequenceName() const          { return "CompositeRLSlide"; }
    virtual QString SequenceMenuName() const;
    virtual QPixmap Pixmap();
};

class MovieSequenceCompositeTBSlide : public MovieSequenceCompositeTransition
{
public:
    MovieSequenceCompositeTBSlide() : MovieSequenceCompositeTransition() { }
    virtual ~MovieSequenceCompositeTBSlide()                    { }

    virtual MovieSequence *NewInstance() const        { return new MovieSequenceCompositeTBSlide; }
    virtual int SequenceId() const                    { return 204; }
    virtual std::string SequenceName() const          { return "CompositeTBSlide"; }
    virtual QString SequenceMenuName() const;
    virtual QPixmap Pixmap();
};

class MovieSequenceCompositeBTSlide : public MovieSequenceCompositeTransition
{
public:
    MovieSequenceCompositeBTSlide() : MovieSequenceCompositeTransition() { }
    virtual ~MovieSequenceCompositeBTSlide()                    { }

    virtual MovieSequence *NewInstance() const        { return new MovieSequenceCompositeBTSlide; }
    virtual int SequenceId() const                    { return 205; }
    virtual std::string SequenceName() const          { return "CompositeBTSlide"; }
    virtual QString SequenceMenuName() const;
    virtual QPixmap Pixmap();
};

#endif
