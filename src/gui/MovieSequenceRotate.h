#ifndef MOVIE_SEQUENCE_ROTATE_H
#define MOVIE_SEQUENCE_ROTATE_H
/*****************************************************************************
*
* Copyright (c) 2000 - 2009, Lawrence Livermore National Security, LLC
* Produced at the Lawrence Livermore National Laboratory
* LLNL-CODE-400124
* All rights reserved.
*
* This file is  part of VisIt. For  details, see https://visit.llnl.gov/.  The
* full copyright notice is contained in the file COPYRIGHT located at the root
* of the VisIt distribution or at http://www.llnl.gov/visit/copyright.html.
*
* Redistribution  and  use  in  source  and  binary  forms,  with  or  without
* modification, are permitted provided that the following conditions are met:
*
*  - Redistributions of  source code must  retain the above  copyright notice,
*    this list of conditions and the disclaimer below.
*  - Redistributions in binary form must reproduce the above copyright notice,
*    this  list of  conditions  and  the  disclaimer (as noted below)  in  the
*    documentation and/or other materials provided with the distribution.
*  - Neither the name of  the LLNS/LLNL nor the names of  its contributors may
*    be used to endorse or promote products derived from this software without
*    specific prior written permission.
*
* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT  HOLDERS AND CONTRIBUTORS "AS IS"
* AND ANY EXPRESS OR  IMPLIED WARRANTIES, INCLUDING,  BUT NOT  LIMITED TO, THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND  FITNESS FOR A PARTICULAR  PURPOSE
* ARE  DISCLAIMED. IN  NO EVENT  SHALL LAWRENCE  LIVERMORE NATIONAL  SECURITY,
* LLC, THE  U.S.  DEPARTMENT OF  ENERGY  OR  CONTRIBUTORS BE  LIABLE  FOR  ANY
* DIRECT,  INDIRECT,   INCIDENTAL,   SPECIAL,   EXEMPLARY,  OR   CONSEQUENTIAL
* DAMAGES (INCLUDING, BUT NOT  LIMITED TO, PROCUREMENT OF  SUBSTITUTE GOODS OR
* SERVICES; LOSS OF  USE, DATA, OR PROFITS; OR  BUSINESS INTERRUPTION) HOWEVER
* CAUSED  AND  ON  ANY  THEORY  OF  LIABILITY,  WHETHER  IN  CONTRACT,  STRICT
* LIABILITY, OR TORT  (INCLUDING NEGLIGENCE OR OTHERWISE)  ARISING IN ANY  WAY
* OUT OF THE  USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH
* DAMAGE.
*
*****************************************************************************/
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

