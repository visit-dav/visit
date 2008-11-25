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

#include <QPushButton>
#include <QLayout>
#include <QScrollArea>

#include <QvisPostableWindow.h>
#include <QvisNotepadArea.h>
#include <DataNode.h>

//
// Static members.
//
bool QvisPostableWindow::postWhenShown = false;
bool QvisPostableWindow::postEnabled = true;

// ****************************************************************************
// Method: QvisPostableWindow::QvisPostableWindow
//
// Purpose: 
//   Constructor for the QvisPostableWindow class.
//
// Arguments:
//   captionString : The window caption.
//   shortName     : The name to use when the window is posted.
//   n             : A pointer to the notepad widget to which the
//                   window will post.
// Returns:    
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Tue Jul 25 15:27:58 PST 2000
//
// Modifications:
//   Brad Whitlock, Wed Aug 30 13:52:47 PST 2000
//   Made it inherit from QvisWindowBase.
//
//   Brad Whitlock, Tue Apr  8 15:26:49 PDT 2008
//   Support for internationalization.
//
// ****************************************************************************

QvisPostableWindow::QvisPostableWindow(const QString &captionString,
    const QString &shortName, QvisNotepadArea *n) : QvisWindowBase(captionString)
{
    if(!shortName.isEmpty())
        shortCaption = QString(shortName);
    else
        shortCaption = QString(tr("Untitled"));

    isCreated = false;
    isPosted = false;
    notepad = n;
    addLayoutStretch = true;
}

// ****************************************************************************
// Method: QvisPostableWindow::~QvisPostableWindow
//
// Purpose: 
//   Destructor for the QvisPostableWindow class.
//
// Programmer: Brad Whitlock
// Creation:   Tue Jul 25 15:28:15 PST 2000
//
// Modifications:
//   
// ****************************************************************************

QvisPostableWindow::~QvisPostableWindow()
{
    isCreated = false;
}

// ****************************************************************************
// Method: QvisPostableWindow::CreateNode
//
// Purpose: 
//   Adds the window's geometry and visibility status to the DataNode
//   tree that will be saved to the VisIt config file.
//
// Arguments:
//   parentNode : The parent node under which we'll add a node for
//                this window.
//
// Programmer: Brad Whitlock
// Creation:   Tue Oct 3 15:30:23 PST 2000
//
// Modifications:
//   Brad Whitlock, Fri Sep 5 15:54:03 PST 2003
//   Added postWhenShown.
//
//   Brad Whitlock, Fri Jun  6 10:14:30 PDT 2008
//   Qt 4.
//
// ****************************************************************************

void
QvisPostableWindow::CreateNode(DataNode *parentNode)
{
    if(saveWindowDefaults)
    {
        DataNode *node = new DataNode(windowTitle().toStdString());
        parentNode->AddNode(node);

        // Add generic window attributes
        node->AddNode(new DataNode("x", x()));
        node->AddNode(new DataNode("y", y()));
        node->AddNode(new DataNode("width", width()));
        node->AddNode(new DataNode("height", height()));
        node->AddNode(new DataNode("visible", isVisible()));
        node->AddNode(new DataNode("posted", posted()));
    }

    //
    // Add the postWhenShown flag if it's not already in the tree.
    //
    DataNode *pwsNode = 0;
    if((pwsNode = parentNode->GetNode("postWhenShown")) == 0)
    {
        parentNode->AddNode(new DataNode("postWhenShown", postWhenShown));
    }
}

// ****************************************************************************
// Method: QvisPostableWindow::SetFromNode
//
// Purpose: 
//   Sets the window's geometry information from the DataNode.
//
// Arguments:
//   parentNode : This a pointer to the "GUI" node.
//   borders : An array of 4 ints that represent the window decoration sizes.
//
// Programmer: Brad Whitlock
// Creation:   Tue Oct 3 15:41:34 PST 2000
//
// Modifications:
//   Brad Whitlock, Wed Aug 22 16:55:11 PST 2001
//   Made the window show before posting if it is supposed to be posted.
//
//   Brad Whitlock, Wed Sep 26 10:02:34 PDT 2001
//   I removed the kludge of showing the window before posting it.
//
//   Brad Whitlock, Fri Sep 5 15:56:07 PST 2003
//   Added code to read in the postWhenShown flag.
//
//   Brad Whitlock, Wed Sep 10 09:19:32 PDT 2003
//   Added a method to make sure that the window fits on the screen.
//
//   Hank Childs, Mon Nov 14 16:25:27 PST 2005
//   Don't allow windows to come up off the screen.
//
//   Brad Whitlock, Wed Nov 22 09:56:26 PDT 2006
//   Added code to override the window location if an anchor has been provided.
//
//   Brad Whitlock, Fri Jun  6 10:14:48 PDT 2008
//   Qt 4.
//
// ****************************************************************************

