/*****************************************************************************
*
* Copyright (c) 2000 - 2015, Lawrence Livermore National Security, LLC
* Produced at the Lawrence Livermore National Laboratory
* LLNL-CODE-442911
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
#include <ViewerConnectionPrinterUI.h>
#include <Connection.h>
#include <LostConnectionException.h>

#include <ViewerMessaging.h>
#include <ViewerText.h>

#include <QString>

#include <DebugStream.h>

// ****************************************************************************
// Method: ViewerConnectionPrinterUI::ViewerConnectionPrinterUI
//
// Purpose:
//   Constructor.
//
// Arguments:
//   c : The connection we'll read from.
//
// Programmer: Brad Whitlock
// Creation:   Sat Sep  6 01:23:42 PDT 2014
//
// Modifications:
//
// ****************************************************************************

ViewerConnectionPrinterUI::ViewerConnectionPrinterUI() : ViewerConnectionPrinter()
{
    impl = NULL;
}

// ****************************************************************************
// Method: ViewerConnectionPrinterUI::~ViewerConnectionPrinterUI
//
// Purpose:
//   Destructor.
//
// Programmer: Brad Whitlock
// Creation:   Sat Sep  6 01:24:06 PDT 2014
//
// Modifications:
//
// ****************************************************************************

ViewerConnectionPrinterUI::~ViewerConnectionPrinterUI()
{
    if(impl != NULL)
        delete impl;
}

// ****************************************************************************
// Method: ViewerConnectionPrinterUI::SetConnection
//
// Purpose:
//   Set the connection and create the internal helper that will print the
//   output from the connection's socket to the console.
//
// Arguments:
//   c : The connection we'll read.
//
// Programmer: Brad Whitlock
// Creation:   Sat Sep  6 01:31:55 PDT 2014
//
// Modifications:
//
// ****************************************************************************

void
ViewerConnectionPrinterUI::SetConnection(Connection *c)
{
    conn = c;
    impl = new ViewerConnectionPrinterImplementation(conn);
}

// ****************************************************************************
// Method: ViewerConnectionPrinterImplementation::ViewerConnectionPrinterImplementation
//
// Purpose: 
//   Constructor.
//
// Arguments:
//   conn : The connection associated with the socket.
//
// Programmer: Brad Whitlock
// Creation:   Wed Nov 21 15:22:42 PST 2007
//
// Modifications:
//   
// ****************************************************************************

ViewerConnectionPrinterImplementation::ViewerConnectionPrinterImplementation(Connection *c) : 
    QSocketNotifier(c->GetDescriptor(), QSocketNotifier::Read, 0)
{
    conn = c;
    connect(this, SIGNAL(activated(int)),
            this, SLOT(HandleRead(int)));
}

// ****************************************************************************
// Method: ViewerConnectionPrinterImplementation::~ViewerConnectionPrinterImplementation
//
// Purpose: 
//   Destructor.
//
// Programmer: Brad Whitlock
// Creation:   Wed Nov 21 15:23:38 PST 2007
//
// Modifications:
//   
// ****************************************************************************

ViewerConnectionPrinterImplementation::~ViewerConnectionPrinterImplementation()
{
}

// ****************************************************************************
// Method: ViewerConnectionPrinterImplementation::HandleRead
//
// Purpose: 
//   This is a Qt slot function that is called when the socket has data. The
//   data gets read and printed to the console.
//
// Programmer: Brad Whitlock
// Creation:   Wed Nov 21 15:23:52 PST 2007
//
// Modifications:
//   Brad Whitlock, Fri Jan 9 15:13:01 PST 2009
//   Catch the rest of the possible exceptions.
//
//   Mark C. Miller, Wed Jun 17 17:46:18 PDT 2009
//   Replaced CATCHALL(...) with CATCHALL
//
//   Brad Whitlock, Tue Jun 26 16:45:03 PDT 2012
//   Send output to the client too.
//
// ****************************************************************************

void
ViewerConnectionPrinterImplementation::HandleRead(int)
{
    TRY
    {
        // Fill up the connection from the socket.
        conn->Fill();

        QString msg;
        while(conn->Size() > 0)
        {
            unsigned char c;
            conn->Read(&c);
            msg.append(QChar(c));
        }

        // Print the output that we read to stdout.
        fprintf(stdout, "%s", msg.toStdString().c_str());
        fflush(stdout);

        // Send the message to the client too.
        while(msg.endsWith("\n"))
            msg.chop(1);
        int idx = msg.indexOf("WARNING: ");
        if(idx != -1)
        {
            QString warning(msg.right(msg.size() - (idx + 9)));
            ViewerBase::GetViewerMessaging()->Warning(warning.toStdString());
        }
        else
            ViewerBase::GetViewerMessaging()->Message(msg.toStdString());
    }
    CATCH(LostConnectionException)
    {
        debug1 << "Lost connection in ViewerConnectionPrinterImplementation::HandleRead" << endl;
    }
    CATCHALL
    {
        ; // nothing
    }
    ENDTRY
}
