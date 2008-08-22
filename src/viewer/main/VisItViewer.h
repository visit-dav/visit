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
#ifndef VISIT_VIEWER_H
#define VISIT_VIEWER_H
#include <viewer_exports.h>
#include <string>
#include <qstring.h>

// Objects that we need publicly
#include <avtDatabaseMetaData.h>
#include <ViewerMethods.h>
#include <ViewerState.h>

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
    void ProcessCommandLine(int argc, char **argv);

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
