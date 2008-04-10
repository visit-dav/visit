#ifndef MOVIE_SEQUENCE_SCRIPTING_H
#define MOVIE_SEQUENCE_SCRIPTING_H
/*****************************************************************************
*
* Copyright (c) 2000 - 2008, Lawrence Livermore National Security, LLC
* Produced at the Lawrence Livermore National Laboratory
* LLNL-CODE-400142
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
