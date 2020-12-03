// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#ifndef QVIS_SIMULATION_MESSAGE_WINDOW_H
#define QVIS_SIMULATION_MESSAGE_WINDOW_H
#include <QvisPostableWindow.h>

class QTextEdit;

// ****************************************************************************
// Class: QvisSimulationMessageWindow
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
//   
// ****************************************************************************

class QvisSimulationMessageWindow : public QvisPostableWindow
{
    Q_OBJECT
public:
    QvisSimulationMessageWindow(const QString &caption = QString(),
                                const QString &shortName = QString(),
                                QvisNotepadArea *notepad = 0);
    virtual ~QvisSimulationMessageWindow();
    virtual void CreateEntireWindow();
public slots:
    void addMessage(const QString &txt);
    void clear();
    void post();
    void unpost();
protected:
    virtual void CreateWindowContents();
private:
    QTextEdit *messages;
};

#endif
