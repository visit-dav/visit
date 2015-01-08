/*****************************************************************************
*
* Copyright (c) 2000 - 2015, Lawrence Livermore National Security, LLC
* Produced at the Lawrence Livermore National Laboratory
* LLNL-CODE-442911
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

#include <ViewerBase.h>

#include <ViewerFactory.h>
#include <ViewerProperties.h>
#include <ViewerState.h>
#include <ViewerMessaging.h>
#include <ViewerMethods.h>
#include <ViewerOperatorFactory.h>
#include <ViewerPlotFactory.h>
#include <ViewerStateManager.h>
#include <OperatorPluginManager.h>
#include <PlotPluginManager.h>

ViewerFactory         *ViewerBase::base_viewerFactory = 0;
ViewerState           *ViewerBase::base_viewerState = 0;
ViewerMethods         *ViewerBase::base_viewerMethods = 0;
PlotPluginManager     *ViewerBase::base_plotPlugins = 0;
ViewerPlotFactory     *ViewerBase::base_plotFactory = 0;
OperatorPluginManager *ViewerBase::base_operatorPlugins = 0;
ViewerOperatorFactory *ViewerBase::base_operatorFactory = 0;
ViewerProperties      *ViewerBase::base_viewerProperties = 0;
ViewerStateManager    *ViewerBase::base_viewerStateManager = 0;
ViewerMessaging       *ViewerBase::base_viewerMessaging = 0;
ViewerFileServerInterface    *ViewerBase::base_viewerFileServer = 0;
ViewerEngineManagerInterface *ViewerBase::base_viewerEngineManager = 0;

void (*ViewerBase::ProcessEventsCB)(void *) = NULL;
void  *ViewerBase::ProcessEventsCBData = NULL;

// ****************************************************************************
// Method: ViewerBase::ViewerBase
//
// Purpose: 
//   Constructor for the ViewerBase class.
//
// Programmer: Brad Whitlock
// Creation:   Thu Feb 22 14:36:43 PST 2007
//
// Modifications:
//
// ****************************************************************************

ViewerBase::ViewerBase()
{
}

// ****************************************************************************
// Method: ViewerBase::~ViewerBase
//
// Purpose: 
//   Destructor for the ViewerBase class.
//
// Programmer: Brad Whitlock
// Creation:   Thu Feb 22 14:37:16 PST 2007
//
// Modifications:
//   
// ****************************************************************************

ViewerBase::~ViewerBase()
{
}

// ****************************************************************************
// Method: ViewerBase::SetViewerFactory
//
// Purpose:
//   Set the viewer factory.
//
// Arguments:
//   f : The new factory.
//
// Programmer: Brad Whitlock
// Creation:   Wed Sep 10 17:19:14 PDT 2014
//
// Modifications:
//
// ****************************************************************************

void
ViewerBase::SetViewerFactory(ViewerFactory *f)
{
    if(base_viewerFactory != 0)
        delete base_viewerFactory;
    base_viewerFactory = f;
}

// ****************************************************************************
// Method: ViewerBase::GetViewerFactory
//
// Purpose:
//   Get the viewer factory.
//
// Returns:    The factory.
//
// Programmer: Brad Whitlock
// Creation:   Wed Sep 10 17:20:13 PDT 2014
//
// Modifications:
//
// ****************************************************************************

ViewerFactory *
ViewerBase::GetViewerFactory()
{
    if(base_viewerFactory == 0)
        base_viewerFactory = new ViewerFactory;
    
    return base_viewerFactory;
}

// ****************************************************************************
// Method: ViewerBase::GetViewerState
//
// Purpose: 
//   Returns pointer to the ViewerState object.
//
// Returns:    Pointer to the ViewerState object.
//
// Programmer: Brad Whitlock
// Creation:   Thu Feb 22 14:37:31 PST 2007
//
// Modifications:
//   
// ****************************************************************************

ViewerState *
ViewerBase::GetViewerState()
{
    if(base_viewerState == 0)
        base_viewerState = new ViewerState;

    return base_viewerState;
}

// ****************************************************************************
// Method: ViewerBase::GetViewerMethods
//
// Purpose: 
//   Returns a pointer to the ViewerMethods object.
//
// Returns:    Pointer to the ViewerMethods object.
//
// Programmer: Brad Whitlock
// Creation:   Thu Feb 22 14:38:34 PST 2007
//
// Modifications:
//   
// ****************************************************************************

ViewerMethods *
ViewerBase::GetViewerMethods()
{
    if(base_viewerMethods == 0)
        base_viewerMethods = new ViewerMethods(GetViewerState());

    return base_viewerMethods;
}

// ****************************************************************************
// Method: ViewerBase::GetViewerStateManager
//
// Purpose:
//   Get the pointer to the ViewerStateManager object.
//
// Returns:    Pointer to the ViewerStateManager object.
//
// Programmer: Brad Whitlock
// Creation:   Thu Aug 28 23:09:33 PDT 2014
//
// Modifications:
//
// ****************************************************************************

ViewerStateManager *
ViewerBase::GetViewerStateManager()
{
    if(base_viewerStateManager == 0)
        base_viewerStateManager = new ViewerStateManager;

    return base_viewerStateManager;
}

// ****************************************************************************
// Method: ViewerBase::GetPlotPluginManager
//
// Purpose: 
//   Return the plot plugin manager.
//
// Returns:    The plot plugin manager.
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Tue Jun 24 14:37:20 PDT 2008
//
// Modifications:
//   
// ****************************************************************************

PlotPluginManager *
ViewerBase::GetPlotPluginManager()
{
    if(base_plotPlugins == 0)
        base_plotPlugins = GetViewerFactory()->CreatePlotPluginManager();

    return base_plotPlugins;
}

// ****************************************************************************
// Method: ViewerBase::GetPlotFactory
//
// Purpose:
//   Return the plot factory.
//
// Returns:    The plot factory.
//
// Programmer: Brad Whitlock
// Creation:   Fri Aug 22 11:47:28 PDT 2014
//
// Modifications:
//
// ****************************************************************************

ViewerPlotFactory *
ViewerBase::GetPlotFactory()
{
    if(base_plotFactory == 0)
        base_plotFactory = new ViewerPlotFactory;

    return base_plotFactory;
}

// ****************************************************************************
// Method: ViewerBase::GetOperatorPluginManager
//
// Purpose: 
//   Return the operator plugin manager.
//
// Returns:    The operator plugin manager.
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Tue Jun 24 14:37:20 PDT 2008
//
// Modifications:
//   
// ****************************************************************************

OperatorPluginManager *
ViewerBase::GetOperatorPluginManager()
{
    if(base_operatorPlugins == 0)
        base_operatorPlugins = GetViewerFactory()->CreateOperatorPluginManager();

    return base_operatorPlugins;
}

// ****************************************************************************
// Method: ViewerBase::GetOperatorFactory
//
// Purpose:
//   Return the operator factory.
//
// Returns:    The operator factory.
//
// Programmer: Brad Whitlock
// Creation:   Fri Aug 22 11:47:28 PDT 2014
//
// Modifications:
//
// ****************************************************************************

ViewerOperatorFactory *
ViewerBase::GetOperatorFactory()
{
    if(base_operatorFactory == 0)
        base_operatorFactory = new ViewerOperatorFactory;

    return base_operatorFactory;
}

// ****************************************************************************
// Method: ViewerBase::GetViewerProperties
//
// Purpose: 
//   Return the viewer properties
//
// Returns:    The viewer properties.
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Tue Apr 14 11:19:10 PDT 2009
//
// Modifications:
//   
// ****************************************************************************

ViewerProperties *
ViewerBase::GetViewerProperties()
{
    if(base_viewerProperties == 0)
        base_viewerProperties = new ViewerProperties;

    return base_viewerProperties;
}

// ****************************************************************************
// Method: ViewerBase::GetViewerMessaging
//
// Purpose:
//   Returns the viewer messaging object.
//
// Returns:    The viewer messaging.
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Wed Sep  3 12:42:12 PDT 2014
//
// Modifications:
//
// ****************************************************************************

ViewerMessaging *
ViewerBase::GetViewerMessaging()
{
    if(base_viewerMessaging == NULL)
        base_viewerMessaging = GetViewerFactory()->CreateViewerMessaging();

    return base_viewerMessaging;
}

// ****************************************************************************
// Method: ViewerBase::GetViewerFileServer
//
// Purpose:
//   Returns the viewer file server object.
//
// Returns:    The viewer file server.
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Mon Sep  8 17:47:58 PDT 2014
//
// Modifications:
//
// ****************************************************************************

ViewerFileServerInterface *
ViewerBase::GetViewerFileServer()
{
    if(base_viewerFileServer == NULL)
        base_viewerFileServer = GetViewerFactory()->CreateFileServerInterface();

    return base_viewerFileServer;
}

// ****************************************************************************
// Method: ViewerBase::GetViewerEngineManager
//
// Purpose:
//   Returns the viewer engine manager object.
//
// Returns:    The viewer engine manager.
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Mon Sep  8 17:47:58 PDT 2014
//
// Modifications:
//
// ****************************************************************************

ViewerEngineManagerInterface *
ViewerBase::GetViewerEngineManager()
{
    if(base_viewerEngineManager == NULL)
        base_viewerEngineManager = GetViewerFactory()->CreateEngineManagerInterface();

    return base_viewerEngineManager;
}

// ****************************************************************************
// Method: ViewerBase::SetProcessEventsCallback
//
// Purpose:
//   Set a callback to help process events.
//
// Arguments:
//   cb : The callback function.
//   cbdata : The callback function data.
//
// Returns:    
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Fri Aug 29 16:50:03 PDT 2014
//
// Modifications:
//
// ****************************************************************************

void
ViewerBase::SetProcessEventsCallback(void (*cb)(void*), void *cbdata)
{
    ProcessEventsCB = cb;
    ProcessEventsCBData = cbdata;
}
