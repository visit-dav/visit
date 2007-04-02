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

#include <QvisInterpreter.h>
#include <qapplication.h>
#include <qtimer.h>
#include <vectortypes.h>
#include <ViewerProxy.h>
#include <ClientMethod.h>
#include <ClientInformation.h>
#include <ClientInformationList.h>
#include <DebugStream.h>
#include <Utility.h>

// ****************************************************************************
// Method: QvisInterpreter::QvisInterpreter
//
// Purpose: 
//   Constructor for the QvisInterpreter class.
//
// Programmer: Brad Whitlock
// Creation:   Mon May 9 10:00:08 PDT 2005
//
// Modifications:
//   
// ****************************************************************************

QvisInterpreter::QvisInterpreter(QObject *parent, const char *name) :
    QObject(parent, name), GUIBase(), pendingCommands()
{
}

// ****************************************************************************
// Method: QvisInterpreter::~QvisInterpreter
//
// Purpose: 
//   Destructor for the QvisInterpreter class.
//
// Programmer: Brad Whitlock
// Creation:   Mon May 9 10:00:24 PDT 2005
//
// Modifications:
//   
// ****************************************************************************

QvisInterpreter::~QvisInterpreter()
{
}

// ****************************************************************************
// Method: QvisInterpreter::Interpret
//
// Purpose: 
//   Adds a string to interpret to the list of pending commands and tells the
//   interpreter to process the commands.
//
// Arguments:
//   s : The string to interpret.
//
// Note:       This method might result in an interpreter launch with the
//             code being interpreted only once the interpreter is ready.
//
// Programmer: Brad Whitlock
// Creation:   Mon May 9 10:00:42 PDT 2005
//
// Modifications:
//   
// ****************************************************************************

void
QvisInterpreter::Interpret(const QString &s)
{
    std::string code(s.latin1());

    // Add the code to the pending commands.
    if(code.size() > 0)
    {
        // Make sure that the code has a newline as the last character.
        if(code[code.size()-1] != '\n')
            code += "\n";

        pendingCommands.push_back(code);
    }
    else
    {
        Message("You told VisIt to execute an empty body of code "
                "so VisIt did nothing.");
    }

    if(Initialize())
        emit Synchronize();
    else
        ProcessCommands();
}

// ****************************************************************************
// Method: QvisInterpreter::ProcessCommands
//
// Purpose: 
//   This is a Qt slot function that is executed when the interpreter has
//   code to execute.
//
// Note:       This method loops over the pending commands and makes client
//             method calls to any client that can service an "Interpret"
//             method.
//
// Programmer: Brad Whitlock
// Creation:   Mon May 9 10:02:10 PDT 2005
//
// Modifications:
//   Brad Whitlock, Mon Jul 11 09:48:06 PDT 2005
//   Wrote interpreted code to debug2.
//
// ****************************************************************************

void
QvisInterpreter::ProcessCommands()
{
    if(NoInterpretingClient())
    {
        // If there's still no interpreting client, sync again.
        emit Synchronize();
    }
    else
    {
        while(pendingCommands.size() > 0)
        {
            // Get the code from the front.
            std::string code(pendingCommands.front());
            pendingCommands.pop_front();
        
            debug2 << "Interpreting code:\n" << code.c_str() << endl;

            // Relay an "Interpret" method command to the viewer where it should
            // be routed to the CLI.
            ClientMethod *method = GetViewerState()->GetClientMethod();
            method->ClearArgs();
            method->AddArgument(code);
            method->SetMethodName("Interpret");
            method->Notify();
        }
    }
}

// ****************************************************************************
// Method: QvisInterpreter::Initialize
//
// Purpose: 
//   This method tells the viewer to launch a CLI if no currently connected
//   clients support the "Interpret" method in their interface.
//
// Programmer: Brad Whitlock
// Creation:   Mon May 9 10:04:27 PDT 2005
//
// Modifications:
//   Brad Whitlock, Thu Jul 14 10:56:26 PDT 2005
//   I made it use GetVisItLauncher.
//
// ****************************************************************************

bool
QvisInterpreter::Initialize()
{
    bool launched = false;

    // look for "CLI" in the list of clients. If we don't find it then
    // launch it or, for now, tell the viewer to launch it.
    if(NoInterpretingClient())
    {
        debug1 << "There is no interpreting client. Tell the viewer to "
                  "create a CLI so we can execute code." << endl;
        stringVector args;
        args.push_back("-cli");
        args.push_back("-newconsole");
        GetViewerMethods()->OpenClient("CLI", GetVisItLauncher(), args);
        launched = true;
    }

    return launched;
}

// ****************************************************************************
// Method: QvisInterpreter::NoInterpretingClient
//
// Purpose: 
//   Checks to see if any of the current clients have an "Interpret" client
//   method that takes a string argument.
//
// Returns:    True if no clients support "Interpret"; False otherwise.
//
// Programmer: Brad Whitlock
// Creation:   Mon May 9 10:05:27 PDT 2005
//
// Modifications:
//   
// ****************************************************************************

bool
QvisInterpreter::NoInterpretingClient() const
{
    bool retval = true;

    //
    // Check all of the client information until we find a client that
    // supports the Interpret method with a string argument.
    //
    const ClientInformationList *cL = GetViewerState()->GetClientInformationList();
    for(int i = 0; i < cL->GetNumClientInformations(); ++i)
    {
        const ClientInformation &client = cL->operator[](i);
        int interpretIndex = client.GetMethodIndex("Interpret");
        if(interpretIndex != -1)
        {
            if(client.GetMethodPrototype(interpretIndex) == "s")
            {
                retval = false;
                break;
            }
        }
    }

    return retval;
}
