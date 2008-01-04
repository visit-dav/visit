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

#include <QvisWindowBase.h>
#include <DataNode.h>
#include <vectortypes.h>
#include <qapplication.h>

// Global vars.
static QWidget *parentOfEveryWindow = 0;

// Class static
bool QvisWindowBase::windowAnchorSet = false;
int  QvisWindowBase::windowAnchor[2] = {0,0};

// *****************************************************************************
// Method: QvisWindowBase::QvisWindowBase
//
// Purpose: 
//   Constructor for the QvisWindowBase class.
//
// Arguments:
//   captionString : This is the caption displayed in the window decor.
//   f             : A Qt flag indicating the type of window.
//
// Note:       
//   This is the base class for all windows in the VisIt gui.
//
// Programmer: Brad Whitlock
// Creation:   Wed Aug 30 18:14:03 PST 2000
//
// Modifications:
//   Brad Whitlock, Wed Sep 25 10:03:42 PDT 2002
//   I changed the code so that on the Windows platform, all windows are
//   children of the first window which happens to be the Main window.
//
//   Brad Whitlock, Thu Sep 4 12:18:59 PDT 2003
//   I made all windows be children of the main window on MacOS X so when
//   child windows are active, the VisIt menu does not change.
//
// ****************************************************************************

QvisWindowBase::QvisWindowBase(const char *captionString, WFlags f) :
    QMainWindow(parentOfEveryWindow, captionString,
#if defined(Q_WS_WIN) || defined(Q_WS_MACX)
                WType_TopLevel | f)
{
    // Make each window the child of the first window that is created
    // when we run on the Windows platform. This prevents the sub-windows
    // from showing up in the taskbar.
    if(parentOfEveryWindow == 0)
        parentOfEveryWindow = this;
#else
                f)
{
#endif
    if(captionString != 0)
        setCaption(captionString);
    else
        setCaption("VisIt");

    saveWindowDefaults = false;
}

// ****************************************************************************
// Method: QvisWindowBase::~QvisWindowBase
//
// Purpose: 
//   Destructor for the QvisWindowBase class.
//
// Programmer: Brad Whitlock
// Creation:   Wed Aug 30 18:15:17 PST 2000
//
// Modifications:
//   
// ****************************************************************************

QvisWindowBase::~QvisWindowBase()
{
    // nothing here.
}

// ****************************************************************************
// Method: QvisWindowBase::showMinimized
//
// Purpose: 
//   Is the window is visible, this method minimizes the window.
//
// Programmer: Brad Whitlock
// Creation:   Thu Apr 19 10:36:40 PDT 2001
//
// Modifications:
//   
// ****************************************************************************

void
QvisWindowBase::showMinimized()
{
    if(isVisible())
        QMainWindow::showMinimized();
}

// ****************************************************************************
// Method: QvisWindowBase::showNormal
//
// Purpose: 
//   If the window is minimized, this method shows it normal (de-iconifies it)
//
// Programmer: Brad Whitlock
// Creation:   Thu Apr 19 10:37:14 PDT 2001
//
// Modifications:
//   
// ****************************************************************************

void
QvisWindowBase::showNormal()
{
    if(isMinimized())
        QMainWindow::showNormal();
}

// ****************************************************************************
// Method: QvisWindowBase::CreateNode
//
// Purpose: 
//   Adds the window's geometry and visibility status to the DataNode
//   tree that will be saved to the VisIt config file.
//
// Arguments:
//   parentNode : The parent node under which we'll add a node for
//                this window.
//
// Programmer: Brad Whitlock
// Creation:   Tue Oct 3 15:30:23 PST 2000
//
// Modifications:
//   
// ****************************************************************************

void
QvisWindowBase::CreateNode(DataNode *parentNode)
{
    if(saveWindowDefaults)
    {
        DataNode *node = new DataNode(std::string(caption().latin1()));
        parentNode->AddNode(node);

        // Add generic window attributes
        node->AddNode(new DataNode("x", x()));
        node->AddNode(new DataNode("y", y()));
        node->AddNode(new DataNode("width", width()));
        node->AddNode(new DataNode("height", height()));
        node->AddNode(new DataNode("visible", isVisible()));
    }
}

