#ifndef QVIS_INVERSE_GHOST_ZONE_WINDOW_H
#define QVIS_INVERSE_GHOST_ZONE_WINDOW_H
#include <QvisOperatorWindow.h>

class QCheckBox;
class QLineEdit;
class QLabel;
class InverseGhostZoneAttributes;

// ****************************************************************************
// Class: QvisInverseGhostZoneWindow
//
// Purpose:
//   This class is a postable window that watches the inverse
//   ghost zone operator attributes and always represents their current
//   state.
//
// Programmer: Hank Childs
// Creation:   July 27, 2001
//
// Modifications:
//    Brad Whitlock, Fri Apr 12 13:06:55 PST 2002
//    Made it inherit from QvisOperatorWindow.
//
// ****************************************************************************

class QvisInverseGhostZoneWindow : public QvisOperatorWindow
{
    Q_OBJECT
public:
    QvisInverseGhostZoneWindow(const int type,
                             InverseGhostZoneAttributes *subj,
                             const char *caption = 0,
                             const char *shortName = 0,
                             QvisNotepadArea *notepad = 0);
    virtual ~QvisInverseGhostZoneWindow();
protected:
    virtual void CreateWindowContents();
    void UpdateWindow(bool doAll);
private slots:
    void constantDataToggled(bool);    
private:
    QCheckBox *constantData;

    InverseGhostZoneAttributes *igzAtts;
};
#endif
