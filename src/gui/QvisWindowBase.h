// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#ifndef QVIS_WINDOW_BASE_H
#define QVIS_WINDOW_BASE_H
#include <gui_exports.h>
#include <QMainWindow>
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
//
//   Brad Whitlock, Wed Dec 19 14:49:45 PST 2007
//   Added custom raise method.
//
//   Brad Whitlock, Wed Apr  9 10:32:12 PDT 2008
//   Changed ctor args.
//
//   Brad Whitlock, Tue May 27 14:51:18 PDT 2008
//   Qt 4.
//
// ****************************************************************************

class GUI_API QvisWindowBase : public QMainWindow, public GUIBase
{
    Q_OBJECT
public:
    QvisWindowBase(const QString &captionString = QString(),
                   Qt::WindowFlags f = Qt::Window);
    virtual ~QvisWindowBase();

    virtual void showMinimized();
    virtual void showNormal();

    virtual void CreateNode(DataNode *);
    virtual void SetFromNode(DataNode *, const int *borders);
    virtual void ProcessOldVersions(DataNode *, const char *);

    static void StringToDoubleList(const char *str, doubleVector &dv,
                    int max=INT_MAX);

    static void FitToScreen(int &x, int &y, int &w, int &h);

    static void SetWindowAnchorLocation(int, int);
    static bool GetWindowAnchorLocation(int &, int &);

public slots:
    virtual void show();
    virtual void raise();
protected:
    bool saveWindowDefaults;
private:
    static bool windowAnchorSet;
    static int  windowAnchor[2];
};

// Create a type for a vector of postable windows.
typedef std::vector<QvisWindowBase *> WindowBaseVector;
typedef std::map<QString, QvisWindowBase *> WindowBaseMap;

#endif
