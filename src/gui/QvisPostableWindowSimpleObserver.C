#include <QvisPostableWindowSimpleObserver.h>
#include <qlayout.h>
#include <qpushbutton.h>

// ****************************************************************************
// Method: QvisPostableWindowSimpleObserver::QvisPostableWindowSimpleObserver
//
// Purpose: 
//   Constructor for the QvisPostableWindowSimpleObserver class.
//
// Arguments:
//   caption   : The window caption.
//   shortName : The name used when the window is posted.
//   notepad   : The notepad area into which the window posts.
//
// Programmer: Brad Whitlock
// Creation:   Mon Aug 27 12:39:44 PDT 2001
//
// Modifications:
//   Jeremy Meredith, Wed May  8 10:51:09 PDT 2002
//   Added stretch flag.
//
//   Brad Whitlock, Mon Sep 9 11:00:03 PDT 2002
//   Added applyButton flag.
//
// ****************************************************************************

QvisPostableWindowSimpleObserver::QvisPostableWindowSimpleObserver(
    const char *caption, const char *shortName, QvisNotepadArea *notepad,
    bool stretch) :
    QvisPostableWindow(caption, shortName, notepad), SimpleObserver()
{
    selectedSubject = 0;
    stretchWindow = stretch;
    applyButton = true;
}

// ****************************************************************************
// Method: QvisPostableWindowSimpleObserver::~QvisPostableWindowSimpleObserver
//
// Purpose: 
//   Destructor for the QvisPostableWindowSimpleObserver class.
//
// Programmer: Brad Whitlock
// Creation:   Mon Aug 27 12:40:40 PDT 2001
//
// Modifications:
//   
// ****************************************************************************

QvisPostableWindowSimpleObserver::~QvisPostableWindowSimpleObserver()
{
    // nothing here.
}

// ****************************************************************************
// Method: QvisPostableWindowSimpleObserver::Update
//
// Purpose: 
//   This method is called when the window's subjects are modified.
//
// Arguments:
//   subj : A pointer to the subject that was modified.
//
// Programmer: Brad Whitlock
// Creation:   Mon Aug 27 12:40:58 PDT 2001
//
// Modifications:
//   
// ****************************************************************************

void
QvisPostableWindowSimpleObserver::Update(Subject *subj)
{
    // This is what derived types will query to determine which subject
    // was modified.
    selectedSubject = subj;

    if(isCreated)
        UpdateWindow(false);
}

// ****************************************************************************
// Method: QvisPostableWindowSimpleObserver::SubjectRemoved
//
// Purpose: 
//   This method is called when a subject is deleted.
//
// Programmer: Brad Whitlock
// Creation:   Mon Aug 27 12:41:39 PDT 2001
//
// Modifications:
//   
// ****************************************************************************

void
QvisPostableWindowSimpleObserver::SubjectRemoved(Subject *TheRemovedSubject)
{
    // nothing here.
}

// ****************************************************************************
// Method: QvisPostableWindowSimpleObserver::SelectedSubject
//
// Purpose: 
//   Returns which subject is currently being updated.
//
// Programmer: Brad Whitlock
// Creation:   Mon Aug 27 12:42:10 PDT 2001
//
// Modifications:
//   
// ****************************************************************************

Subject *
QvisPostableWindowSimpleObserver::SelectedSubject()
{
    return selectedSubject;
}

// ****************************************************************************
// Method: QvisPostableWindow::CreateEntireWindow
//
// Purpose: 
//   Creates the widgets for the window.
//
// Programmer: Brad Whitlock
// Creation:   Mon Aug 27 12:36:50 PDT 2001
//
// Modifications:
//   Brad Whitlock, Fri Feb 15 11:27:15 PDT 2002
//   Added code to return if the window is already created.
//
//   Jeremy Meredith, Wed May  8 10:50:54 PDT 2002
//   Only add stretch if the stretchWindow flag is set.
//
//   Brad Whitlock, Mon Sep 9 11:01:00 PDT 2002
//   I made the apply button optional.
//
// ****************************************************************************

void
QvisPostableWindowSimpleObserver::CreateEntireWindow()
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
    if(applyButton)
    {
        QPushButton *applyButton = new QPushButton("Apply", central,
            "applyButton");
        connect(applyButton, SIGNAL(clicked()), this, SLOT(apply()));
        buttonLayout->addWidget(applyButton);
    }
    buttonLayout->addStretch();

    postButton = new QPushButton("Post", central,
        "postButton");
    buttonLayout->addWidget(postButton);
    QPushButton *dismissButton = new QPushButton("Dismiss", central,
        "dismissButton");
    buttonLayout->addWidget(dismissButton);
    if(stretchWindow)
        topLayout->addStretch(0);

    // Make the window post itself when the post button is clicked.
    if(notepad)
        connect(postButton, SIGNAL(clicked()), this, SLOT(post()));
    else
        postButton->setEnabled(false);

    // Hide this window when the dimiss button is clicked.
    connect(dismissButton, SIGNAL(clicked()), this, SLOT(hide()));
}
