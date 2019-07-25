// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#ifndef XMLEDITENUMS_H
#define XMLEDITENUMS_H

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
//  Class:  XMLEditEnums
//
//  Purpose:
//    Enum editing widget for the XML editor.
//
//  Programmer:  Jeremy Meredith
//  Creation:    October 17, 2002
//
// Modifications:
//   Cyrus Harrison, Thu May 15 15:04:20 PDT 2008
//   Ported to Qt 4.4
//
// ****************************************************************************
class XMLEditEnums : public QFrame
{
    Q_OBJECT
  public:
    XMLEditEnums(QWidget *p);
    void SetDocument(XMLDocument *doc) { xmldoc = doc; }
    void BlockAllSignals(bool);
  public slots:
    void removeEmptyLines();
    void addEmptyLine();
    void UpdateWindowContents();
    void UpdateWindowSensitivity();
    void UpdateWindowSingleItem();
    void nameTextChanged(const QString&);
    void valuelistChanged();
    void enumlistNew();
    void enumlistDel();
  private:
    XMLDocument     *xmldoc;

    QPushButton     *newButton;
    QPushButton     *delButton;

    QListWidget     *enumlist;
    QLineEdit       *name;
    QTextEdit       *valuelist;
};

#endif
