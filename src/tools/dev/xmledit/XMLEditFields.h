// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#ifndef XMLEDITFIELDS_H
#define XMLEDITFIELDS_H

#include <QFrame>

class XMLDocument;
class QButtonGroup;
class QGroupBox;
class QLineEdit;
class QNarrowLineEdit;
class QButtonGroup;
class QComboBox;
class QCheckBox;
class QListWidget;
class QTextEdit;
class QPushButton;

// ****************************************************************************
//  Class:  XMLEditFields
//
//  Purpose:
//    Field editing widget for the XML editor.
//
//  Programmer:  Jeremy Meredith
//  Creation:    October 17, 2002
//
//  Modifications:
//    Brad Whitlock, Fri Dec 10 10:27:41 PDT 2004
//    Added controls for setting some variable name attributes.
//
//    Brad Whitlock, Wed Feb 28 18:41:56 PST 2007
//    Added public/private
//
//    Cyrus Harrison, Thu May 15 16:00:46 PDT 200
//    First pass at porting to Qt 4.4.0
//
//    Kathleen Biagas, Thu Aug 25 11:28:49 PDT 2011
//    Added persistent.
//
// ****************************************************************************

class XMLEditFields : public QFrame
{
    Q_OBJECT
  public:
    XMLEditFields(QWidget *p);
    void SetDocument(XMLDocument *doc) { xmldoc = doc; }
    void BlockAllSignals(bool);
  public slots:
    void UpdateWindowContents();
    void UpdateWindowSensitivity();
    void UpdateWindowSingleItem();
    void UpdateTypeList();
    void UpdateEnablerList();
    void nameTextChanged(const QString&);
    void labelTextChanged(const QString&);
    void typeChanged(int);
    void subtypeTextChanged(const QString&);
    void enablerChanged(int);
    void enablevalTextChanged(const QString&);
    void lengthTextChanged(const QString&);
    void internalChanged();
    void persistentChanged();
    void ignoreeqChanged();
    void initChanged();
    void valuesChanged();
    void fieldlistNew();
    void fieldlistDel();
    void fieldlistUp();
    void fieldlistDown();
    void variableTypeClicked(int);
    void accessChanged(int);
  private:
    XMLDocument     *xmldoc;

    QPushButton     *newButton;
    QPushButton     *delButton;

    QPushButton     *upButton;
    QPushButton     *downButton;

    QListWidget     *fieldlist;
    QLineEdit       *name;
    QLineEdit       *label;
    QComboBox       *type;
    QLineEdit       *subtype;
    QComboBox       *enabler;
    QLineEdit       *enableval;
    QNarrowLineEdit *length;
    QCheckBox       *internal;
    QCheckBox       *persistent;
    QCheckBox       *ignoreeq;
    QButtonGroup    *access;
    QGroupBox       *accessGroup;
    QGroupBox       *variableNameGroup;
    QButtonGroup    *varNameButtons;
    QCheckBox       *init;
    QTextEdit       *values;
};

#endif
