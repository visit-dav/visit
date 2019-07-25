// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#ifndef QVISINTERPRETER_H
#define QVISINTERPRETER_H
#include <gui_exports.h>
#include <QObject>
#include <GUIBase.h>
#include <deque>

// ****************************************************************************
// Class: QvisInterpreter
//
// Purpose:
//   This class provides an interpreter interface.
//
// Notes:      
//
// Programmer: Brad Whitlock
// Creation:   Fri May 6 09:41:51 PDT 2005
//
// Modifications:
//   Brad Whitlock, Mon Apr 7 11:41:21 PDT 2008
//   Made Initialize be public.
//
//   Brad Whitlock, Fri May 30 15:41:19 PDT 2008
//   Qt 4.
//
// ****************************************************************************

class GUI_API QvisInterpreter : public QObject, public GUIBase
{
    Q_OBJECT
public:
    QvisInterpreter(QObject *parent = 0);
    virtual ~QvisInterpreter();
    bool Initialize();
signals:
    void Synchronize();
public slots:
    void Interpret(const QString &s);
    void ProcessCommands();
private:
    bool NoInterpretingClient() const;

    std::deque<std::string> pendingCommands;
};

#endif
