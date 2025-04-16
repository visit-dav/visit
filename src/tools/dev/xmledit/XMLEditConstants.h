// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#ifndef XMLEDITCONSTANTS_H
#define XMLEDITCONSTANTS_H

#include <QFrame>

class XMLDocument;
class QLineEdit;
class QButtonGroup;
class QComboBox;
class QCheckBox;
class QListWidget;
class QTextEdit;
class QPushButton;

// ****************************************************************************
//  Class:  XMLEditConstants
//
//  Purpose:
//    Constant editing widget for the XML editor.
//
//  Programmer:  Jeremy Meredith
//  Creation:    October 17, 2002
//
//  Modifications:
//    Brad Whitlock, Thu Mar 6 16:10:40 PST 2008
//    Added target.
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

class XMLEditConstants : public QFrame
{
    Q_OBJECT
  public:
    XMLEditConstants(QWidget *p);
    void SetDocument(XMLDocument *doc) { xmldoc = doc; }
    void BlockAllSignals(bool);
  public slots:
    void UpdateWindowContents();
    void UpdateWindowSensitivity();
    void UpdateWindowSingleItem();
    void nameTextChanged();
    void nameTextChanged(const QString&);
    void targetTextChanged();
    void memberChanged();
    void declarationTextChanged();
    void definitionChanged();
    void constantlistNew();
    void constantlistDel();
  private:
    int CountConstants(const QString &) const;

    XMLDocument     *xmldoc;

    QPushButton    *newButton;
    QPushButton    *delButton;

    QListWidget    *constantlist;
    QLineEdit      *target;
    QLineEdit      *name;
    QCheckBox      *member;
    QLineEdit      *declaration;
    QTextEdit      *definition;
};

#endif
