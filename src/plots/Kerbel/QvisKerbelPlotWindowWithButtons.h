#ifndef QVISKERBELPLOTWINDOWWITHBUTTONS_H
#define QVISKERBELPLOTWINDOWWITHBUTTONS_H

#include <QvisKerbelPlotWindow.h>
#include <ConfigManager.h>

class QvisKerbelPlotWindowWithButtons:
    public QvisKerbelPlotWindow, public ConfigManager
{
    Q_OBJECT
public:
    QvisKerbelPlotWindowWithButtons(const int type, KerbelAttributes *subj,
                                    const char *caption = 0,
                                    const char *shortName = 0,
                                    QvisNotepadArea *notepad = 0);

    virtual void CreateWindowContents();
    virtual void WriteConfigFile(const char *filename) {}; // Not used, but needed by superclass
    virtual DataNode* ReadConfigFile(const char *filename) {return NULL;}; // Not used, but needed by superclass

protected slots:
    void LoadSlot();
    void SaveSlot();

private:
    KerbelAttributes *atts;
};

#endif
