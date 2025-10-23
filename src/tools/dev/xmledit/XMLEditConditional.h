// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#ifndef XMLEDITCONDITIONAL_H
#define XMLEDITCONDITIONAL_H

#include <QFrame>

class XMLDocument;
class QLabel;
class QLineEdit;
class QListWidget;
class QPushButton;

// ****************************************************************************
//  Class:  XMLEditConditional
//
//  Purpose:
//    Code snippet editing widget for the XML editor.
//
//  Programmer:  Kathleen Biagas 
//  Creation:    Sep 29, 2025
//
//  Modifications:
//    Kathleen Biagas, Thu Oct 9, 2025
//    Add support for cxxflags.
//
//    Kathleen Biagas, Tue Oct 21, 2025
//    Add support for vlinklibs.
//
//    Kathleen Biagas, Thu Oct 23, 2025
//    Changed use of QTextEdit to QLineEdit. Added QLabel ivars to assist
//    with visuals when widgets are disabled.
//
// ****************************************************************************

class XMLEditConditional : public QFrame
{
    Q_OBJECT
  public:
    XMLEditConditional(QWidget *p);
    void SetDocument(XMLDocument *doc) { xmldoc = doc; }
    void BlockAllSignals(bool);
  public slots:
    void UpdateWindowContents();
    void UpdateWindowSensitivity();
    void UpdateWindowSingleItem();
    void conditionTextChanged();
    void conditionTextChanged(const QString&);
    void definitionsChanged();
    void cxxflagsChanged();
    void mlinklibsChanged();
    void vlinklibsChanged();
    void elinklibsChanged();
    void conditionlistNew();
    void conditionlistDel();
  private:
    int CountConditions(const QString &) const;

    XMLDocument     *xmldoc;

    QPushButton    *newButton;
    QPushButton    *delButton;

    QListWidget    *conditionList;

    QLabel         *conditionLabel;
    QLineEdit      *condition;

    QLabel         *targetLabel;
    QLineEdit      *target;

    QLabel         *defLabel;
    QLineEdit      *definitions;

    QLabel         *flagsLabel;
    QLineEdit      *cxxflags;

    QLabel         *mlinkLabel;
    QLineEdit      *mlinklibs;

    QLabel         *vlinkLabel;
    QLineEdit      *vlinklibs;

    QLabel         *elinkLabel;
    QLineEdit      *elinklibs;
};

#endif
