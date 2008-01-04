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
//
//   Brad Whitlock, Wed Dec 19 14:49:45 PST 2007
//   Added custom raise method.
//
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
typedef std::map<std::string, QvisWindowBase *> WindowBaseMap;

#endif