void
QvisPostableWindow::SetFromNode(DataNode *parentNode, const int *borders)
{
    // Set the postWhenShown flag.
    DataNode *pwsNode = 0;
    if((pwsNode = parentNode->GetNode("postWhenShown")) != 0)
        postWhenShown = pwsNode->AsBool();

    DataNode *winNode = parentNode->GetNode(windowTitle().toStdString());
    if(winNode == 0)
        return;

    // Indicate that the window's defaults should be saved.
    saveWindowDefaults = true;

    DataNode *node;
    bool xy_set = false, wh_set = false, is_posted = false;
    int x = 0, y = 0;
    int w = width();
    int h = height();

    // See if any attributes are set.
    // If the window is posted, post it.
    if((node = winNode->GetNode("posted")) != 0)
    {
        is_posted = node->AsBool();
    }
    if((node = winNode->GetNode("x")) != 0)
    {
        int x_pos = node->AsInt();
        if (x_pos < 0)
            x_pos = 0;
        x = x_pos;
        if(!is_posted)
            x += borders[2];
        xy_set = true;
    }
    if((node = winNode->GetNode("y")) != 0)
    {
        int y_pos = node->AsInt();
        if (y_pos < 0)
            y_pos = 0;
        y = y_pos;
        if (!is_posted)
            y += borders[0];
        xy_set = true;
    }
    if((node = winNode->GetNode("width")) != 0)
    {
        w = node->AsInt();
        wh_set = true;
    }
    if((node = winNode->GetNode("height")) != 0)
    {
        h = node->AsInt();
        wh_set = true;
    }

    // Possibly override the window anchor location.
    xy_set |= GetWindowAnchorLocation(x, y);

    // Make sure that the window will fit on the screen.
    FitToScreen(x, y, w, h);

    // Set the window geometry.
    if(wh_set && xy_set)
        setGeometry(x, y, w, h);
    else if(xy_set)
        move(x, y);
    else if(wh_set)
        resize(w, h);

    // If the window is posted, post it.
    if(is_posted)
        post();

    // If the window is visible and not posted, make it visible.
    if((node = winNode->GetNode("visible")) != 0)
    {
        if(node->AsBool() && !posted())
            show();
    }
}

// ****************************************************************************
// Method: QvisPostableWindow::SetPostEnabled
//
// Purpose: 
//   Sets whether window posting is enabled.
//
// Arguments:
//   v : True if enabled; false otherwise.
//
// Programmer: Brad Whitlock
// Creation:   Mon Nov 14 10:41:29 PDT 2005
//
// Modifications:
//   
// ****************************************************************************

void
QvisPostableWindow::SetPostEnabled(bool v)
{
    postEnabled = v;
}

// ****************************************************************************
// Method: QvisPostableWindow::show
//
// Purpose: 
//   This is a Qt slot that shows the window. It has been
//   extended to support late creation of a window's widgets and the
//   ability to work with a QvisNotepadArea object.
//
// Programmer: Brad Whitlock
// Creation:   Tue Jul 25 15:24:58 PST 2000
//
// Modifications:
//   Brad Whitlock, Fri Sep 5 15:59:42 PST 2003
//   I made it possible to post windows when they are first shown.
//
//   Brad Whitlock, Tue Apr  8 15:26:49 PDT 2008
//   Support for internationalization.
//
//   Brad Whitlock, Fri Jun  6 10:24:17 PDT 2008
//   Qt 4.
//
// ****************************************************************************

void
QvisPostableWindow::show()
{
    if(postWhenShown)
        post();
    else
    {
        if(!isCreated)
        {
            // Create the window and show it.
            CreateEntireWindow();
            isCreated = true;

            // Update the widgets based on the state information.
            UpdateWindow(true);
        }
        else if(isPosted && (notepad != NULL))
        {
            // Call code to unpost the window.
            notepad->postWindow(this);
            central->setParent(this);
            central->show();
            setCentralWidget(central);
            isPosted = false;

            // Reset the post button so it will post the window. 
            postButton->setText(tr("Post"));
            disconnect(postButton, SIGNAL(clicked()), this, SLOT(unpost()));
            connect(postButton, SIGNAL(clicked()), this, SLOT(post()));
        }
        else
        {
            // Update the geometry and show the window
            central->updateGeometry();
        }

        // Show the window.
        QvisWindowBase::show();
    }
}

// ****************************************************************************
// Method: QvisPostableWindow::raise
//
// Purpose: 
//   This is a Qt slot that raises the window. It has been
//   extended to support late creation of a window's widgets and the
//   ability to work with a QvisNotepadArea object.
//
// Programmer: Brad Whitlock
// Creation:   Tue Jul 25 15:24:58 PST 2000
//
// Modifications:
//   
// ****************************************************************************

