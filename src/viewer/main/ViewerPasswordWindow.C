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

#include <visit-config.h>
#include "ViewerPasswordWindow.h"
#include <ViewerConnectionProgressDialog.h>

#include <ChangeUsernameException.h>
#include <CouldNotConnectException.h>
#include <CancelledConnectException.h>

#include <qlayout.h>
#include <qlineedit.h>
#include <qpushbutton.h>
#include <qlabel.h>
#include <qtimer.h>

#if !defined(_WIN32)
#include <unistd.h>
#include <ViewerSubject.h>
extern ViewerSubject *viewerSubject;
#endif

// Static members
ViewerPasswordWindow *ViewerPasswordWindow::instance = NULL;
ViewerConnectionProgressDialog *ViewerPasswordWindow::dialog = NULL;
#if defined(PANTHERHACK)
// Broken on Panther
#else
std::set<int> ViewerPasswordWindow::failedPortForwards;
#endif

// ****************************************************************************
//  Constructor:  ViewerPasswordWindow::ViewerPasswordWindow
//
//  Programmer:  Jeremy Meredith
//  Creation:    April 26, 2001
//
//  Modifications:
//    Brad Whitlock, Mon Apr 15 11:22:44 PDT 2002
//    Added an ok button.
//
//    Jeremy Meredith. Tue Dec  9 15:16:52 PST 2003
//    Added a cancel button, as well as supported Rejected functionality.
//
//    Brad Whitlock, Mon Feb 23 15:07:41 PST 2004
//    Added space between the password line edit and the cancel button.
//
//    Hank Childs, Sun Nov 11 22:21:55 PST 2007
//    Add support for changing the username.
//
// ****************************************************************************

ViewerPasswordWindow::ViewerPasswordWindow(QWidget *parent, const char *name)
    : QDialog(parent, name, true)
{
    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->setMargin(10);

    QHBoxLayout *l2 = new QHBoxLayout(layout);
    l2->setSpacing(5);
    label = new QLabel("Password for localhost: ", this);
    l2->addWidget(label);

    passedit = new QLineEdit(this);
    passedit->setEchoMode(QLineEdit::Password);
    l2->addWidget(passedit);
    connect(passedit, SIGNAL(returnPressed()), this, SLOT(accept()));
    layout->addSpacing(20);

    QHBoxLayout *l3 = new QHBoxLayout(layout);
    QPushButton *okay = new QPushButton("OK", this, "OK");
    connect(okay, SIGNAL(clicked()), this, SLOT(accept()));
    l3->addWidget(okay);
    l3->addStretch(10);

    QPushButton *cub = new QPushButton("Change username", this, 
                                                  "changeUsernameButton");
    connect(cub, SIGNAL(clicked()), this, SLOT(changeUsername()));
    l3->addWidget(cub);
    l3->addStretch(10);

    QPushButton *cancel = new QPushButton("Cancel", this, "Cancel");
    connect(cancel, SIGNAL(clicked()), this, SLOT(reject()));
    l3->addWidget(cancel);

    setCaption("Enter password");
}

// ****************************************************************************
//  Destructor:  ViewerPasswordWindow::~ViewerPasswordWindow
//
//  Programmer:  Jeremy Meredith
//  Creation:    April 26, 2001
//
// ****************************************************************************

ViewerPasswordWindow::~ViewerPasswordWindow()
{
}

