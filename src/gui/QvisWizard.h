#ifndef QVIS_WIZARD_H
#define QVIS_WIZARD_H
#include <qwizard.h>
#include <GUIBase.h>
#include <gui_exports.h>

class AttributeSubject;

// ****************************************************************************
// Class: QvisWizard
//
// Purpose:
//   This a base class for wizards in VisIt.
//
// Notes:      
//
// Programmer: Brad Whitlock
// Creation:   Tue Dec 14 09:36:43 PDT 2004
//
// Modifications:
//   
// ****************************************************************************

class GUI_API QvisWizard : public QWizard, public GUIBase
{
    Q_OBJECT
public:
    QvisWizard(AttributeSubject *subj, QWidget *parent, const char *name = 0);
    virtual ~QvisWizard();
    virtual void SendAttributes();
protected:
    AttributeSubject *atts;
    AttributeSubject *localCopy;
};

#endif
