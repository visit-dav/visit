#include <QvisPostableWindowObserver.h>
#include <qpushbutton.h>
#include <qlayout.h>

#include <AttributeSubject.h>

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
//    Brad Whitlock, Fri Nov 7 16:23:53 PST 2003
//    I made it inherit from QvisPostableWindowSimpleObserver.
//
// ****************************************************************************

QvisPostableWindowObserver::QvisPostableWindowObserver(
    Subject *subj, const char *caption, const char *shortName,
    QvisNotepadArea *notepad, int buttonCombo, bool stretch) : 
    QvisPostableWindowSimpleObserver(caption, shortName, notepad, buttonCombo,
        stretch)
{
    subject = subj;
    subject->Attach(this);
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
    if(subject != 0)
        subject->Detach(this);
}

// ****************************************************************************
// Method: QvisPostableWindowObserver::SubjectRemoved
//
// Purpose: 
//   This method is called when the subject is deleted so we can set the 
//   selectedSubject pointer to zero so we don't try and detach from the
//   subject in this window's destructor.
//
// Arguments:
//   TheRemovedSubject : A pointer to the subject being removed.
//
// Programmer: Brad Whitlock
// Creation:   Fri Nov 7 16:25:49 PST 2003
//
// Modifications:
//   
// ****************************************************************************

void
QvisPostableWindowObserver::SubjectRemoved(Subject *TheRemovedSubject)
{
    if(subject == TheRemovedSubject)
        subject = 0;
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
//   Brad Whitlock, Fri Nov 7 16:53:29 PST 2003
//   I made it use the selectedSubject pointer.
//
// ****************************************************************************

void
QvisPostableWindowObserver::apply()
{
    AttributeSubject *as = (AttributeSubject *)selectedSubject;

    // If the AttributeSubject that we're observing has fields
    // that are selected, make it notify all its observers.
    if(as->NumAttributesSelected() > 0)
    {
        as->Notify();
    }
}

