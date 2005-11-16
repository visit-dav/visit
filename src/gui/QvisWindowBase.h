#ifndef QVIS_WINDOW_BASE_H
#define QVIS_WINDOW_BASE_H
#include <gui_exports.h>
#include <qmainwindow.h>
#include <GUIBase.h>
#include <vectortypes.h>
#include <vector>
#include <map>
#include <limits.h>

class DataNode;

// ****************************************************************************
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
//   Brad Whitlock, Wed Sep 10 09:28:22 PDT 2003
//   I added FitToScreen.
//
//   Mark C. Miller, Wed Nov 16 10:46:36 PST 2005
//   Added StringToDoubleList utility which was duplicated in other gui classes 
// ****************************************************************************

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

    static void StringToDoubleList(const char *str, doubleVector &dv,
                    int max=INT_MAX);

public slots:
    virtual void show();
protected:
    void FitToScreen(int &x, int &y, int &w, int &h) const;

    bool saveWindowDefaults;
};

// Create a type for a vector of postable windows.
typedef std::vector<QvisWindowBase *> WindowBaseVector;
typedef std::map<std::string, QvisWindowBase *> WindowBaseMap;

#endif
