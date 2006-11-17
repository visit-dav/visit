/*****************************************************************************
*
* Copyright (c) 2000 - 2006, The Regents of the University of California
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
//   
// ****************************************************************************

std::string
MovieSequenceScripting::SequenceMenuName() const
{
    return "Scripting";
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

