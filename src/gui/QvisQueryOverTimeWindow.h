// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#ifndef QVISQUERYOVERTIMEWINDOW_H
#define QVISQUERYOVERTIMEWINDOW_H

#include <AttributeSubject.h>
#include <QvisPostableWindowObserver.h>

class QueryOverTimeAttributes;
class QLabel;
class QCheckBox;
class QLineEdit;
class QButtonGroup;

// ****************************************************************************
// Class: QvisQueryOverTimeWindow
//
// Purpose: 
//   Defines QvisQueryOverTimeWindow class.
//
// Notes:      This class was automatically generated!

// Programmer: xml2window
// Creation:   Wed Mar 31 08:46:20 PDT 2004
//
// Modifications:
//   Brad Whitlock, Wed Apr  9 11:32:17 PDT 2008
//   QString for caption, shortName.
//
//   Kathleen Biagas, Fri Aug 26 17:12:13 PDT 2011
//   Removed start/end times and stride.
//
// ****************************************************************************

class QvisQueryOverTimeWindow : public QvisPostableWindowObserver
{
    Q_OBJECT
  public:
    QvisQueryOverTimeWindow(
                         QueryOverTimeAttributes *subj,
                         const QString &caption = QString(),
                         const QString &shortName = QString(),
                         QvisNotepadArea *notepad = 0);
    virtual ~QvisQueryOverTimeWindow();
    virtual void CreateWindowContents();
  public slots:
    virtual void apply();
    virtual void makeDefault();
    virtual void reset();
  protected:
    void UpdateWindow(bool doAll);
    void GetCurrentValues(int which_widget);
    void Apply(bool ignore = false);
  private slots:
    void timeTypeChanged(int val);
    void createWindowChanged(bool val);
    void windowIdProcessText();
  private:
    QButtonGroup *timeType;
    QCheckBox *createWindow;
    QLineEdit *windowId;
    QLabel *strideLabel;
    QLabel *createWindowLabel;
    QLabel *windowIdLabel;

    QueryOverTimeAttributes *atts;
};



#endif
