#include <QvisWindowBase.h>
#include <DataNode.h>
#include <qapplication.h>

static QWidget *parentOfEveryWindow = 0;

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
// ****************************************************************************

QvisWindowBase::QvisWindowBase(const char *captionString, WFlags f) :
    QMainWindow(parentOfEveryWindow, captionString,
#if defined(_WIN32)
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
        x = node->AsInt() + borders[2];
        xy_set = true;
    }
    if((node = winNode->GetNode("y")) != 0)
    {
        y = node->AsInt() + borders[0];
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
QvisWindowBase::FitToScreen(int &x, int &y, int &w, int &h) const
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
// ****************************************************************************

void
QvisWindowBase::show()
{
    // Indicate that the window should be saved.
    saveWindowDefaults = true;

    QMainWindow::show();
    QMainWindow::raise();
}
