// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#ifndef QVISDISPLACEWINDOW_H
#define QVISDISPLACEWINDOW_H

#include <QvisOperatorWindow.h>
#include <AttributeSubject.h>

class DisplaceAttributes;
class QLabel;
class QLineEdit;
class QvisVariableButton;

// ****************************************************************************
// Class: QvisDisplaceWindow
//
// Purpose:
//    Defines QvisDisplaceWindow class.
//
// Notes:      Autogenerated by xml2window.
//
// Programmer: xml2window
// Creation:   omitted
//
// Modifications:
//   
// ****************************************************************************

class QvisDisplaceWindow : public QvisOperatorWindow
{
    Q_OBJECT
  public:
    QvisDisplaceWindow(const int type,
                         DisplaceAttributes *subj,
                         const QString &caption = QString::null,
                         const QString &shortName = QString::null,
                         QvisNotepadArea *notepad = 0);
    virtual ~QvisDisplaceWindow();
    virtual void CreateWindowContents();
  protected:
    void UpdateWindow(bool doAll);
    virtual void GetCurrentValues(int which_widget);
  private slots:
    void factorProcessText();
    void variableChanged(const QString &varName);
  private:
    QLineEdit *factor;
    QvisVariableButton *variable;
    QLabel *factorLabel;
    QLabel *variableLabel;

    DisplaceAttributes *atts;
};



#endif