// ****************************************************************************
//  Method:  ViewerPasswordWindow::authenticate
//
//  Purpose:
//    Get the password.
//
//  Arguments:
//    username : The user's login name.
//    host     : the host name
//    fd       : the file descriptor
//
//  Programmer:  Jeremy Meredith
//  Creation:    April 26, 2001
//
//  Modifications:
//    Jeremy Meredith, Thu Apr 11 10:48:37 PDT 2002
//    Made it clear the password text before re-displaying.  This is
//    to help prevent difficulties with ssh, as well as one-time passwords.
//    Also, made it automatically answer "yes" to attempts to connect to
//    an unknown host.
//
//    Brad Whitlock, Mon Apr 15 12:16:46 PDT 2002
//    Added code to give focus to the password window.
//
//    Brad Whitlock, Tue Apr 16 13:02:09 PST 2002
//    I made it use the new getPassword method.
//
//    Brad Whitlock, Mon Sep 30 14:35:53 PST 2002
//    I added code to set the timeout on a connection progress dialog.
//
//    Jeremy Meredith, Mon May 19 17:37:36 PDT 2003
//    Added code to allow a passphrase instead of just a password.
//
//    Jeremy Meredith, Tue Dec  9 15:01:09 PST 2003
//    Made search for visit's "Running" message more specific; it turns
//    out a similar message was output by some versions of SSH in 
//    other circumstances.  Also check for NULL return values from 
//    getPassword since these can now occur when the user cancels or
//    closes the password window.  Also, changed the size of the read
//    buffer to be more reasonable.
//
//    Brad Whitlock, Mon Feb 23 15:12:27 PST 2004
//    I changed the code so it uses CancelledConnectException when the
//    password window returns a NULL password since the only way to do that
//    is to click the Cancel button. This changes the message that is seen
//    after the launch is cancelled.
//
//    Jeremy Meredith, Tue Mar 22 08:42:14 PST 2005
//    Added a new parsed message ('6090).
//
//    Hank Childs, Thu Jan 26 09:55:31 PST 2006
//    Add support for "Enter PASSCODE:". ('6946)
//
//    Jeremy Meredith, Thu May 24 10:57:02 EDT 2007
//    Added support for checking failed port forward messages.
//
//    Thomas R. Treadway, Mon Oct  8 13:27:42 PDT 2007
//    Backing out SSH tunneling on Panther (MacOS X 10.3)
//
//    Hank Childs, Sun Nov 11 22:21:55 PST 2007
//    Add support for changing the username.
//
// ****************************************************************************

void
ViewerPasswordWindow::authenticate(const char *username, const char *host,
                                   int fd)
{
#if !defined(_WIN32)
    int  timeout = 3000;
    char *buffer = new char[20000];
    char *pbuf   = buffer;

#if defined(PANTHERHACK)
// Broken on Panther
#else
    failedPortForwards.clear();
#endif

    if(!instance)
        instance = new ViewerPasswordWindow();

    for (;;)
    {
        int nread = read(fd, pbuf, 10000);
        if (nread <= 0)
        {
            // signal caught, error, or EOF
            // could have been wrong password
            delete[] buffer;
            EXCEPTION0(CouldNotConnectException);
        }

        // Write output to stderr
        write(STDERR_FILENO, pbuf, nread);

        pbuf += nread;
        *pbuf = '\0';

        if (strstr(buffer, "continue connecting (yes/no)?"))
        {
            write(fd, "yes\n", 4);
            pbuf = buffer;
        }
        else if (strstr(buffer, "continue connecting(yes/no)?"))
        {
            write(fd, "yes\n", 4);
            pbuf = buffer;
        }
        else if (strstr(buffer, "assword") ||
                 strstr(buffer, "ASSWORD") ||
                 strstr(buffer, "asscode") ||
                 strstr(buffer, "ASSCODE"))
        {
            // Password needed. Prompt for it and write it to the FD.
            instance->needToChangeUsername = false;
            const char *passwd = instance->getPassword(username, host);

            if (!passwd)
            {
                if (instance->needToChangeUsername)
                {
                    EXCEPTION0(ChangeUsernameException)
                }
                else
                {
                    // User closed the window or hit cancel
                    EXCEPTION0(CancelledConnectException);
                }
            }

            write(fd, passwd, strlen(passwd));
            write(fd, "\n", 1);
            pbuf = buffer;

            // We put up the password window, have zero timeout for
            // the connection progress dialog.
            timeout = 0;
        }
        else if (strstr(buffer, "assphrase"))
        {
            // Passphrase needed. Prompt for it and write it to the FD.
            instance->needToChangeUsername = false;
            const char *passphr = instance->getPassword(username, host, true);

            if (!passphr)
            {
                if (instance->needToChangeUsername)
                {
                    EXCEPTION0(ChangeUsernameException);
                }
                else
                {
                    // User closed the window or hit cancel
                    EXCEPTION0(CancelledConnectException);
                }
            }

            write(fd, passphr, strlen(passphr));
            write(fd, "\n", 1);
            pbuf = buffer;

            // We put up the password window, have zero timeout for
            // the connection progress dialog.
            timeout = 0;
        }
        else if (strstr(buffer, "Running: "))
        {
            // Check for failed port forwards; this will be
            // in the buffer after the password was accepted.
#if defined(PANTHERHACK)
// Broken on Panther
#else
            char failedStr[]  = "forwarding failed for listen port ";
            if (strstr(buffer, failedStr))
            {
                char failedStrLen = strlen(failedStr);
                for (char *p = buffer; p < pbuf-failedStrLen-1; p++)
                {
                    if (strncmp(failedStr, p, failedStrLen) == 0)
                    {
                        int failedPort = -1;
                        sscanf(p+failedStrLen, "%d", &failedPort);
                        failedPortForwards.insert(failedPort);
                    }
                }
            }
#endif

            // Success!  Just return.
            delete[] buffer;

            // If we have a connection progress dialog, set its timeout.
            if(dialog != NULL)
                dialog->setTimeout(timeout);
            dialog = NULL;

            return;
        }
    }
#endif
}

