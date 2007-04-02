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

#ifndef XFER_H
#define XFER_H
#include <state_exports.h>
#include <SimpleObserver.h>
#include <BufferConnection.h>
#include <vector>

class AttributeSubject;
class Connection;
class VisItRPC;

// ****************************************************************************
//  Class: Xfer
//
//  Purpose:
//    Observes multiple AttributeSubject objects. This class is
//    responsible for writing AttributeSubject objects onto a Connection.
//
//  Programmer: Brad Whitlock
//  Creation:   Mon Jun 5 14:43:41 PST 2000
//
//  Modifications:
//    Brad Whitlock, Thu Aug 31 15:26:28 PST 2000
//    Added SubjectRemoved.
//
//    Jeremy Meredith, Tue Jul  3 15:02:57 PDT 2001
//    Added ReadPendingMessages and SendInterruption. 
//    Added buffered input.
//
//    Brad Whitlock, Fri Mar 22 16:47:23 PST 2002
//    Added a debugging function called ListObjects.
//
//    Brad Whitlock, Thu Feb 27 10:57:26 PDT 2003
//    Added support for special opcodes, which are opcodes that the user
//    can inject into the command stream to cause immediate effects.
//
//    Brad Whitlock, Thu Apr 14 16:37:24 PST 2005
//    Added a method to give access to the buffered input so we can store
//    objects directly in it.
//
// ****************************************************************************

class STATE_API Xfer : public SimpleObserver
{
public:
    Xfer();
    virtual ~Xfer();
    virtual void Update(Subject *);
    virtual void SubjectRemoved(Subject *);
    void Add(AttributeSubject *);
    void Add(VisItRPC *);
    void Remove(AttributeSubject *);
    void SetInputConnection(Connection *);
    void SetOutputConnection(Connection *);
    Connection *GetInputConnection() const;
    Connection *GetOutputConnection() const;
    Connection *GetBufferedInputConnection();

    virtual void Process();
    virtual bool ReadPendingMessages();
    virtual void SendInterruption();

    int  CreateNewSpecialOpcode();
    void SendSpecialOpcode(int opcode);
    void SetupSpecialOpcodeHandler(void (*cb)(int, void *), void *data);

    void ListObjects();
protected:
    bool ReadHeader();
    int  GetNextGuido();
protected:
    std::vector<AttributeSubject *> subjectList;
    Connection *input, *output;
    BufferConnection bufferedInput;

    void (*specialOpcodeCallback)(int, void *);
    void *specialOpcodeCallbackData;

    // These deal with the input connection.
    int opcode, length;
    bool haveStoredHeader;

    // global unique identifier for object (guido)
    int nextGuido;
    int nextSpecialOpcode;
};

#endif
