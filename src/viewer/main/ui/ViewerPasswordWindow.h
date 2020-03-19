// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#ifndef VIEWERPASSWORDWINDOW_H
#define VIEWERPASSWORDWINDOW_H
#include <VisItPasswordWindow.h>
#include <set>

// Forward declarations
class ViewerConnectionProgressDialog;

// ****************************************************************************
//  Class:  ViewerPasswordWindow
//
//  Purpose:
//    Main window for the program.
//
//  Programmer:  Jeremy Meredith
//  Creation:    April 25, 2001
//
//  Modifications:
//    Brad Whitlock, Thu Aug 29 10:40:19 PDT 2002
//    I removed the Windows API stuff since this file is not part of the
//    viewer on Windows. I also added a new static function.
//
//    Brad Whitlock, Thu Aug 29 17:50:25 PST 2002
//    I added a userName argument to the getPassword and authenticate methods.
//
//    Jeremy Meredith, Thu May 24 10:57:02 EDT 2007
//    Added support for checking failed SSH tunneling (port forwards).
//
//    Thomas R. Treadway, Mon Oct  8 13:27:42 PDT 2007
//    Backing out SSH tunneling on Panther (MacOS X 10.3)
//
//    Hank Childs, Sat Nov 10 11:31:34 PST 2007
//    Add a new button for changing the username.
//
//    Kathleen Bonnell, Wed Feb 13 14:05:03 PST 2008 
//    Added static methods to retrieve and reset the value of 
//    'needToChangeUsername'. 
//
//    Brad Whitlock, Tue May 27 13:44:12 PDT 2008
//    Qt 4.
//
//    Kathleen Bonnell, Thu Apr 22 18:06:28 MST 2010 
//    Changed return type of getPassword to std::string.
//
//    Brad Whitlock, Tue Jun 12 11:17:23 PDT 2012
//    I made it a subclass of VisItPasswordWindow.
//
// ****************************************************************************

class ViewerPasswordWindow : public VisItPasswordWindow
{
    Q_OBJECT
public:
    ViewerPasswordWindow(QWidget *parent=NULL);
    virtual ~ViewerPasswordWindow();

    // Callback function for RemoteProcess' authentication callback.
    static void authenticate(const char *username, const char* password, const char *host, int fd);

    static void SetConnectionProgressDialog(ViewerConnectionProgressDialog *d);

    static std::set<int> GetFailedPortForwards();
private:
    std::string password(const char *username, const char *host,
                         const char *phrase,
                         VisItPasswordWindow::ReturnCode &ret);

    static ViewerPasswordWindow           *instance;
    static ViewerConnectionProgressDialog *dialog;
    static std::set<int>                   failedPortForwards;
};

#endif
