#include <QvisPostableWindowObserver.h>
#include <qpushbutton.h>
#include <qlayout.h>
#include <qmessagebox.h>

#include <AttributeSubject.h>
#include <GlobalAttributes.h>
#include <ViewerProxy.h>

// Static members.
const int QvisPostableWindowObserver::NoExtraButtons    = 0;
const int QvisPostableWindowObserver::ApplyButton       = 1;
const int QvisPostableWindowObserver::MakeDefaultButton = 2;
const int QvisPostableWindowObserver::ResetButton       = 4;
const int QvisPostableWindowObserver::AllExtraButtons   = 7;

// ****************************************************************************
// Method: QvisPostableWindowObserver::QvisPostableWindowObserver
//
// Purpose: 
//   Constructor for the QvisPostableWindowObserver class. It
//   passes construction responsibility to the superclasses.
//
// Arguments:
//   caption : A string containing the name of the window.
//   notepad : The notepad that the window will post to.
//   subj    : The subject that the window is observing.
//
// Programmer: Brad Whitlock
// Creation:   Fri Jul 28 17:10:59 PST 2000
//
// Modifications:
//   Brad Whitlock, Mon Feb 12 13:40:02 PST 2001
//   I added the extra_ argument which specifies whether or not the
//   "Make default" and "Reset" buttons are created.
//
//   Brad Whitlock, Wed May 2 11:48:12 PDT 2001
//   Replaced the extraArgument with buttonCombination.
//
//   Brad Whitlock, Tue Aug 21 16:33:50 PST 2001
//   Added the stretchWindow argument.
//
// ****************************************************************************

QvisPostableWindowObserver::QvisPostableWindowObserver(
    Subject *subj, const char *caption, const char *shortName,
    QvisNotepadArea *notepad, int buttonCombo, bool stretch) : 
    QvisPostableWindow(caption, shortName, notepad), Observer(subj)
{
    buttonCombination = buttonCombo;
    stretchWindow = stretch;
}

// ****************************************************************************
// Method: QvisPostableWindowObserver::~QvisPostableWindowObserver
//
// Purpose: 
//   Destructor for the QvisPostableWindowObserver class.
//
// Programmer: Brad Whitlock
// Creation:   Fri Jul 28 17:11:41 PST 2000
//
// Modifications:
//   
// ****************************************************************************

QvisPostableWindowObserver::~QvisPostableWindowObserver()
{
    // nothing here.
}

// ****************************************************************************
// Method: QvisPostableWindowObserver::CreateEntireWindow
//
// Purpose: 
//   This method creates the central widget and the top layout. It
//   also creates the Apply, Post, Dismiss buttons and sets up their
//   behavior.
//
// Programmer: Brad Whitlock
// Creation:   Fri Jul 28 18:29:54 PST 2000
//
// Modifications:
//   Brad Whitlock, Mon Feb 12 13:38:53 PST 2001
//   I made it so the "Make default" and "Reset" buttons are only
//   created some of the time.
//
//   Brad Whitlock, Wed May 2 11:49:51 PDT 2001
//   Modified the button creation code so it works with the new
//   buttonCombination member.
//
//   Brad Whitlock, Tue Aug 21 16:34:23 PST 2001
//   Added code to make the stretch in the topLayout optional.
//
//   Brad Whitlock, Thu Aug 23 8:31:23 PDT 2001
//   Made the make default button use a different slot function.
//
//   Brad Whitlock, Fri Feb 15 11:18:56 PDT 2002
//   Added code to return early if the window is already created.
//
// ****************************************************************************

void
QvisPostableWindowObserver::CreateEntireWindow()
{
    // If the window is already created, return.
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
    int nrows = ((buttonCombination & MakeDefaultButton) ||
                 (buttonCombination & ResetButton)) ? 2 : 1;
    QGridLayout *buttonLayout = new QGridLayout(topLayout, nrows, 4);
    buttonLayout->setColStretch(1, 50);

    // Create the extra buttons if necessary.
    if(buttonCombination & MakeDefaultButton)
    {
        QPushButton *makeDefaultButton = new QPushButton("Make default",
            central, "makeDefaultButton");
        connect(makeDefaultButton, SIGNAL(clicked()),
                this, SLOT(makeDefaultHelper()));
        buttonLayout->addWidget(makeDefaultButton, 0, 0);
    }
    if(buttonCombination & ResetButton)
    {
        QPushButton *resetButton = new QPushButton("Reset", central,
                                                   "resetButton");
        connect(resetButton, SIGNAL(clicked()), this, SLOT(reset()));
        buttonLayout->addWidget(resetButton, 0, 3);
    }
    if(buttonCombination & ApplyButton)
    {
        QPushButton *applyButton = new QPushButton("Apply", central,
            "applyButton");
        connect(applyButton, SIGNAL(clicked()), this, SLOT(apply()));
        buttonLayout->addWidget(applyButton, 1, 0);
    }
    else
    {
        // Add a little space to try and make up for the absence of the
        // grid layout.
        buttonLayout->addColSpacing(1, 50);
    }

    postButton = new QPushButton("Post", central, "postButton");
    // Make the window post itself when the post button is clicked.
    if(notepad)
        connect(postButton, SIGNAL(clicked()), this, SLOT(post()));
    else
        postButton->setEnabled(false);
    buttonLayout->addWidget(postButton, 1, 2);
    QPushButton *dismissButton = new QPushButton("Dismiss", central,
        "dismissButton");
    connect(dismissButton, SIGNAL(clicked()), this, SLOT(hide()));
    buttonLayout->addWidget(dismissButton, 1, 3);
    if(stretchWindow)
        topLayout->addStretch(0);

    // Set the isCreated flag.
    isCreated = true;
}

