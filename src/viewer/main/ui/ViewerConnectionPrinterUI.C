// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

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
