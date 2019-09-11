// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#ifndef QVIS_NOTEPAD_AREA_H
#define QVIS_NOTEPAD_AREA_H
#include <gui_exports.h>
#include <QWidget>
#include <QSizePolicy>
#include <QMap>

// forward declarations
class QvisPostableWindow;
class QTabWidget;
class QVBoxLayout;

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
//   Brad Whitlock, Fri Jun  6 09:36:11 PDT 2008
//   Qt 4.
//
//   Cyrus Harrison, Thu May  6 16:34:02 PDT 2010
//   Added 'avoid_scroll' argument to postWindow.
//
// ****************************************************************************

class GUI_API QvisNotepadArea : public QWidget
{
    Q_OBJECT

    typedef struct
    {
        QWidget     *parent;
        QVBoxLayout *parentLayout;
        bool         parentIsScrollView;
        int          minWidth;
        int          minHeight;
    } PostedInfo;

    typedef QMap<QWidget*, PostedInfo> PostedInfoLookup;
public:
    QvisNotepadArea(QWidget *parent = 0);
    virtual ~QvisNotepadArea();
    void showPage(QvisPostableWindow *pw);
    void postWindow(QvisPostableWindow *pw, bool avoid_scroll=false);
private:
    int               numPosted;
    QTabWidget       *tabs;
    QWidget          *empty;
    PostedInfoLookup  postedLookup;
};

#endif
