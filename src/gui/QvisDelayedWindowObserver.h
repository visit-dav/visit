// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#ifndef QVIS_DELAYED_WINDOW_OBSERVER_H
#define QVIS_DELAYED_WINDOW_OBSERVER_H
#include <gui_exports.h>
#include <QvisDelayedWindow.h>
#include <Observer.h>

class GUI_API QvisDelayedWindowObserver : public QvisDelayedWindow, public Observer
{
    Q_OBJECT
public:
    QvisDelayedWindowObserver(Subject *subj, const QString &caption);
    virtual ~QvisDelayedWindowObserver();
    virtual void CreateWindowContents() = 0;
    virtual void Update(Subject *TheChangedSubject);
};

#endif