void
QvisPostableWindow::raise()
{
    if(!isCreated)
    {
        // Create the window
        CreateEntireWindow();
        isCreated = true;

        // Update the widgets to the right values
        UpdateWindow(true);

        // raise the window
        QvisWindowBase::raise();
    }
    else if(isPosted && (notepad != NULL))
    {
        notepad->showPage(this);
    }
    else
        QvisWindowBase::raise();
}

// ****************************************************************************
// Method: QvisPostableWindow::post
//
// Purpose: 
//   This is a Qt slot function that causes a window to be posted
//   to a QvisNotepadArea object.
//
// Programmer: Brad Whitlock
// Creation:   Fri Jul 28 15:52:21 PST 2000
//
// Modifications:
//   Brad Whitlock, Mon Nov 14 10:37:57 PDT 2005
//   Added code to disable posting if it's not enabled.
//
//   Gunther H. Weber, Fri Jan 18 10:16:48 PST 2008
//   Added check to prevent posting when window is already posted.
//
//   Brad Whitlock, Tue Apr  8 15:26:49 PDT 2008
//   Support for internationalization.
//
// ****************************************************************************

void
QvisPostableWindow::post()
{
    if(!isPosted && notepad)
    {
        if(!isCreated)
        {
            // Create the window.
            CreateEntireWindow();
            isCreated = true;

            // Update the widgets based on the state information.
            UpdateWindow(true);
            saveWindowDefaults = true;
        }

        if(postEnabled)
        {
            // Post to the notepad
            notepad->postWindow(this);

            // Hide the main window.
            QvisWindowBase::hide();

            // Make sure the window knows it is posted.
            isPosted = true;
            postButton->setText(tr("Unpost"));
            disconnect(postButton, SIGNAL(clicked()), this, SLOT(post()));
            connect(postButton, SIGNAL(clicked()), this, SLOT(unpost()));
        }
        else
            QvisWindowBase::show();
    }
}

// ****************************************************************************
// Method: QvisPostableWindow::unpost
//
// Purpose: 
//   This is a Qt slot function that causes the window to be unposted
//   from a QvisNotepad area.
//
// Programmer: Brad Whitlock
// Creation:   Fri Jul 28 15:53:37 PST 2000
//
// Modifications:
//   Brad Whitlock, Tue Apr  8 15:26:49 PDT 2008
//   Support for internationalization.
//
//   Brad Whitlock, Fri Jun  6 10:25:12 PDT 2008
//   Qt 4.
//
// ****************************************************************************

void
QvisPostableWindow::unpost()
{
    if(notepad)
    {
        if(!isCreated)
        {
            // Create the window and show it.
            CreateEntireWindow();
            isCreated = true;

            // Update the widgets based on the state information.
            UpdateWindow(true);
            QvisWindowBase::show();
        }
        else
        {
            // Unpost the window
            notepad->postWindow(this);
            central->setParent(this);
            central->show();
            setCentralWidget(central);

            // Show the main window
            QvisWindowBase::show();

            // Make sure the window knows it is not posted.
            isPosted = false;
            postButton->setText(tr("Post"));
            disconnect(postButton, SIGNAL(clicked()), this, SLOT(unpost()));
            connect(postButton, SIGNAL(clicked()), this, SLOT(post()));
        }
    }
}

// ****************************************************************************
// Method: QvisPostableWindow::hide
//
// Purpose: 
//   This is a Qt slot function that hides the window or unposts and
//   hides the window.
//
// Programmer: Brad Whitlock
// Creation:   Fri Jul 28 15:54:48 PST 2000
//
// Modifications:
//   Brad Whitlock, Tue Apr  8 15:26:49 PDT 2008
//   Support for internationalization.
//
//   Brad Whitlock, Fri Jun  6 10:25:46 PDT 2008
//   Qt 4.
//
// ****************************************************************************

void
QvisPostableWindow::hide()
{
    if(isPosted && (notepad != NULL))
    {
        // The window is posted. Tell the notepad area to unpost it.
        notepad->postWindow(this);

        // Now that the window is unposted, reparent it to this widget
        // since the act of posting it reparented it to the notepad area.
        central->setParent(this);
        central->show();
        setCentralWidget(central);
        postButton->setText(tr("Post"));

        // The window has been hidden. Make sure that the next time we
        // bring it up, the post button will post.
        disconnect(postButton, SIGNAL(clicked()), this, SLOT(unpost()));
        connect(postButton, SIGNAL(clicked()), this, SLOT(post()));
    }
    else
        QvisWindowBase::hide();

    isPosted = false;
}

