#ifndef QVIS_DELAYED_WINDOW_OBSERVER_H
#define QVIS_DELAYED_WINDOW_OBSERVER_H
#include <gui_exports.h>
#include <QvisDelayedWindow.h>
#include <Observer.h>

class GUI_API QvisDelayedWindowObserver : public QvisDelayedWindow, public Observer
{
    Q_OBJECT
public:
    QvisDelayedWindowObserver(Subject *subj, const char *caption = 0);
    virtual ~QvisDelayedWindowObserver();
    virtual void CreateWindowContents() = 0;
    virtual void Update(Subject *TheChangedSubject);
};

#endif
