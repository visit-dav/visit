/*****************************************************************************
*
* Copyright (c) 2000 - 2008, Lawrence Livermore National Security, LLC
* Produced at the Lawrence Livermore National Laboratory
* LLNL-CODE-400142
* All rights reserved.
*
* This file is  part of VisIt. For  details, see https://visit.llnl.gov/.  The
* full copyright notice is contained in the file COPYRIGHT located at the root
* of the VisIt distribution or at http://www.llnl.gov/visit/copyright.html.
*
* Redistribution  and  use  in  source  and  binary  forms,  with  or  without
* modification, are permitted provided that the following conditions are met:
*
*  - Redistributions of  source code must  retain the above  copyright notice,
*    this list of conditions and the disclaimer below.
*  - Redistributions in binary form must reproduce the above copyright notice,
*    this  list of  conditions  and  the  disclaimer (as noted below)  in  the
*    documentation and/or other materials provided with the distribution.
*  - Neither the name of  the LLNS/LLNL nor the names of  its contributors may
*    be used to endorse or promote products derived from this software without
*    specific prior written permission.
*
* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT  HOLDERS AND CONTRIBUTORS "AS IS"
* AND ANY EXPRESS OR  IMPLIED WARRANTIES, INCLUDING,  BUT NOT  LIMITED TO, THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND  FITNESS FOR A PARTICULAR  PURPOSE
* ARE  DISCLAIMED. IN  NO EVENT  SHALL LAWRENCE  LIVERMORE NATIONAL  SECURITY,
* LLC, THE  U.S.  DEPARTMENT OF  ENERGY  OR  CONTRIBUTORS BE  LIABLE  FOR  ANY
* DIRECT,  INDIRECT,   INCIDENTAL,   SPECIAL,   EXEMPLARY,  OR   CONSEQUENTIAL
* DAMAGES (INCLUDING, BUT NOT  LIMITED TO, PROCUREMENT OF  SUBSTITUTE GOODS OR
* SERVICES; LOSS OF  USE, DATA, OR PROFITS; OR  BUSINESS INTERRUPTION) HOWEVER
* CAUSED  AND  ON  ANY  THEORY  OF  LIABILITY,  WHETHER  IN  CONTRACT,  STRICT
* LIABILITY, OR TORT  (INCLUDING NEGLIGENCE OR OTHERWISE)  ARISING IN ANY  WAY
* OUT OF THE  USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH
* DAMAGE.
*
*****************************************************************************/

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
// ****************************************************************************

class WINUTIL_API WindowMetrics
{
  public:
    static WindowMetrics *Instance();
    void MeasureScreen(bool);

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

    WindowMetrics();
    static WindowMetrics *instance;
};

#endif
