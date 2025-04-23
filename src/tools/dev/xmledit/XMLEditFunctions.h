// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#ifndef XMLEDITFUNCTIONS_H
#define XMLEDITFUNCTIONS_H

#include <QFrame>

class XMLDocument;
class QLabel;
class QLineEdit;
class QButtonGroup;
class QComboBox;
class QCheckBox;
class QListWidget;
class QTextEdit;
class QRadioButton;
class QPushButton;

// ****************************************************************************
//  Class:  XMLEditFunctions
//
//  Purpose:
//    Function editing widget for the XML editor.
//
//  Programmer:  Jeremy Meredith
//  Creation:    October 17, 2002
//
//  Modifications:
//    Brad Whitlock, Thu Mar 6 15:45:09 PST 2008
//    Added target.
//
//    Brad Whitlock, Mon Apr 28 15:59:21 PDT 2008
//    Added accessGroup.
//
//    Cyrus Harrison, Thu May 15 16:00:46 PDT 200
//    First pass at porting to Qt 4.4.0
//
//    Kathleen Biagas, Fri Mar 21, 2025
//    Removed QString arg from xxxTextChanged slots as they are now connected
//    to 'editingFinished' signal. nameTextChanged retains its arg as it
//    can be called with the arg from another function.
//
//    Kathleen Biagas, Wed April 16, 2025
//    Add no-arg nameTextChanged to match editingFinished signal.
//
// ****************************************************************************

class XMLEditFunctions : public QFrame
{
    Q_OBJECT
  public:
    XMLEditFunctions(QWidget *p);
    void SetDocument(XMLDocument *doc) { xmldoc = doc; }
    void BlockAllSignals(bool);
  public slots:
    void UpdateWindowContents();
    void UpdateWindowSensitivity();
    void UpdateWindowSingleItem();
    void nameTextChanged();
    void nameTextChanged(const QString&);
    void typeGroupChanged(int);
    void accessChanged(int);
    void memberChanged();
    void targetTextChanged();
    void declarationTextChanged();
    void definitionChanged();
    void functionlistNew();
    void functionlistDel();
  private:
    int CountFunctions(const QString &) const;

    XMLDocument    *xmldoc;

    QPushButton    *newButton;
    QPushButton    *delButton;

    QListWidget    *functionlist;
    QLineEdit      *name;
    QButtonGroup   *typeGroup;
    QRadioButton   *newFunctionButton;
    QRadioButton   *replaceBuiltinButton;
    QCheckBox      *member;
    QLineEdit      *target;
    QLabel         *accessLabel;
    QComboBox      *access;
    QLineEdit      *declaration;
    QTextEdit      *definition;
};

#endif
