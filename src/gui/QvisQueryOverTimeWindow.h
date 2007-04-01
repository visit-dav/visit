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
//   
// ****************************************************************************

class QvisQueryOverTimeWindow : public QvisPostableWindowObserver
{
    Q_OBJECT
  public:
    QvisQueryOverTimeWindow(
                         QueryOverTimeAttributes *subj,
                         const char *caption = 0,
                         const char *shortName = 0,
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
    void startTimeFlagChanged(bool val);
    void startTimeProcessText();
    void endTimeFlagChanged(bool val);
    void endTimeProcessText();
    void strideProcessText();
    void createWindowChanged(bool val);
    void windowIdProcessText();
  private:
    QButtonGroup *timeType;
    QCheckBox *startTimeFlag;
    QLineEdit *startTime;
    QCheckBox *endTimeFlag;
    QLineEdit *endTime;
    QLineEdit *stride;
    QCheckBox *createWindow;
    QLineEdit *windowId;
    QLabel *strideLabel;
    QLabel *createWindowLabel;
    QLabel *windowIdLabel;

    QueryOverTimeAttributes *atts;
};



#endif
