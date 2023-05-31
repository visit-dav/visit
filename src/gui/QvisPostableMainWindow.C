// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#include <QvisPostableMainWindow.h>
#include <qpushbutton.h>
#include <qlayout.h>
#include <qlabel.h>

// ****************************************************************************
// Method: QvisPostableMainWindow::QvisPostableMainWindow
//
// Purpose: 
//   Constructor.
//
// Arguments:
//   captionString : The window caption.
//   shortName     : The window name that appears in the notepad title.
//   n             : The notepad.
//
// Programmer: Brad Whitlock
// Creation:   Fri Jul 24 09:39:38 PDT 2009
//
// Modifications:
//   
// ****************************************************************************

QvisPostableMainWindow::QvisPostableMainWindow(const QString &captionString,
    const QString &shortName, QvisNotepadArea *n) : QvisPostableWindow(captionString, shortName, n)
{
    contentsWidget = 0;
    contentsLayout = 0;
    addLayoutStretch = false;

    CreateEntireWindow();
    dismissButton->setEnabled(false);
    isCreated = true;
}

// ****************************************************************************
// Method: QvisPostableMainWindow::~QvisPostableMainWindow
//
// Purpose: 
//   Destructor
//
// Programmer: Brad Whitlock
// Creation:   Fri Jul 24 09:40:24 PDT 2009
//
// Modifications:
//   
// ****************************************************************************

QvisPostableMainWindow::~QvisPostableMainWindow()
{
}

// ****************************************************************************
// Method: QvisPostableMainWindow::CreateWindowContents
//
// Purpose: 
//   Creates a widget that we'll use for the parent of the main window stuff.
//
// Programmer: Brad Whitlock
// Creation:   Fri Jul 24 09:40:38 PDT 2009
//
// Modifications:
//   
// ****************************************************************************

void
QvisPostableMainWindow::CreateWindowContents()
{
    topLayout->setContentsMargins(0,0,0,0);
    contentsWidget = new QWidget(central);
    topLayout->addWidget(contentsWidget);
    contentsLayout = new QVBoxLayout(contentsWidget);
    contentsLayout->setContentsMargins(0,0,0,0);
}

void
QvisPostableMainWindow::CreateNode(DataNode *)
{
}

void
QvisPostableMainWindow::SetFromNode(DataNode *, const int *borders)
{
}

// ****************************************************************************
// Method: QvisPostableMainWindow::ContentsWidget
//
// Purpose: 
//   Return the contents widget.
//
// Programmer: Brad Whitlock
// Creation:   Fri Jul 24 09:41:03 PDT 2009
//
// Modifications:
//   
// ****************************************************************************

QWidget *
QvisPostableMainWindow::ContentsWidget()
{
    return contentsWidget;
}

// ****************************************************************************
// Method: QvisPostableMainWindow::ContentsLayout
//
// Purpose: 
//   Return the contents layout.
//
// Programmer: Brad Whitlock
// Creation:   Fri Jul 24 09:41:46 PDT 2009
//
// Modifications:
//   
// ****************************************************************************

QVBoxLayout *
QvisPostableMainWindow::ContentsLayout()
{
    return contentsLayout;
}
