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

#include <ViewerMessageBuffer.h>

// ****************************************************************************
//  Method: ViewerMessageBuffer constructor
//
//  Programmer: Eric Brugger
//  Creation:   October 5, 2000
//
//  Modifications:
//
// ****************************************************************************

ViewerMessageBuffer::ViewerMessageBuffer()
{
    head = 0;
    tail = 0;
    size = 512;
    mask = size - 1;
    chrs = new char[size];
}

// ****************************************************************************
//  Method: ViewerMessageBuffer destructor
//
//  Programmer: Eric Brugger
//  Creation:   October 5, 2000
//
//  Modifications:
//
// ****************************************************************************

ViewerMessageBuffer::~ViewerMessageBuffer()
{
    delete [] chrs;
}

// ****************************************************************************
//  Method: ViewerMessageBuffer::Grow
//
//  Purpose:
//      Increase the size of the message buffer.
//
//  Returns:    The new head of the buffer.
//
//  Programmer: Eric Brugger
//  Creation:   October 5, 2000
//
//  Modifications:
//
// ****************************************************************************

int ViewerMessageBuffer::Grow()
{
    //
    // Create a new, larger buffer.
    //
    int       newSize, newMask;
    char     *newChrs=0;

    newSize = size * 4;
    newMask = newSize - 1;
    newChrs = new char[newSize];

    //
    // Copy the old array into the new.  Simply start at the head and
    // copy to the tail mod'ing appropriately.
    //
    int       i, j;

    i = tail;
    newChrs[0] = chrs[i];
    i = (i + 1) & mask;
    for (j=1; i != tail; i = (i+1)&mask, ++j)
    {
        newChrs[j] = chrs[i];
    }

    //
    // Store the new buffer information back into the buffer.
    //
    size = newSize;
    mask = newMask;
    delete [] chrs;
    chrs = newChrs;
    tail = 0;
    head = j;

    return head;
}

// ****************************************************************************
//  Method: ViewerMessageBuffer::AddString
//
//  Purpose:
//      Add a string to the message buffer.
//
//  Arguments:
//      str     The string to add to the buffer.
//
//  Programmer: Eric Brugger
//  Creation:   October 5, 2000
//
//  Modifications:
//
// ****************************************************************************

void ViewerMessageBuffer::AddString(const char *str)
{
    //
    // Return if we have an empty message.
    //
    if (str == 0 || str[0] == '\0')
    {
        return;
    }

    //
    // Assume at the start that a head pointing to the same spot as
    // the tail is okay.  If this situation exists at the end,
    // however, we need to allocate more space.
    //
    int   i, j;

    i = head;
    chrs[i] = str[0];
    i = (i + 1) & mask;
    for (j = 1; str[j] != '\0'; j++)
    {
        if(i == tail)
        {
            i = Grow();
        }

        chrs[i] = str[j];
        i = (i + 1) & mask;
    }
    head = i;

    //
    // If the head of the buffer points to the tail  of the buffer,
    // then we need to allocate more space.
    //
    if(head == tail)
    {
        head = Grow();
    }
}

// ****************************************************************************
//  Method: ViewerMessageBuffer::ReadMessage
//
//  Purpose:
//      Fill the user supplied buffer with the next ';' terminated message.
//
//  Arguments:
//      message  The buffer to fill with the message.
//
//  Returns:    The number of characters in the message excluding the null
//              terminating character.
//
//  Programmer: Eric Brugger
//  Creation:   October 5, 2000
//
//  Modifications:
//
// ****************************************************************************

int ViewerMessageBuffer::ReadMessage(char *message)
{
    //
    // Keep searching for ";" terminated commands, parsing them as we
    // go along.
    //
    int       i, j;

    j = 0;
    for (i = tail; i != head; i = (i + 1) & mask) 
    {
        message[j++] = chrs[i];
        if (chrs[i] == ';')
        {
            tail = (i + 1) & mask;
            message[j] = '\0';
            return j;
        }
    }

    return 0;
}
