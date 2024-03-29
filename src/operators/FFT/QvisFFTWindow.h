// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#ifndef QVISFFTWINDOW_H
#define QVISFFTWINDOW_H

#include <QvisOperatorWindow.h>
#include <AttributeSubject.h>

class FFTAttributes;
class QLabel;
class QLineEdit;

// ****************************************************************************
// Class: QvisFFTWindow
//
// Purpose:
//    Defines QvisFFTWindow class.
//
// Notes:      Autogenerated by xml2window.
//
// Programmer: xml2window
// Creation:   omitted
//
// Modifications:
//   
// ****************************************************************************

class QvisFFTWindow : public QvisOperatorWindow
{
    Q_OBJECT
  public:
    QvisFFTWindow(const int type,
                         FFTAttributes *subj,
                         const QString &caption = QString(),
                         const QString &shortName = QString(),
                         QvisNotepadArea *notepad = 0);
    virtual ~QvisFFTWindow();
    virtual void CreateWindowContents();
  protected:
    void UpdateWindow(bool doAll);
    virtual void GetCurrentValues(int which_widget);
  private slots:
    void dummyProcessText();
  private:
    QLineEdit *dummy;
    QLabel *dummyLabel;

    FFTAttributes *atts;
};



#endif
