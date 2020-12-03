// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#ifndef QVISISOSURFACEWINDOW_H
#define QVISISOSURFACEWINDOW_H

#include <QvisOperatorWindow.h>
#include <AttributeSubject.h>

class IsosurfaceAttributes;
class QCheckBox;
class QComboBox;
class QLineEdit;
class QButtonGroup;
class QvisVariableButton;

// ****************************************************************************
// Class: QvisIsosurfaceWindow
//
// Purpose: 
//   Defines QvisIsosurfaceWindow class.
//
// Notes:      This class was automatically generated!

// Programmer: xml2window
// Creation:   Tue Apr 16 17:41:29 PST 2002
//
// Modifications:
//   Brad Whitlock, Thu Dec 9 17:44:32 PST 2004
//   I made the window use a variable button.
//
//   Mark C. Miller, Wed Nov 16 10:46:36 PST 2005
//   Moved StringToDoubleList to QvisWindowBase 
// ****************************************************************************

class QvisIsosurfaceWindow : public QvisOperatorWindow
{
    Q_OBJECT
  public:
    QvisIsosurfaceWindow(const int type,
                         IsosurfaceAttributes *subj,
                         const QString &caption = QString(),
                         const QString &shortName = QString(),
                         QvisNotepadArea *notepad = 0);
    virtual ~QvisIsosurfaceWindow();
    virtual void CreateWindowContents();
  protected:
    void UpdateWindow(bool doAll);
    virtual void GetCurrentValues(int which_widget);
    void UpdateSelectByText();
 
    QString LevelString(int i);
    void ProcessSelectByText();
  private slots:
    void scaleClicked(int scale);
    void selectByChanged(int);
    void processSelectByText();
    void minToggled(bool);
    void processMinLimitText();
    void maxToggled(bool);
    void processMaxLimitText();
    void variableChanged(const QString &);
  private:
    QComboBox              *selectByComboBox;
    QLineEdit              *selectByLineEdit;
    QButtonGroup           *scalingButtons;
    QCheckBox              *minToggle;
    QLineEdit              *minLineEdit;
    QCheckBox              *maxToggle;
    QLineEdit              *maxLineEdit;
    QvisVariableButton     *variable;

    IsosurfaceAttributes *atts;
};



#endif