// ****************************************************************************
// Method: ViewerPasswordWindow::getPassword
//
// Purpose: 
//   Pops up the password window and returns the password string.
//
// Arguments:
//   username : The user's login name.
//   host     : The name of the host for which we're getting the password.
//
// Returns:    The password string.
//
// Programmer: Jeremy Meredith
// Creation:   Thu Aug 29 10:42:49 PDT 2002
//
// Modifications:
//   Brad Whitlock, Thu Aug 29 10:44:02 PDT 2002
//   I moved this code out of the authenticate method to here and made it
//   use QString.
//
//   Brad Whitlock, Thu Aug 29 17:53:13 PST 2002
//   I added code to show the username.
//
//   Brad Whitlock, Thu Dec 26 13:05:49 PST 2002
//   I added code to suspend socket signals in the viewer so the password
//   window does not cause synchronization events from the client to
//   be processed before we are ready for them. This makes the password
//   window work properly with the CLI.
//
//   Brad Whitlock, Mon Mar 31 13:34:20 PST 2003
//   I made the password window be the active window.
//
//   Jeremy Meredith, Mon May 19 17:37:52 PDT 2003
//   Added code to allow a passphrase instead of just a password.
//
//   Jeremy Meredith, Tue Dec  9 15:18:00 PST 2003
//   Make use of the exec() return value to allow both Accepted and Rejected
//   behavior.
//
//   Brad Whitlock, Wed Nov 3 17:20:49 PST 2004
//   Raise the window after it becomes active. This ensures that the
//   password window is not hidden on MacOS X.
//
//   Hank Childs, Sun Nov 11 22:21:55 PST 2007
//   Make the username be red.
//
// ****************************************************************************

const char *
ViewerPasswordWindow::getPassword(const char *username, const char *host,
                                  bool passphrase)
{
    if(!instance)
        instance = new ViewerPasswordWindow();

    const char *queryType = passphrase ? "Passphrase" : "Password";
    if (passphrase)
        instance->setCaption("Enter passphrase");
    else
        instance->setCaption("Enter password");

    // Set the password prompt.
    QString labelText;
    instance->label->setTextFormat(Qt::RichText);
    if(strcmp(username, "notset") == 0)
        labelText.sprintf("%s for %s: ", queryType, host);
    else
        // "nobr" means no line breaks, which is how it worked when we weren't
        // using rich text.
        labelText.sprintf("<nobr>%s for <font color=\"red\">%s</font>@%s: </nobr>", 
                          queryType, username, host);
    instance->label->setText(labelText);

    // Make the password window be the active window.
    instance->topLevelWidget()->setActiveWindow();
    instance->topLevelWidget()->raise();

    // Clear the password.
    instance->passedit->clear();

    // Give focus to the password window.
    QTimer::singleShot(300, instance->passedit, SLOT(setFocus()));

    // Enter the local event loop for the dialog.
#if !defined(_WIN32)
    viewerSubject->BlockSocketSignals(true);
#endif
    int status = instance->exec();
#if !defined(_WIN32)
    viewerSubject->BlockSocketSignals(false);
#endif

    // Return the password string.
    if (status == Accepted)
    {
        // Accepted; hit return or Okay.
        return instance->passedit->text().latin1();
    }
    else
    {
        // Rejected or cancelled.  Return a NULL pointer as a sentinel.
        return NULL;
    }
}

// ****************************************************************************
// Method: ViewerPasswordWindow::changeUsername
//
// Purpose:
//    A slot for changing the user name.
//
// Programmer: Hank Childs
// Creation:   November 10, 2007
//
// ****************************************************************************

void
ViewerPasswordWindow::changeUsername(void)
{
    instance->needToChangeUsername = true;
    reject();
}

