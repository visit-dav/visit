#include <qlayout.h>
#include <QvisDelayedWindow.h>

// ****************************************************************************
// Method: QvisDelayedWindow::QvisDelayedWindow
//
// Purpose: 
//   Constructor for the QvisDelayedWindow class.
//
// Arguments:
//   captionString : The caption that appears in the window decoration.
//
// Programmer: Brad Whitlock
// Creation:   Wed Aug 30 10:01:16 PDT 2000
//
// Modifications:
//   Brad Whitlock, Wed Aug 30 13:50:54 PST 2000
//   Made it inherit from QvisWindowBase.
//
//   Brad Whitlock, Mon Sep 30 07:51:17 PDT 2002
//   I added window flags.
//
// ****************************************************************************

QvisDelayedWindow::QvisDelayedWindow(const char *captionString, WFlags f) : 
    QvisWindowBase(captionString, f)
{
    isCreated = false;
}

// ****************************************************************************
// Method: QvisDelayedWindow::~QvisDelayedWindow
//
// Purpose: 
//   Destructor for the QvisDelayedWindow class.
//
// Programmer: Brad Whitlock
// Creation:   Wed Aug 30 10:01:52 PDT 2000
//
// Modifications:
//   
// ****************************************************************************

QvisDelayedWindow::~QvisDelayedWindow()
{
    isCreated = false;
}

// ****************************************************************************
// Method: QvisDelayedWindow::show
//
// Purpose: 
//   This is a slot function that shows the window. It creates the
//   window first if it has not yet been created. It raises the
//   window if it is already showing.
//
// Programmer: Brad Whitlock
// Creation:   Wed Aug 30 10:02:16 PDT 2000
//
// Modifications:
//   
// ****************************************************************************

void
QvisDelayedWindow::show()
{
    if(!isCreated)
    {
        // Create the window and show it.
        CreateEntireWindow();
        isCreated = true;

        // Update the widgets based on the state information.
        UpdateWindow(true);

        // Show the window
        QvisWindowBase::show();
    }
    else if(isVisible())
    {
        // The window is created and is already showing.
        raise();
    }
    else
    {
        // Show the window
        QvisWindowBase::show();
    }
}

// ****************************************************************************
// Method: QvisDelayedWindow::raise
//
// Purpose: 
//   This is a slot function that raises the window. It creates the
//   window first if it has not yet been created.
//
// Programmer: Brad Whitlock
// Creation:   Wed Aug 30 10:03:07 PDT 2000
//
// Modifications:
//   
// ****************************************************************************

void
QvisDelayedWindow::raise()
{
    if(!isCreated)
    {
        // Create the window
        CreateEntireWindow();
        isCreated = true;

        // Update the widgets to the right values
        UpdateWindow(true);
    }

    // raise the window
    QvisWindowBase::raise();
}

// ****************************************************************************
// Method: QvisDelayedWindow::hide
//
// Purpose: 
//   This is a slot function that hides the window if it has been
//   created.
//
// Programmer: Brad Whitlock
// Creation:   Wed Aug 30 10:04:16 PDT 2000
//
// Modifications:
//   
// ****************************************************************************

void
QvisDelayedWindow::hide()
{
    if(isCreated)
    {
        QvisWindowBase::hide();
    }
}

// ****************************************************************************
// Method: QvisDelayedWindow::GetCentralWidget
//
// Purpose: 
//   Returns a pointer to the window's central widget.
//
// Programmer: Brad Whitlock
// Creation:   Wed Aug 30 10:05:08 PDT 2000
//
// Modifications:
//   
// ****************************************************************************

QWidget *
QvisDelayedWindow::GetCentralWidget()
{
    return central;
}

// ****************************************************************************
// Method: QvisDelayedWindow::CreateEntireWindow
//
// Purpose: 
//   Creates the central widget and the top layout. It then calls the
//   subclass's CreateWindowContents method which actually adds the bulk of
//   the widgets to the window.
//
// Programmer: Brad Whitlock
// Creation:   Wed Aug 30 10:05:38 PDT 2000
//
// Modifications:
//   Brad Whitlock, Fri Feb 15 11:34:31 PDT 2002
//   Added an early return if the window exists.
//
// ****************************************************************************

void
QvisDelayedWindow::CreateEntireWindow()
{
    // If the window is created then return.
    if(isCreated)
        return;

    // Create the central widget and the top layout.
    central = new QWidget( this );
    setCentralWidget( central );
    topLayout = new QVBoxLayout(central, 10);

    // Call the Sub-class's CreateWindowContents function to create the
    // internal parts of the window.
    CreateWindowContents();
}

// ****************************************************************************
// Method: QvisDelayedWindow::UpdateWindow
//
// Purpose: 
//   This function is called by the subclass's CreateWindowContents method.
//   It is supposed to be overridden in subclasses so it does something
//   useful.
//
// Arguments:
//   doAll : Whether or not to update all the widgets in the window.
//
// Programmer: Brad Whitlock
// Creation:   Wed Aug 30 10:06:31 PDT 2000
//
// Modifications:
//   
// ****************************************************************************

void
QvisDelayedWindow::UpdateWindow(bool)
{
    // do nothing.
}
