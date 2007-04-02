#ifndef VIEWER_BASE_H
#define VIEWER_BASE_H
#include <qobject.h>
#include <viewer_exports.h>

class ViewerMethods;
class ViewerState;

// ****************************************************************************
// Class: ViewerBase
//
// Purpose:
//   Base class for viewer classes
//
// Notes:      
//
// Programmer: Brad Whitlock
// Creation:   Mon Feb 12 09:25:39 PDT 2007
//
// Modifications:
//   
// ****************************************************************************

class VIEWER_API ViewerBase : public QObject
{
public:
    ViewerBase(QObject *parent = 0, const char *name = 0);
    virtual ~ViewerBase();

    // Methods to get pointers to the client state and the methods.
    static ViewerState   *GetViewerState();
    static ViewerMethods *GetViewerMethods();

    // Methods to send messages
    static void Error(const char *message);
    static void Warning(const char *message);
    static void Message(const char *message);
    static void ErrorClear();

    // Methods to send status
    static void Status(const char *message);
    static void Status(const char *message, int milliseconds);
    static void Status(const char *sender, const char *message);
    static void Status(const char *sender, int percent, int curStage,
                const char *curStageName, int maxStage);
    static void ClearStatus(const char *sender = 0);
private:
    static ViewerState   *base_viewerState;
    static ViewerMethods *base_viewerMethods;
};

#endif
