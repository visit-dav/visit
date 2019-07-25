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
    void nameTextChanged(const QString&);
    void targetTextChanged(const QString&);
    void memberChanged();
    void declarationTextChanged(const QString&);
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
