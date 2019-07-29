// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#ifndef QVISEXTRUDEWINDOW_H
#define QVISEXTRUDEWINDOW_H

#include <QvisOperatorWindow.h>
#include <AttributeSubject.h>

class ExtrudeAttributes;
class QLabel;
class QCheckBox;
class QLineEdit;
class QvisVariableButton;

// ****************************************************************************
// Class: QvisExtrudeWindow
//
// Purpose:
//    Defines QvisExtrudeWindow class.
//
// Notes:      Autogenerated by xml2window.
//
// Programmer: xml2window
// Creation:   omitted
//
// Modifications:
//   
// ****************************************************************************

class QvisExtrudeWindow : public QvisOperatorWindow
{
    Q_OBJECT
  public:
    QvisExtrudeWindow(const int type,
                         ExtrudeAttributes *subj,
                         const QString &caption = QString::null,
                         const QString &shortName = QString::null,
                         QvisNotepadArea *notepad = 0);
    virtual ~QvisExtrudeWindow();
    virtual void CreateWindowContents();
  protected:
    void UpdateWindow(bool doAll);
    virtual void GetCurrentValues(int which_widget);
  private slots:
    void axisProcessText();
    void byVariableChanged(bool val);
    void variableChanged(const QString &varName);
    void lengthProcessText();
    void stepsProcessText();
    void preserveOriginalCellNumbersChanged(bool val);
  private:
    QLineEdit *axis;
    QLineEdit *length;
    QCheckBox *byVariable;
    QvisVariableButton *variable;
    QLineEdit *steps;
    QCheckBox *preserveOriginalCellNumbers;
    QLabel *axisLabel;
    QLabel *lengthLabel;
    QLabel *stepsLabel;

    ExtrudeAttributes *atts;
};

#endif