// ****************************************************************************
// Method: QvisWindowBase::SetFromNode
//
// Purpose: 
//   Sets the window's geometry information from the DataNode.
//
// Arguments:
//   parentNode : This a pointer to the "GUI" node.
//   borders : An array of 4 ints containing the sizes of the window
//             decorations.
//
// Programmer: Brad Whitlock
// Creation:   Tue Oct 3 15:41:34 PST 2000
//
// Modifications:
//   Brad Whitlock, Wed Sep 10 09:19:32 PDT 2003
//   Added a method to make sure that the window fits on the screen.
//
//   Hank Childs, Mon Nov 14 16:25:27 PST 2005
//   Don't allow windows to come up off the screen.
//
//   Brad Whitlock, Wed Nov 22 09:56:26 PDT 2006
//   Added code to override the window location if an anchor has been provided.
//
// ****************************************************************************

void
QvisWindowBase::SetFromNode(DataNode *parentNode, const int *borders)
{
    DataNode *winNode = parentNode->GetNode(std::string(caption().latin1()));
    if(winNode == 0)
        return;

    // Indicate that the window should be saved.
    saveWindowDefaults = true;

    DataNode *node;
    bool xy_set = false, wh_set = false;
    int x = 0, y = 0;
    int w = width();
    int h = height();

    // See if any attributes are set.
    if((node = winNode->GetNode("x")) != 0)
    {
        int x_pos = node->AsInt();
        if (x_pos < 0)
            x_pos = 0;
        x = x_pos + borders[2];
        xy_set = true;
    }
    if((node = winNode->GetNode("y")) != 0)
    {
        int y_pos = node->AsInt();
        if (y_pos < 0)
            y_pos = 0;
        y = y_pos + borders[0];
        xy_set = true;
    }
    if((node = winNode->GetNode("width")) != 0)
    {
        w = node->AsInt();
        wh_set = true;
    }
    if((node = winNode->GetNode("height")) != 0)
    {
        h = node->AsInt();
        wh_set = true;
    }

    // Possibly override the window anchor location.
    xy_set |= GetWindowAnchorLocation(x, y);

    // Make sure that the window will fit on the screen.
    FitToScreen(x, y, w, h);

    // Set the window geometry.
    if(wh_set && xy_set)
        setGeometry(x, y, w, h);
    else if(xy_set)
        move(x, y);
    else if(wh_set)
        resize(w, h);

    // If the window is visible, show it.
    if((node = winNode->GetNode("visible")) != 0)
    {
        if(node->AsBool())
            show();
    }
}

// ****************************************************************************
// Method: QvisWindowBase::FitToScreen
//
// Purpose: 
//   Makes sure that the window will fit on the screen.
//
// Arguments:
//   x : The x location of the window.
//   y : The y location of the window.
//   w : The window's width.
//   h : The window's height.
//
// Programmer: Brad Whitlock
// Creation:   Wed Sep 10 09:27:07 PDT 2003
//
// Modifications:
//   
// ****************************************************************************

void
QvisWindowBase::FitToScreen(int &x, int &y, int &w, int &h)
{
    const int screenW = qApp->desktop()->width();
    const int screenH = qApp->desktop()->height();

    if(x + w > screenW)
    {
        x -= ((x + w) - screenW);
        if(x < 0)
        {
            w += x;
            x = 0;
        }
    }

    if(y + h > screenH)
    {
        y -= ((y + h) - screenH);
        if(y < 0)
        {
            h += y;
            y = 0;
        }
    }
}

// ****************************************************************************
// Method: QvisWindowBase::ProcessOldVersions
//
// Purpose: 
//   Processes old versions. Override this if you need to change the DataNode
//   for the window to migrate old settings to newer settings.
//
// Programmer: Brad Whitlock
// Creation:   Fri Mar 21 10:31:12 PDT 2003
//
// Modifications:
//   
// ****************************************************************************

void
QvisWindowBase::ProcessOldVersions(DataNode *, const char *)
{
}

// ****************************************************************************
// Method: QvisWindowBase::show
//
// Purpose: 
//   This is a Qt slot function that shows the window.
//
// Note:       It has been extended to make note that the window
//             information should be saved since the window has been
//             shown.
//
// Programmer: Brad Whitlock
// Creation:   Wed Oct 4 10:51:12 PDT 2000
//
// Modifications:
//   Brad Whitlock, Thu Apr 19 09:30:01 PDT 2001
//   Added code to raise the window when it is shown.
//
//   Brad Whitlock, Wed Nov 22 10:00:18 PDT 2006
//   Added code to move the window if an anchor has been set.
//
// ****************************************************************************

