// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#ifndef XMLEDITCODE_H
#define XMLEDITCODE_H

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
//  Class:  XMLEditCode
//
//  Purpose:
//    Code snippet editing widget for the XML editor.
//
//  Programmer:  Jeremy Meredith
//  Creation:    October 17, 2002
//
//  Modifications:
//    Brad Whitlock, Thu Mar 6 16:00:06 PST 2008
//    Added target support.
//
//    Cyrus Harrison, Thu May 15 16:00:46 PDT 200
//    First pass at porting to Qt 4.4.0
//
// ****************************************************************************
class XMLEditCode : public QFrame
{
    Q_OBJECT
  public:
    XMLEditCode(QWidget *p);
    void SetDocument(XMLDocument *doc) { xmldoc = doc; }
    void BlockAllSignals(bool);
  public slots:
    void UpdateWindowContents();
    void UpdateWindowSensitivity();
    void UpdateWindowSingleItem();
    void targetTextChanged(const QString&);
    void nameTextChanged(const QString&);
    void prefixChanged();
    void postfixChanged();
    void codelistNew();
    void codelistDel();
  private:
    int CountCodes(const QString &) const;

    XMLDocument     *xmldoc;

    QPushButton    *newButton;
    QPushButton    *delButton;

    QListWidget    *codelist;
    QLineEdit      *name;
    QLineEdit      *target;
    QTextEdit      *prefix;
    QTextEdit      *postfix;
};

#endif
