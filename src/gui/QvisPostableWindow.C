#include <qpushbutton.h>
#include <qlayout.h>

#include <QvisPostableWindow.h>
#include <QvisNotepadArea.h>
#include <DataNode.h>

//
// Static members.
//
bool QvisPostableWindow::postWhenShown = false;

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
// ****************************************************************************

QvisPostableWindow::QvisPostableWindow(const char *captionString,
    const char *shortName, QvisNotepadArea *n) : QvisWindowBase(captionString)
{
    if(shortName)
        shortCaption = QString(shortName);
    else
        shortCaption = QString("Untitled");

    isCreated = false;
    isPosted = false;
    notepad = n;
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
// ****************************************************************************

void
QvisPostableWindow::CreateNode(DataNode *parentNode)
{
    if(saveWindowDefaults)
    {
        DataNode *node = new DataNode(std::string(caption().latin1()));
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
// ****************************************************************************

void
QvisPostableWindow::SetFromNode(DataNode *parentNode, const int *borders)
{
    // Set the postWhenShown flag.
    DataNode *pwsNode = 0;
    if((pwsNode = parentNode->GetNode("postWhenShown")) != 0)
        postWhenShown = pwsNode->AsBool();

    DataNode *winNode = parentNode->GetNode(std::string(caption().latin1()));
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
        x = node->AsInt();
        if(!is_posted)
            x += borders[2];
        xy_set = true;
    }
    if((node = winNode->GetNode("y")) != 0)
    {
        y = node->AsInt();
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
            central->reparent(this, 0, QPoint(0,0), true);
            setCentralWidget(central);
            isPosted = false;

            // Reset the post button so it will post the window. 
            postButton->setText("Post");
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
//   
// ****************************************************************************

void
QvisPostableWindow::post()
{
    if(notepad)
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

        // Post to the notepad
        notepad->postWindow(this);

        // Hide the main window.
        QvisWindowBase::hide();

        // Make sure the window knows it is posted.
        isPosted = true;
        postButton->setText("Unpost");
        disconnect(postButton, SIGNAL(clicked()), this, SLOT(post()));
        connect(postButton, SIGNAL(clicked()), this, SLOT(unpost()));
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
            central->reparent(this, 0, QPoint(0,0), true);
            setCentralWidget(central);

            // Show the main window
            QvisWindowBase::show();

            // Make sure the window knows it is not posted.
            isPosted = false;
            postButton->setText("Post");
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
        central->reparent(this, 0, QPoint(0,0), true);
        setCentralWidget(central);
        postButton->setText("Post");

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
// ****************************************************************************

void
QvisPostableWindow::CreateEntireWindow()
{
    // Return if the window is already created.
    if(isCreated)
        return;

    // Create the central widget and the top layout.
    central = new QWidget( this );
    setCentralWidget( central );
    topLayout = new QVBoxLayout(central, 10);

    // Call the Sub-class's CreateWindowContents function to create the
    // internal parts of the window.
    CreateWindowContents();

    // Create a button layout and the buttons.
    topLayout->addSpacing(10);
    QHBoxLayout *buttonLayout = new QHBoxLayout(topLayout);
    buttonLayout->addStretch();
    postButton = new QPushButton("Post", central,
        "postButton");
    buttonLayout->addWidget(postButton);
    QPushButton *dismissButton = new QPushButton("Dismiss", central,
        "dismissButton");
    buttonLayout->addWidget(dismissButton);
    topLayout->addStretch(0);

    // Make the window post itself when the post button is clicked.
    if(notepad)
        connect(postButton, SIGNAL(clicked()), this, SLOT(post()));
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
