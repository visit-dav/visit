// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#ifndef QVIS_DB_OPTIONS_DIALOG_H
#define QVIS_DB_OPTIONS_DIALOG_H
#include <QDialog>
#include <QLabel>

class QPushButton;
class QCheckBox;
class QLineEdit;
class QComboBox;
class QPlainTextEdit;
class DBOptionsAttributes;

// ****************************************************************************
// Class: QvisDBOptionsDialog
//
// Purpose:
//   A dialog showing DB options.
//
// Notes:
//
// Programmer: Jeremy Meredith
// Creation:   July 19, 2007
//
// Modifications:
//   Cyrus Harrison, Tue Jun 24 11:15:28 PDT 2008
//   Initial Qt4 Port.
//
//    Mark C. Miller, Thu Dec 18 13:19:38 PST 2014
//    Added helpClicked and helpButton.
//
//    Kathleen Biagas, Tue Sep 13, 2022
//    Use QPlainTextEdit for MultiLineString instead of QTextEdit, so that
//    multiple lines are properly displayed.
//
// ****************************************************************************

class QvisDBOptionsDialog : public QDialog
{
    Q_OBJECT
public:
    QvisDBOptionsDialog(DBOptionsAttributes *opts, QWidget *parent = 0);
    virtual ~QvisDBOptionsDialog();
public slots:
    void okayClicked();
    void helpClicked();

private:
    DBOptionsAttributes *atts;

    QList<QCheckBox*>       checkboxes;
    QList<QLineEdit*>       lineedits;
    QList<QComboBox*>       comboboxes;
    QList<QPlainTextEdit*>  multiLineEdits;

    QPushButton         *okButton;
    QPushButton         *helpButton;
    QPushButton         *cancelButton;
};

#endif
