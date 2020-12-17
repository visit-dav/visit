// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#ifndef QVISPERSISTENTPARTICLESWINDOW_H
#define QVISPERSISTENTPARTICLESWINDOW_H

#include <QvisOperatorWindow.h>
#include <AttributeSubject.h>

class PersistentParticlesAttributes;
class QLabel;
class QCheckBox;
class QLineEdit;
class QButtonGroup;
class QvisVariableButton;

// ****************************************************************************
// Class: QvisPersistentParticlesWindow
//
// Purpose:
//    Defines QvisPersistentParticlesWindow class.
//
// Notes:      Autogenerated by xml2window.
//
// Programmer: xml2window
// Creation:   omitted
//
// Modifications:
//   
// ****************************************************************************

class QvisPersistentParticlesWindow : public QvisOperatorWindow
{
    Q_OBJECT
  public:
    QvisPersistentParticlesWindow(const int type,
                         PersistentParticlesAttributes *subj,
                         const QString &caption = QString(),
                         const QString &shortName = QString(),
                         QvisNotepadArea *notepad = 0);
    virtual ~QvisPersistentParticlesWindow();
    virtual void CreateWindowContents();
  protected:
    void UpdateWindow(bool doAll);
    virtual void GetCurrentValues(int which_widget);
  private slots:
    void startIndexProcessText();
    void startPathTypeChanged(int val);
    void stopIndexProcessText();
    void stopPathTypeChanged(int val);
    void updateStopIndexText();
    void updateStartIndexText();
    void strideProcessText();
    void traceVariableXChanged(const QString &varName);
    void traceVariableYChanged(const QString &varName);
    void traceVariableZChanged(const QString &varName);
    void connectParticlesChanged(bool val);
    void showPointsChanged(bool val);
    void indexVariableChanged(const QString &varName);
  private:
    QLineEdit *startIndex;
    QWidget *startPathType;
    QButtonGroup *startPathTypeButtonGroup;
    QLineEdit *stopIndex;
    QWidget *stopPathType;
    QButtonGroup *stopPathTypeButtonGroup;
    QLineEdit *stride;
    QvisVariableButton *traceVariableX;
    QvisVariableButton *traceVariableY;
    QvisVariableButton *traceVariableZ;
    QCheckBox *connectParticles;
    QCheckBox *showPoints;
    QvisVariableButton *indexVariable;
    QLabel *startIndexLabel;
    QLabel *startPathTypeLabel;
    QLabel *stopIndexLabel;
    QLabel *stopPathTypeLabel;
    QLabel *strideLabel;
    QLabel *traceVariableXLabel;
    QLabel *traceVariableYLabel;
    QLabel *traceVariableZLabel;
    QLabel *indexVariableLabel;

    PersistentParticlesAttributes *atts;

    QCheckBox *startIndexRelative;
    QCheckBox *stopIndexRelative;

    void stopIndexRelativeChanged(bool val);
    void startIndexRelativeChanged(bool val);
};



#endif