// ****************************************************************************
// Method: QvisPostableWindowObserver::Update
//
// Purpose: 
//   Updates the widgets in the window that need to be updated. This
//   method is called by a subject that wants to update its observers.
//
// Note:       
//     The subclass's UpdateWindow method is called to do the update.
//
// Programmer: Brad Whitlock
// Creation:   Tue Aug 1 15:36:23 PST 2000
//
// Modifications:
//   
// ****************************************************************************

void
QvisPostableWindowObserver::Update(Subject *)
{
    if(isCreated)
        UpdateWindow(false);
}

// ****************************************************************************
// Method: QvisPostableWindowObserver::apply
//
// Purpose: 
//   This is a Qt slot function that forces the subject that is being
//   observed to notify its observers if it has changed.
//
// Programmer: Brad Whitlock
// Creation:   Fri Jul 28 18:28:49 PST 2000
//
// Modifications:
//   
// ****************************************************************************

void
QvisPostableWindowObserver::apply()
{
    AttributeSubject *as = (AttributeSubject *)subject;

    // If the AttributeSubject that we're observing has fields
    // that are selected, make it notify all its observers.
    if(as->NumAttributesSelected() > 0)
    {
        as->Notify();
    }
}

// ****************************************************************************
// Method: QvisPostableWindowObserver::makeDefaultHelper
//
// Purpose: 
//   This is a Qt slot function that calls the makeDefault() slot after
//   optionally displaying a confirmation dialog box.
//
// Programmer: Brad Whitlock
// Creation:   Thu Aug 23 8:31:23 PDT 2001
//
// Modifications:
//   Brad Whitlock, Mon Aug 27 11:17:21 PDT 2001
//   I changed the behavior of the third button in the confirmation window.
//
//   Brad Whitlock, Mon Nov 4 14:11:22 PST 2002
//   I made it so the "make default" behavior can be turned off permanently.
//
// ****************************************************************************

void
QvisPostableWindowObserver::makeDefaultHelper()
{
    if(makeDefaultConfirm)
    {
        QString msg("Do you really want to make these the default attributes?");

        // Ask the user if he really wants to set the defaults
        int button = QMessageBox::warning(0, "VisIt", msg, "Ok", "Cancel",
                                          "Yes, Do not prompt again", 0, 1);

        if(button == 0)
        {
            // The user actually chose to set the defaults.
            makeDefault();
        }
        else if(button == 2)
        {
            // Make it so no confirmation is needed.
            makeDefaultConfirm = false;
            GlobalAttributes *globalAtts = viewer->GetGlobalAttributes();
            globalAtts->SetMakeDefaultConfirm(false);
            globalAtts->Notify();

            makeDefault();
        }
    }
    else
        makeDefault();
}

// ****************************************************************************
// Method: QvisPostableWindowObserver::makeDefault
//
// Purpose: 
//   Causes the current attributes to be copied into the default
//   attributes. This should be overridden in derived classes.
//
// Programmer: Brad Whitlock
// Creation:   Fri Sep 15 13:40:19 PST 2000
//
// Modifications:
//   
// ****************************************************************************

void
QvisPostableWindowObserver::makeDefault()
{
    // override in derived class.
}

// ****************************************************************************
// Method: QvisPostableWindowObserver::reset
//
// Purpose: 
//   Causes the last applied attributes to be restored. This will
//   result in the window's UpdateWindow method being called to
//   update the window's widgets.
//
// Programmer: Brad Whitlock
// Creation:   Fri Sep 15 13:40:19 PST 2000
//
// Modifications:
//   
// ****************************************************************************

void
QvisPostableWindowObserver::reset()
{
    // override in derived class.
}
