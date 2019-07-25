// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#ifndef QVIS_WIZARD_H
#define QVIS_WIZARD_H
#include <QWizard>
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
//   Cyrus Harrison, Tue Jun 10 09:20:47 PDT 2008
//   Initial Qt4 Port.
//
// ****************************************************************************

class GUI_API QvisWizard : public QWizard, public GUIBase
{
    Q_OBJECT
public:
    QvisWizard(AttributeSubject *subj, QWidget *parent);
    virtual ~QvisWizard();
    virtual void SendAttributes();
    bool UpdateAttributes();
protected:
    AttributeSubject *atts;
    AttributeSubject *localCopy;
};

#endif
