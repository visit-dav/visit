// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#ifndef XMLEDITINCLUDES_H
#define XMLEDITINCLUDES_H

#include <QFrame>

class XMLDocument;
class QLineEdit;
class QButtonGroup;
class QComboBox;
class QCheckBox;
class QListWidget;
class QTextEdit;
class QRadioButton;
class QPushButton;

// ****************************************************************************
//  Class:  XMLEditIncludes
//
//  Purpose:
//    Include editing widget for the XML editor.
//
//  Programmer:  Jeremy Meredith
//  Creation:    October 17, 2002
//
//  Modifications:
//    Brad Whitlock, Thu Mar 6 16:20:35 PST 2008
//    Added target.
//
//    Cyrus Harrison, Thu May 15 16:00:46 PDT 200
//    First pass at porting to Qt 4.4.0
//
// ****************************************************************************
class XMLEditIncludes : public QFrame
{
    Q_OBJECT
  public:
    XMLEditIncludes(QWidget *p);
    void SetDocument(XMLDocument *doc) { xmldoc = doc; }
    void BlockAllSignals(bool);
  public slots:
    void UpdateWindowContents();
    void UpdateWindowSensitivity();
    void UpdateWindowSingleItem();
    void targetTextChanged(const QString&);
    void includeTextChanged(const QString&);
    void fileGroupChanged(int);
    void quotedGroupChanged(int);
    void includelistNew();
    void includelistDel();
  private:
    int CountIncludes(const QString &) const;

    XMLDocument    *xmldoc;

    QPushButton    *newButton;
    QPushButton    *delButton;

    QListWidget       *includelist;
    QRadioButton   *CButton;
    QRadioButton   *HButton;
    QRadioButton   *quotesButton;
    QRadioButton   *bracketsButton;
    QButtonGroup   *fileGroup;
    QButtonGroup   *quotedGroup;
    QLineEdit      *file;
    QLineEdit      *target;
};

#endif
