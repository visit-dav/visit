#include <QvisDelayedWindowObserver.h>

QvisDelayedWindowObserver::QvisDelayedWindowObserver(Subject *subj,
    const char *caption) : QvisDelayedWindow(caption), Observer(subj)
{
    // nothing here.
}

QvisDelayedWindowObserver::~QvisDelayedWindowObserver()
{
    // nothing here.
}

void
QvisDelayedWindowObserver::Update(Subject *)
{
    if(isCreated)
        UpdateWindow(false);
}
