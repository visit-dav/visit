// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#ifndef GUI_BASE_H
#define GUI_BASE_H
#include <gui_exports.h>
#include <string>
#include <vectortypes.h>

// Forward declarations
class QLineEdit;
class QString;
class FileServerList;
class MessageAttributes;
class ViewerMethods;
class ViewerProxy;
class ViewerState;
struct StatusSubject;
struct QualifiedFilename;
class SimpleObserver;
class QvisHelpWindow;

// ****************************************************************************
// Class: GUIBase
//
// Purpose:
//   This class contains useful methods that are needed in all windows
//   and all widgets, for instance, error messages, status bar stuff.
//   They all need a pointer to the viewer proxy too. This class is
//   therefore a base class for all of those classes.
//
// Notes:
//
// Programmer: Brad Whitlock
// Creation:   Mon Sep 25 16:40:18 PST 2000
//
// Modifications:
//   Brad Whitlock, Thu Aug 23 8:31:23 PDT 2001
//   Added makeDefaultConfirm attribute.
//
//   Brad Whitlock, Wed Mar 13 14:21:34 PST 2002
//   Added methods to set the cursor.
//
//   Brad Whitlock, Thu May 9 16:26:29 PST 2002
//   Made fileServer a static member and added OpenDataFile function.
//
//   Brad Whitlock, Mon Jul 29 14:39:30 PST 2002
//   I added a reOpen argument to OpenDataFile.
//
//   Brad Whitlock, Thu May 15 12:33:11 PDT 2003
//   I added a timeState argument to OpenDataFile.
//
//   Brad Whitlock, Wed Oct 22 12:16:01 PDT 2003
//   I added an addDefaultPlots argument to OpenDataFile.
//
//   Brad Whitlock, Mon Nov 3 10:44:14 PDT 2003
//   Renamed OpenDataFile to SetOpenDataFile and removed its addDefaultPlots
//   argument.
//
//   Brad Whitlock, Fri Jan 30 11:40:54 PDT 2004
//   I added methods to get the state for the active source and to open
//   the active source in the file server.
//
//.  Brad Whitlock, Mon May 3 17:27:11 PST 2004
//   I added an argument to GetTimeSliderStateForDatabaseState.
//
//   Brad Whitlock, Tue Feb 13 09:21:03 PDT 2007
//   I added GetViewerMethods and GetViewerState.
//
//   Brad Whitlock, Fri Jan 18 14:38:53 PST 2008
//   Added Information method.
//
//   Brad Whitlock, Wed Jun 18 15:03:18 PDT 2008
//   Added ResettingError, FloatsToString, DoublesToString, QStringToFloats,
//   QStringToDoubles, LineEditGetDoubles, LineEditGetFloats.
//
//   Cyrus Harrison, Tue Aug 26 07:46:41 PDT 2008
//   Made helper functions public static so other classes can utilize them.
//
//   Kathleen Biagas, Thu Apr  9 11:48:50 PDT 2015
//   Add precision argument to FloatsToQString and DoublesToQString.
//
//   Kathleen Biagas, Thu Nov 17, 2022
//   Added QStringToBools, BoolsToQString, LineEditGetBools.
//
// ****************************************************************************

class GUI_API GUIBase
{
public:
    GUIBase();
    virtual ~GUIBase();

    // Functions to set information global to all of the window classes.
    void ConnectMessageAttr(MessageAttributes *attr);

    // Helpers for converting between values and qstrings
    static QString FloatsToQString(const float *, int, int precision=7);
    static QString FloatsToQString(const floatVector &, int precision=7);
    static QString FloatToQString(float, int precision=7);
    static QString DoublesToQString(const double *, int, int precision=16);
    static QString DoublesToQString(const doubleVector &, int precision=16);
    static QString DoubleToQString(double, int precision=16);
    static QString IntsToQString(const int *, int);
    static QString IntsToQString(const intVector &);
    static QString IntToQString(int);
    static QString BoolsToQString(const bool *, int);
    static QString BoolsToQString(const boolVector &);

    static bool    QStringToFloats(const QString &str, float *vals, int maxVals);
    static bool    QStringToFloats(const QString &str, floatVector &vals, int maxVals=-1);
    static bool    QStringToDoubles(const QString &str, double *vals, int maxVals);
    static bool    QStringToDoubles(const QString &str, doubleVector &vals, int maxVal=-1);
    static bool    QStringToInts(const QString &str, int *vals, int maxVals);
    static bool    QStringToInts(const QString &str, intVector &vals, int maxVal=-1);
    static bool    QStringToBools(const QString &str, bool *vals, int maxVals);
    static bool    QStringToBools(const QString &str, boolVector &vals, int maxVal=-1);

    static bool    LineEditGetDoubles(QLineEdit *lineEdit, double *vals, int maxVals);
    static bool    LineEditGetDoubles(QLineEdit *lineEdit, doubleVector &vals, int maxVals=-1);
    static bool    LineEditGetFloats(QLineEdit *lineEdit, float *vals, int maxVals);
    static bool    LineEditGetFloats(QLineEdit *lineEdit, floatVector &vals, int maxVals=-1);
    static bool    LineEditGetInts(QLineEdit *lineEdit, int *vals, int maxVals);
    static bool    LineEditGetInts(QLineEdit *lineEdit, intVector &vals, int maxVals=-1);
    static bool    LineEditGetBools(QLineEdit *lineEdit, bool *vals, int maxVals);
    static bool    LineEditGetBools(QLineEdit *lineEdit, boolVector &vals, int maxVals=-1);

    static bool    LineEditGetDouble(QLineEdit *lineEdit, double &val);
    static bool    LineEditGetFloat(QLineEdit *lineEdit, float &val);
    static bool    LineEditGetInt(QLineEdit *lineEdit, int &val);

protected:
    void           SetViewerProxy(ViewerProxy *);
    ViewerProxy   *GetViewerProxy() const;
    ViewerState   *GetViewerState() const;
    ViewerMethods *GetViewerMethods() const;

    void Error(const QString &msg);
    void Warning(const QString &msg);
    void Message(const QString &msg);
    // Shows right away, cleared by other message types
    void Information(const QString &msg);
    // Specialized error that we use all the time
    void ResettingError(const QString &name, const QString &val);

    void Status(const QString &msg, int milliseconds = 10000);
    void ClearStatus();

    bool AutoUpdate() const;

    void SetWaitCursor();
    void RestoreCursor();

    bool SetOpenDataFile(const QualifiedFilename &qf,
                         int timeState,
                         SimpleObserver *sob = 0,
                         bool reOpen = false);

    void OpenActiveSourceInFileServer();
    int  GetStateForSource(const QualifiedFilename &source) const;
    int  GetTimeSliderStateForDatabaseState(const std::string &tsName,
                                            const QualifiedFilename &source,
                                            int dbState) const;

    QString GetPlotDescription(const QString &plotName) const;

    static void RegisterHelpWindow(QvisHelpWindow* help);
    static QvisHelpWindow* GetHelpWindow();
private:
    static ViewerProxy       *viewer;
protected:
    static StatusSubject     *statusSubject;
    static bool              writeToConsole;
    static bool              autoUpdate;
    static bool              makeDefaultConfirm;
    static MessageAttributes *msgAttr;
    static FileServerList    *fileServer;
    static QvisHelpWindow* helpWindow;
};

#endif
