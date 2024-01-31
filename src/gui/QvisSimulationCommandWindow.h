// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#ifndef QVIS_SIMULATION_COMMAND_WINDOW_H
#define QVIS_SIMULATION_COMMAND_WINDOW_H
#include <QvisPostableWindow.h>

class QButtonGroup;
class QGridLayout;
class QGroupBox;
class QLabel;
class QLineEdit;
class QPushButton;

// ****************************************************************************
// Class: QvisSimulationCommandWindow
//
// Purpose:
//   Shows messages in the simulation window.
//
// Notes:      
//
// Programmer: Brad Whitlock
// Creation:   Tue Jul  8 13:26:51 PDT 2008
//
// Modifications:
//    Brad Whitlock, Wed Aug  5 17:25:48 PDT 2015
//    Let the window have an arbitrary number of simulation command buttons.
//
// ****************************************************************************

class QvisSimulationCommandWindow : public QvisPostableWindow
{
    Q_OBJECT
public:
    QvisSimulationCommandWindow(const QString &caption = QString(),
                                const QString &shortName = QString(),
                                QvisNotepadArea *notepad = 0);
    virtual ~QvisSimulationCommandWindow();
    virtual void CreateEntireWindow();

    int  numCommandButtons() const;
    bool setButtonCommand(int index, const QString &cmd);
    bool setButtonEnabled(int index, bool enabled, bool clearText);
    void setCustomButtonEnabled(bool);
    void setTimeValues(bool timeRanging, 
                       const QString &start, 
                       const QString &stop,
                       const QString &step);

    void setTimeRanging(bool timeRanging);
    void setTimeStart(const QString &start);
    void setTimeStep(const QString &step);
    void setTimeStop(const QString &stop);

signals:
    void executeButtonCommand(const QString &cmd);
    void showCustomUIWindow();
    void timeRangingToggled(const QString &);
    void executeStart(const QString &);
    void executeStop(const QString &);
    void executeStep(const QString &);
public slots:
    void post();
    void unpost();
private slots:
    void handleTimeRanging(bool);
    void handleStart(const QString&);
    void handleStop(const QString&);
    void handleStep(const QString&);
    void handleCommandButton(int);
protected:
    virtual void CreateWindowContents();
    bool EnsureButtonExists(int index, bool &added);
private:
    QGroupBox          *timeGroup;
    QLineEdit          *startCycle;
    QLineEdit          *stopCycle;
    QLineEdit          *stepCycle;
    QLabel             *startLabel;
    QLabel             *stopLabel;
    QLabel             *stepLabel;

    QGroupBox          *commandGroupBox;
    QWidget            *commandButtonParent;
    QButtonGroup       *commandGroup;
    QGridLayout        *commandButtonLayout;
    QPushButton        *activateCustomGUI;
};

#endif
