// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#ifndef QVISLINEOUTWINDOW_H
#define QVISLINEOUTWINDOW_H

#include <QvisOperatorWindow.h>
#include <AttributeSubject.h>

class LineoutAttributes;
class QLabel;
class QCheckBox;
class QLineEdit;
class QButtonGroup;
class QGroupBox;

// ****************************************************************************
// Class: QvisLineoutWindow
//
// Purpose:
//    Defines QvisLineoutWindow class.
//
// Notes:      Autogenerated by xml2window.
//
// Programmer: xml2window
// Creation:   omitted
//
// Modifications:
//   Cyrus Harrison, Tue Aug 19 11:37:56 PDT 2008
//   Qt4 Port. (Removed pre Qt 3.2 logic)
//
// ****************************************************************************

class QvisLineoutWindow : public QvisOperatorWindow
{
    Q_OBJECT
  public:
    QvisLineoutWindow(const int type,
                         LineoutAttributes *subj,
                         const QString &caption = QString::null,
                         const QString &shortName = QString::null,
                         QvisNotepadArea *notepad = 0);
    virtual ~QvisLineoutWindow();
    virtual void CreateWindowContents();
  protected:
    void UpdateWindow(bool doAll);
    virtual void GetCurrentValues(int which_widget);
  private slots:
    void point1ProcessText();
    void point2ProcessText();
    void interactiveChanged(bool val);
    void ignoreGlobalChanged(bool val);
    void samplingOnChanged(bool val);
    void numberOfSamplePointsProcessText();
    void reflineLabelsChanged(bool val);
  private:
    QLineEdit *point1;
    QLineEdit *point2;
    QCheckBox *interactive;
    QGroupBox *ignoreGlobal;
    QCheckBox *samplingOn;
    QLineEdit *numberOfSamplePoints;
    QCheckBox *reflineLabels;
    QLabel *point1Label;
    QLabel *point2Label;
    QLabel *numberOfSamplePointsLabel;

    LineoutAttributes *atts;
};



#endif
