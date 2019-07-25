// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#ifndef MOVIE_SEQUENCE_ROTATE_H
#define MOVIE_SEQUENCE_ROTATE_H

#include <MovieSequence.h>

// ****************************************************************************
// Class: MovieSequenceRotate
//
// Purpose:
//   Base class for canned movie rotations.
//
// Notes:      
//
// Programmer: Brad Whitlock
// Creation:   Tue Nov 7 17:24:26 PST 2006
//
// Modifications:
//   Brad Whitlock, Tue Apr  8 09:14:04 PDT 2008
//   Moved SequenceMenuName methods to the .C file.
//
// ****************************************************************************

class MovieSequenceRotate : public MovieSequence
{
public:
    MovieSequenceRotate();
    virtual ~MovieSequenceRotate();

    virtual int SubMenuIndex() const { return 2; }

    virtual QWidget *CreateUI();
    virtual void ReadUIValues(QWidget *ui, DataNode *node);
    virtual void WriteUIValues(QWidget *ui, DataNode *node);
};

//
// Different types of rotation sequences.
//

class MovieSequenceScreenRotationX : public MovieSequenceRotate
{
public:
    MovieSequenceScreenRotationX() : MovieSequenceRotate() { }
    virtual ~MovieSequenceScreenRotationX()                    { }

    virtual MovieSequence *NewInstance() const      { return new MovieSequenceScreenRotationX; }
    virtual int SequenceId() const                  { return 300; }
    virtual std::string SequenceName() const        { return "ScreenRotationX"; }
    virtual QString SequenceMenuName() const;
    virtual QPixmap Pixmap();
};

class MovieSequenceScreenRotationY : public MovieSequenceRotate
{
public:
    MovieSequenceScreenRotationY() : MovieSequenceRotate() { }
    virtual ~MovieSequenceScreenRotationY()                    { }

    virtual MovieSequence *NewInstance() const      { return new MovieSequenceScreenRotationY; }
    virtual int SequenceId() const                  { return 301; }
    virtual std::string SequenceName() const        { return "ScreenRotationY"; }
    virtual QString SequenceMenuName() const;
    virtual QPixmap Pixmap();
};

class MovieSequenceScreenRotationZ : public MovieSequenceRotate
{
public:
    MovieSequenceScreenRotationZ() : MovieSequenceRotate() { }
    virtual ~MovieSequenceScreenRotationZ()                    { }

    virtual MovieSequence *NewInstance() const      { return new MovieSequenceScreenRotationZ; }
    virtual int SequenceId() const                  { return 302; }
    virtual std::string SequenceName() const        { return "ScreenRotationZ"; }
    virtual QString SequenceMenuName() const;
    virtual QPixmap Pixmap();
};


class MovieSequenceRotationX : public MovieSequenceRotate
{
public:
    MovieSequenceRotationX() : MovieSequenceRotate() { }
    virtual ~MovieSequenceRotationX()                    { }

    virtual MovieSequence *NewInstance() const      { return new MovieSequenceRotationX; }
    virtual int SequenceId() const                  { return 303; }
    virtual std::string SequenceName() const        { return "RotationX"; }
    virtual QString SequenceMenuName() const;
    virtual QPixmap Pixmap();
};

class MovieSequenceRotationY : public MovieSequenceRotate
{
public:
    MovieSequenceRotationY() : MovieSequenceRotate() { }
    virtual ~MovieSequenceRotationY()                    { }

    virtual MovieSequence *NewInstance() const      { return new MovieSequenceRotationY; }
    virtual int SequenceId() const                  { return 304; }
    virtual std::string SequenceName() const        { return "RotationY"; }
    virtual QString SequenceMenuName() const;
    virtual QPixmap Pixmap();
};

class MovieSequenceRotationZ : public MovieSequenceRotate
{
public:
    MovieSequenceRotationZ() : MovieSequenceRotate() { }
    virtual ~MovieSequenceRotationZ()                    { }

    virtual MovieSequence *NewInstance() const      { return new MovieSequenceRotationZ; }
    virtual int SequenceId() const                  { return 305; }
    virtual std::string SequenceName() const        { return "RotationZ"; }
    virtual QString SequenceMenuName() const;
    virtual QPixmap Pixmap();
};

#endif