// ****************************************************************************
// Method: QvisPostableWindow::posted
//
// Purpose: 
//   Returns whether or not the window is currently posted.
//
// Programmer: Brad Whitlock
// Creation:   Fri Jul 28 14:51:09 PST 2000
//
// Modifications:
//   
// ****************************************************************************

bool
QvisPostableWindow::posted()
{
    return isPosted;
}

// ****************************************************************************
// Method: QvisPostableWindow::GetCentralWidget
//
// Purpose: 
//   Returns a pointer to the central widget.
//
// Arguments:
//
// Returns:    
//    A pointer to the central widget.
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Fri Jul 28 14:50:28 PST 2000
//
// Modifications:
//   
// ****************************************************************************

QWidget *
QvisPostableWindow::GetCentralWidget()
{
    return central;
}

// ****************************************************************************
// Method: QvisPostableWindow::GetShortCaption
//
// Purpose: 
//   Returns the short name of the window.
//
// Programmer: Brad Whitlock
// Creation:   Tue Aug 1 17:28:08 PST 2000
//
// Modifications:
//   
// ****************************************************************************

const QString &
QvisPostableWindow::GetShortCaption()
{
    return shortCaption;
}

// ****************************************************************************
// Method: QvisPostableWindow::CreateEntireWindow
//
// Purpose: 
//   This method creates the entire window. First it makes the central
//   widget and the toplayout, then it calls the subclass's CreateWindowContents
//   method to put the useful stuff into the window. Finally, it creates
//   the Post and Dismiss buttons.
//
// Programmer: Brad Whitlock
// Creation:   Fri Jul 28 14:49:02 PST 2000
//
// Modifications:
//   Brad Whitlock, Fri Feb 15 11:24:06 PDT 2002
//   Added code to return early if the window is already created.
//
//   Brad Whitlock, Mon Nov 14 10:38:48 PDT 2005
//   Added code to disable the post button if posting is not enabled.
//
//   Brad Whitlock, Tue Jan 22 16:49:54 PST 2008
//   Embed the contents in a scrollview if there's no notepad.
//
//   Brad Whitlock, Tue Apr  8 15:26:49 PDT 2008
//   Support for internationalization.
//
//   Brad Whitlock, Fri May 30 10:01:20 PDT 2008
//   Qt 4.
//
// ****************************************************************************

void
QvisPostableWindow::CreateEntireWindow()
{
    // Return if the window is already created.
    if(isCreated)
        return;

    // Create the central widget and the top layout.
    QWidget *topCentral = 0;
    QVBoxLayout *vLayout = 0;
    if(notepad)
    {
        central = new QWidget( this );
        setCentralWidget( central );
        topCentral = central;
        topLayout = new QVBoxLayout(central);
        vLayout = topLayout;
    }
    else
    {
        topCentral = new QWidget(this);
        vLayout = new QVBoxLayout(topCentral);
        vLayout->setSpacing(5);
        setCentralWidget( topCentral );
        
        QScrollArea *sv = new QScrollArea(topCentral);
        sv->setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);
        sv->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
        sv->setWidgetResizable(true);
        central = new QWidget(0);
        sv->setWidget(central);
        vLayout->addWidget(sv);
        topLayout = new QVBoxLayout(central);
    }

    // Call the Sub-class's CreateWindowContents function to create the
    // internal parts of the window.
    CreateWindowContents();

    // Create a button layout and the buttons.
    vLayout->addSpacing(10);
    QHBoxLayout *buttonLayout = new QHBoxLayout(0);
    vLayout->addLayout(buttonLayout);
    buttonLayout->addStretch();
    postButton = new QPushButton(tr("Post"));
    buttonLayout->addWidget(postButton);
    dismissButton = new QPushButton(tr("Dismiss"));
    buttonLayout->addWidget(dismissButton);
    if(notepad != 0 && addLayoutStretch)
        vLayout->addStretch(0);

    // Make the window post itself when the post button is clicked.
    if(notepad)
    {
        connect(postButton, SIGNAL(clicked()), this, SLOT(post()));
        postButton->setEnabled(postEnabled);
    }
    else
        postButton->setEnabled(false);

    // Hide this window when the dimiss button is clicked.
    connect(dismissButton, SIGNAL(clicked()), this, SLOT(hide()));
}

// ****************************************************************************
// Method: QvisPostableWindow::UpdateWindow
//
// Purpose: 
//   This is method is supposed to be overidden in subclasses. It's
//   a place holder for the sub classes. This method is called after
//   a window is created as the result of a show or a raise. It is 
//   intended to update the widgets in the newly created window.
//
// Programmer: Brad Whitlock
// Creation:   Tue Aug 1 15:41:48 PST 2000
//
// Modifications:
//   
// ****************************************************************************

void
QvisPostableWindow::UpdateWindow(bool)
{
    // do nothing.
}
