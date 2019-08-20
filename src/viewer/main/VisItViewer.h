// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#ifndef VISIT_VIEWER_H
#define VISIT_VIEWER_H
#include <viewer_exports.h>
#include <string>
#include <qstring.h>

// Objects that we need publicly
#include <avtDatabaseMetaData.h>
#include <ViewerMethods.h>
#include <ViewerState.h>
#include <ViewerProperties.h>

// Forward definitions
class vtkQtRenderWindow;
class ViewerSubject;

// ****************************************************************************
// Class: VisItViewer
//
// Purpose:
//   This class provides an interface for controlling VisIt's viewer as an
//   embedded component in another application. There is limited access to 
//   various viewer pieces to ensure they remain encapsulated.
//
// Notes:      
//
// Programmer: Brad Whitlock
// Creation:   Mon Aug 18 11:43:42 PDT 2008
//
// Modifications:
//    Jeremy Meredith, Thu Oct 16 19:29:12 EDT 2008
//    Added a flag for whether or not ProcessCommandLine should force 
//    the identical version.  It's necessary for new viewer-apps but
//    might confuse VisIt-proper's smart versioning.
//   
//    Brad Whitlock, Tue Apr 14 14:23:53 PDT 2009
//    Added ViewerProperties.
//
// ****************************************************************************

class VIEWER_API VisItViewer
{
public:
    // Initialize the viewer environment.
    static void Initialize(int *argc, char ***argv);

    // Construct the viewer.
    VisItViewer();

    // Destroy the viewer.
    ~VisItViewer();

    // Set the method for determining VISITHOME. VISITHOME is used to determine
    // where VisIt is installed so the application writer has the option of
    // telling the VisItViewer application where VisIt is located since the
    // application will not be located with VisIt itself except for when
    // the VisItViewer class is used in VisIt's own viewer application.
    typedef enum
    {
        FromEnvironment, // Get VISITHOME from environment (default)

        FromArgv0,       // Get VISITHOME from argv[0] path minus last path
                         // /path/to/app/bin/appname -> /path/to/app

        UserSpecified    // Set VISITHOME to user-specified path.
    } VISITHOME_METHOD;

    // Set the method used for determining VISITHOME [optional]
    // Must be called before ProcessCommandLine.
    void SetVISITHOMEMethod(VISITHOME_METHOD m);

    // Set the path to VISITHOME [optional]
    // Must be called before ProcessCommandLine.
    void SetVISITHOME(const std::string &);

    // Set viewer options from the command line [optional]
    void ProcessCommandLine(int argc, char **argv, bool addForceVersion=true);

    // Connect back to the client that launched the viewer [optional]
    void Connect(int *argc, char ***argv);

    // Finish setting up the viewer
    void Setup();

    // Removes the crash recovery file.
    void RemoveCrashRecoveryFile() const;

    // Returns whether the viewer was started with the -nowin flag.
    bool GetNowinMode() const;

    // Access the viewer's methods at once. The methods block until finished
    ViewerMethods *Methods() const;

    // Access the viewer's methods but call them in a delayed manner from the 
    // main event loop. Call methods through this object if you are doing an
    // operation that can be posted for execution but executed later. Use this 
    // For example, use this method if you are setting plot attributes or 
    // something that causes recalculation on the compute engine. Methods called
    // through this interface return immediately but are not executed immediately.
    ViewerMethods *DelayedMethods() const;

    // Access the viewer's state. No Notify is needed.
    ViewerState   *State() const;

    // Access the viewer's state. Changes to these state objects require a 
    // Notify() to post them into the event loop where they can be safely
    // used.
    ViewerState   *DelayedState() const;

    // Access viewer properties.
    ViewerProperties *Properties() const;

    // Get a file's metadata.
    const avtDatabaseMetaData *GetMetaData(const std::string &hostDB, int ts=-1);

    // Set a window creation callback function. Note that you only need to
    // supply this function if you want to embed vtkQtRenderWindow windows
    // into your own GUI interface.
    void SetWindowCreationCallback(vtkQtRenderWindow* (*wcc)(void *),
                                   void *wccdata);

    // Send an error message
    void Error(const QString &);

    // Send a warning
    void Warning(const QString &);

    // Methods to query plot plugin names.
    int         GetNumPlotPlugins() const;
    std::string GetPlotName(int index) const;
    int         GetPlotIndex(const std::string &plotName) const;

    // Methods to query operator plugin names.
    int         GetNumOperatorPlugins() const;
    std::string GetOperatorName(int index) const;
    int         GetOperatorIndex(const std::string &operatorName) const;

    // Get the VisIt command
    std::string GetVisItCommand() const;

    // Finalize the viewer environment.
    static void Finalize();

private:
    std::string GetVisItHome() const;

    VISITHOME_METHOD visitHomeMethod;
    std::string      visitHome;
    ViewerSubject   *viewer;
};

#endif
