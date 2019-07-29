// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#ifndef XMLEDIT_CODESELECTION_DIALOG_H
#define XMLEDIT_CODESELECTION_DIALOG_H
#include <QDialog>
#include <XMLToolIds.h>

class QCheckBox;

// ****************************************************************************
// Class: XMLEditCodeSelectionDialog
//
// Purpose: 
//   Dialog that lets you select the code tools that you want to run.
//
// Programmer: Brad Whitlock
// Creation:   Fri Mar 7 16:17:39 PST 2008
//
// Modifications:
//   
// ****************************************************************************

class XMLEditCodeSelectionDialog : public QDialog
{
    Q_OBJECT
public:
    XMLEditCodeSelectionDialog(QWidget *parent);
    virtual ~XMLEditCodeSelectionDialog();

    static bool selectTools(const QString &caption, bool *useTools, 
                            const bool *toolEnabled);
private:
    QCheckBox *buttons[ID_XML_MAX];
};

#endif
