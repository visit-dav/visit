// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#ifndef QVISZONEDUMPWINDOW_H
#define QVISZONEDUMPWINDOW_H

#include <QvisOperatorWindow.h>
#include <AttributeSubject.h>

class ZoneDumpAttributes;
class QLabel;
class QCheckBox;
class QLineEdit;
class QvisVariableButton;

// ****************************************************************************
// Class: QvisZoneDumpWindow
//
// Purpose: 
//   Defines QvisZoneDumpWindow class.
//
// Notes:      This class was automatically generated!

// Programmer: xml2window
// Creation:   Wed Apr 4 08:13:39 PDT 2007
//
// Modifications:
//   
// ****************************************************************************

class QvisZoneDumpWindow : public QvisOperatorWindow
{
    Q_OBJECT
  public:
    QvisZoneDumpWindow(const int type,
                         ZoneDumpAttributes *subj,
                         const QString &caption = QString(),
                         const QString &shortName = QString(),
                         QvisNotepadArea *notepad = 0);
    virtual ~QvisZoneDumpWindow();
    virtual void CreateWindowContents();
  protected:
    void UpdateWindow(bool doAll);
    virtual void GetCurrentValues(int which_widget);
  private slots:
    void variableChanged(const QString &varName);
    void lowerBoundProcessText();
    void upperBoundProcessText();
    void outputFileProcessText();
    void enabledChanged(bool val);
  private:
    QvisVariableButton *variable;
    QLineEdit *lowerBound;
    QLineEdit *upperBound;
    QLineEdit *outputFile;
    QCheckBox *enabled;
    QLabel *variableLabel;
    QLabel *lowerBoundLabel;
    QLabel *upperBoundLabel;
    QLabel *outputFileLabel;
    QLabel *enabledLabel;

    ZoneDumpAttributes *atts;
};



#endif
