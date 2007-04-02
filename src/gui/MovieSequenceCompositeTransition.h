#ifndef MOVIE_SEQUENCE_COMPOSITE_TRANSITION_H
#define MOVIE_SEQUENCE_COMPOSITE_TRANSITION_H
/*****************************************************************************
*
* Copyright (c) 2000 - 2007, The Regents of the University of California
* Produced at the Lawrence Livermore National Laboratory
* All rights reserved.
*
* This file is part of VisIt. For details, see http://www.llnl.gov/visit/. The
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
*    documentation and/or materials provided with the distribution.
*  - Neither the name of the UC/LLNL nor  the names of its contributors may be
*    used to  endorse or  promote products derived from  this software without
*    specific prior written permission.
*
* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT  HOLDERS AND CONTRIBUTORS "AS IS"
* AND ANY EXPRESS OR  IMPLIED WARRANTIES, INCLUDING,  BUT NOT  LIMITED TO, THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND  FITNESS FOR A PARTICULAR  PURPOSE
* ARE  DISCLAIMED.  IN  NO  EVENT  SHALL  THE  REGENTS  OF  THE  UNIVERSITY OF
* CALIFORNIA, THE U.S.  DEPARTMENT  OF  ENERGY OR CONTRIBUTORS BE  LIABLE  FOR
* ANY  DIRECT,  INDIRECT,  INCIDENTAL,  SPECIAL,  EXEMPLARY,  OR CONSEQUENTIAL
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
    virtual std::string SequenceMenuName() const    { return "Fade in viewport"; }
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
    virtual std::string SequenceMenuName() const    { return "Fade out viewport"; }
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
    virtual std::string SequenceMenuName() const      { return "Slide in viewport left to right"; }
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
    virtual std::string SequenceMenuName() const      { return "Slide in viewport right to left"; }
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
    virtual std::string SequenceMenuName() const      { return "Slide in viewport top to bottom"; }
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
    virtual std::string SequenceMenuName() const      { return "Slide in viewport bottom to top"; }
    virtual QPixmap Pixmap();
};

#endif
