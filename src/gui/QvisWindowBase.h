#ifndef QVIS_WINDOW_BASE_H
#define QVIS_WINDOW_BASE_H
#include <gui_exports.h>
#include <qmainwindow.h>
#include <GUIBase.h>

class DataNode;

// *******************************************************************
// Class: QvisWindowBase
//
// Purpose:
//   This is the base class for all windows in the VisIt gui. It
//   provides a few features that all windows need. It inherits a lot
//   of useful messaging routines and a viewer pointer from GUIBase.
//
// Notes:      
//
// Programmer: Brad Whitlock
// Creation:   Wed Aug 30 18:19:35 PST 2000
//
// Modifications:
//   Brad Whitlock, Thu Apr 19 10:36:13 PDT 2001
//   Added overrides of showMinimized and showNormal methods.
//
//   Brad Whitlock, Fri Mar 21 10:30:05 PDT 2003
//   I added ProcessOldVersions.
//
// *******************************************************************

class GUI_API QvisWindowBase : public QMainWindow, public GUIBase
{
    Q_OBJECT
public:
    QvisWindowBase(const char *captionString = 0,
                   WFlags f = WType_TopLevel);
    virtual ~QvisWindowBase();

    virtual void showMinimized();
    virtual void showNormal();

    virtual void CreateNode(DataNode *);
    virtual void SetFromNode(DataNode *, const int *borders);
    virtual void ProcessOldVersions(DataNode *, const char *);
public slots:
    virtual void show();
protected:
    bool saveWindowDefaults;
};

#endif
