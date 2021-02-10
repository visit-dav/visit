// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#ifndef QVISDEFEREXPRESSIONWINDOW_H
#define QVISDEFEREXPRESSIONWINDOW_H

#include <QvisOperatorWindow.h>
#include <AttributeSubject.h>

class DeferExpressionAttributes;
class QLineEdit;
class QvisVariableButton;

// ****************************************************************************
// Class: QvisDeferExpressionWindow
//
// Purpose: 
//   Defines QvisDeferExpressionWindow class.
//
// Notes:      This class was automatically generated!

// Programmer: xml2window
// Creation:   Tue Sep 20 13:25:13 PST 2005
//
// Modifications:
//   
// ****************************************************************************

class QvisDeferExpressionWindow : public QvisOperatorWindow
{
    Q_OBJECT
  public:
    QvisDeferExpressionWindow(const int type,
                              DeferExpressionAttributes *subj,
                              const QString &caption = QString(),
                              const QString &shortName = QString(),
                              QvisNotepadArea *notepad = 0);
    virtual ~QvisDeferExpressionWindow();
    virtual void CreateWindowContents();
  protected:
    void UpdateWindow(bool doAll);
    virtual void GetCurrentValues(int which_widget);
  private slots:
    void addVariable(const QString &);
    void variableProcessText();
  private:

    DeferExpressionAttributes *atts;
    QvisVariableButton        *varsButton;
    QLineEdit                 *varsLineEdit;
};



#endif
