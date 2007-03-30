#ifndef QVIS_NOTEPAD_AREA
#define QVIS_NOTEPAD_AREA
#include <gui_exports.h>
#include <qvbox.h>
#include <qsizepolicy.h>
#include <qmap.h>

// forward declarations
class QvisPostableWindow;
class QTabWidget;

// ****************************************************************************
// Class: QvisNotepadArea
//
// Purpose:
//   This class allows QvisPostableWindows to be posted into it.
//
// Notes:      
//
// Programmer: Brad Whitlock
// Creation:   Tue Jul 25 17:56:27 PST 2000
//
// Modifications:
//   Brad Whitlock, Thu Sep 6 15:15:59 PST 2001
//   Added a method to handle paletteChange events.
//
//   Brad Whitlock, Tue Sep 25 15:16:59 PST 2001
//   Changed an internal data structure and removed the method to handle
//   paletteChange events since I found a better way to do it.
//
// ****************************************************************************

class GUI_API QvisNotepadArea : public QVBox
{
    Q_OBJECT

    typedef struct
    {
        QWidget *parent;
        bool     parentIsScrollView;
        int      minWidth;
        int      minHeight;
    } PostedInfo;

    typedef QMap<QWidget*, PostedInfo> PostedInfoLookup;
public:
    QvisNotepadArea(QWidget *parent = 0, const char *name = 0);
    virtual ~QvisNotepadArea();
    void showPage(QvisPostableWindow *pw);
    void postWindow(QvisPostableWindow *pw);
private:
    int              numPosted;
    QTabWidget       *tabs;
    QWidget          *empty;
    PostedInfoLookup postedLookup;
};

#endif
