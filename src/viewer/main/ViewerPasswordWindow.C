#include "ViewerPasswordWindow.h"
#include <ViewerConnectionProgressDialog.h>

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
// ****************************************************************************

void
ViewerPasswordWindow::authenticate(const char *username, const char *host,
    int fd)
{
#if !defined(_WIN32)
    int  timeout = 3000;
    char *buffer = new char[20000];
    char *pbuf   = buffer;

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
        else if (strstr(buffer, "assword"))
        {
            // Password needed. Prompt for it and write it to the FD.
            const char *passwd = instance->getPassword(username, host);

            if (!passwd)
            {
                // User closed the window or hit cancel
                EXCEPTION0(CancelledConnectException);
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
            const char *passphr = instance->getPassword(username, host, true);

            if (!passphr)
            {
                // User closed the window or hit cancel
                EXCEPTION0(CancelledConnectException);
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
    if(strcmp(username, "notset") == 0)
        labelText.sprintf("%s for %s: ", queryType, host);
    else
        labelText.sprintf("%s for %s@%s: ", queryType, username, host);
    instance->label->setText(labelText);

    // Make the password window be the active window.
    instance->topLevelWidget()->setActiveWindow();

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
