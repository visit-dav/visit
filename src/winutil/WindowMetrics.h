// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#ifndef WINDOW_METRICS_H
#define WINDOW_METRICS_H
#include <winutil_exports.h>
#include <QWidget>    // For the definition of the window system.

// ****************************************************************************
//  Class:  WindowMetrics
//
//  Purpose:
//    Metrics for window positioning and sizing.
//
//  Programmer:  Jeremy Meredith
//  Creation:    September 25, 2001
//
//  Modifications:
//    Jeremy Meredith, Tue Sep 25 17:08:21 PDT 2001
//    Made the static helper function public.
//
//    Brad Whitlock, Mon Mar 24 11:37:45 PDT 2003
//    Added API.
//
//    Brad Whitlock, Wed Jan 11 17:37:08 PST 2006
//    Added MeasureScreen.
//
//    Cyrus Harrison, Wed Mar 17 08:59:17 PDT 2010
//    Added testWindow member.
//
// ****************************************************************************

class WINUTIL_API WindowMetrics
{
  public:
    static WindowMetrics *Instance();
    void                  MeasureScreen(bool);

    int GetScreenX()   {return screenX;   }
    int GetScreenY()   {return screenY;   }
    int GetScreenW()   {return screenW;   }
    int GetScreenH()   {return screenH;   }
    int GetBorderT()   {return borderT;   }
    int GetBorderB()   {return borderB;   }
    int GetBorderL()   {return borderL;   }
    int GetBorderR()   {return borderR;   }
    int GetShiftX()    {return shiftX;    }
    int GetShiftY()    {return shiftY;    }
    int GetPreshiftX() {return preshiftX; }
    int GetPreshiftY() {return preshiftY; }

    // Here are some helper functions useful for clients
    static void WaitForWindowManagerToGrabWindow(QWidget *win);
    static void WaitForWindowManagerToMoveWindow(QWidget *win);
    static void CalculateScreen(QWidget *win, int &screenX, int &screenY,
                                              int &screenW, int &screenH);
    static void CalculateBorders(QWidget *win, int &borderT, int &borderB,
                                               int &borderL, int &borderR);
    static void CalculateTopLeft(QWidget *win, int &X, int &Y);

    static void SetEmbeddedWindowState(bool _embedded){ embedded = _embedded; }
    static bool EmbeddedWindowState() { return embedded; }
  private:
    int screenX;
    int screenY;
    int screenW;
    int screenH;

    int borderT;
    int borderB;
    int borderL;
    int borderR;

    int shiftX;
    int shiftY;

    int preshiftX;
    int preshiftY;

    static bool embedded;

    WindowMetrics();
    static WindowMetrics *instance;
#if defined(Q_OS_LINUX) 
    QWidget              *testWindow;
#endif
};

#endif