void
QvisWindowBase::show()
{
    // Indicate that the window should be saved.
    saveWindowDefaults = true;

    // Move the window in case it has not been moved already in SetFromNode.
    int wx, wy;
    if(GetWindowAnchorLocation(wx, wy) &&
       (wx != x() || wy != y()))
    {
        QMainWindow::move(wx, wy);
    }

    if(isMinimized())
        QMainWindow::showNormal();
    else
        QMainWindow::show();
    QMainWindow::raise();
}

// ****************************************************************************
// Method: QvisWindowBase::raise
//
// Purpose: 
//   Show iconified windows normally before raising them.
//
// Programmer: Brad Whitlock
// Creation:   Wed Dec 19 14:48:45 PST 2007
//
// Modifications:
//   
// ****************************************************************************

void
QvisWindowBase::raise()
{
    if(isMinimized())
        QMainWindow::showNormal();
    QMainWindow::raise();
}

// ****************************************************************************
// Method: QvisBaseWindow::StringToDoubleList
//
// Purpose:
//   Dissects a string into a list of doubles and stores it in the passed-in
//   doubleVector.
//
// Arguments:
//   str : The string to be searched for doubles.
//   dv  : The return vector to contain the doubles.
//
// Programmer: Brad Whitlock
// Creation:   Sat Feb 17 13:05:15 PST 2001
//
// Modifications:
//   Brad Whitlock, Thu Feb 14 15:18:35 PST 2002
//   Added code to prevent adding any more than MAX_CONTOURS elements to
//   the dv vector.
//
//   Mark C. Miller, Sun Nov  6 19:35:43 PST 2005
//   Relocated to here from QvisIsosurfaceWindow due to common functionality
//
// ****************************************************************************

void
QvisWindowBase::StringToDoubleList(const char *str, doubleVector &dv, int max)
{
    int length, offset = 0;

    // Clear the vector.
    dv.clear();

    // Get out if the input string is nothing.
    if(str == NULL || ((length = strlen(str)) == 0))
    {
        return;
    }

    do {
        // Skip any preceding spaces, stop at end of string too.
        for(; str[offset] == ' ' || str[offset] == '\0'; ++offset);

        if(offset < length)
        {
            char buf[30];
            sscanf(str + offset, "%s", buf);
            offset += strlen(buf);

            // Only add if the token was something.
            if(strlen(buf) > 0)
            {
                double dtemp = (double)atof(buf);
                if(dv.size() < max)
                    dv.push_back(dtemp);
                else
                    offset = length * 2;
            }
        }
    } while(offset < length);
}

// ****************************************************************************
// Method: QvisWindowBase::SetWindowAnchorLocation
//
// Purpose: 
//   Sets the window anchor location where windows will appear.
//
// Arguments:
//   wx : The x location where windows will appear.
//   wy : The y location where windows will appear.
//
// Programmer: Brad Whitlock
// Creation:   Wed Nov 22 09:53:10 PDT 2006
//
// Modifications:
//   
// ****************************************************************************

void
QvisWindowBase::SetWindowAnchorLocation(int wx, int wy)
{
    windowAnchorSet = true;
    windowAnchor[0] = wx;
    windowAnchor[1] = wy;
}

// ****************************************************************************
// Method: QvisWindowBase::GetWindowAnchorLocation
//
// Purpose: 
//   Gets the window anchor location if one has been set.
//
// Arguments:
//   wx : The return value for the x window location.
//   wy : The return value for the y window location.
//
// Returns:    True if the window anchor was set; False otherwise.
//
// Programmer: Brad Whitlock
// Creation:   Wed Nov 22 09:54:12 PDT 2006
//
// Modifications:
//   
// ****************************************************************************

bool
QvisWindowBase::GetWindowAnchorLocation(int &wx, int &wy)
{
    bool ret = windowAnchorSet;
    if(ret)
    {
        wx = windowAnchor[0];
        wy = windowAnchor[1];
    }
    return ret;
}

