#include <QvisWizard.h>
#include <AttributeSubject.h>

// ****************************************************************************
// Method: QvisWizard::QvisWizard
//
// Purpose: 
//   Constructor for the QvisWizard class.
//
// Arguments:
//   subj   : The subject that the wizard will use to send its data to
//            the viewer.
//   parent : The wizard's parent widget.
//   name   : The name of the wizard.
//
// Programmer: Brad Whitlock
// Creation:   Tue Dec 14 09:32:56 PDT 2004
//
// Modifications:
//   
// ****************************************************************************

QvisWizard::QvisWizard(AttributeSubject *subj,
    QWidget *parent, const char *name) : QWizard(parent, name)
{
    atts = subj;
    localCopy = atts->NewInstance(true);
}

// ****************************************************************************
// Method: QvisWizard::~QvisWizard
//
// Purpose: 
//   Destructor for the QvisWizard class.
//
// Programmer: Brad Whitlock
// Creation:   Tue Dec 14 09:35:31 PDT 2004
//
// Modifications:
//   
// ****************************************************************************

QvisWizard::~QvisWizard()
{
    atts = 0;
    delete localCopy;
}

// ****************************************************************************
// Method: QvisWizard::SendAttributes
//
// Purpose: 
//   Tells the wizard to send its attributes to the viewer.
//
// Programmer: Brad Whitlock
// Creation:   Tue Dec 14 09:35:51 PDT 2004
//
// Modifications:
//   
// ****************************************************************************

void
QvisWizard::SendAttributes()
{
    if(atts->CopyAttributes(localCopy))
    {
        atts->SelectAll();
        atts->Notify();
    }
}

// ****************************************************************************
// Method: QvisWizard::UpdateAttributes
//
// Purpose: 
//   Writes over the wizard's local atts with the original atts.
//
// Programmer: Brad Whitlock
// Creation:   Wed Jun 22 11:44:22 PDT 2005
//
// Modifications:
//   
// ****************************************************************************

bool
QvisWizard::UpdateAttributes()
{
    return localCopy->CopyAttributes(atts);
}
