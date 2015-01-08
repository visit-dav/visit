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
#ifndef VIEWER_BASE_H
#define VIEWER_BASE_H
#include <viewercore_exports.h>

class OperatorPluginManager;
class PlotPluginManager;
class ViewerEngineManagerInterface;
class ViewerFactory;
class ViewerFileServerInterface;
class ViewerMessaging;
class ViewerMethods;
class ViewerOperatorFactory;
class ViewerPlotFactory;
class ViewerProperties;
class ViewerState;
class ViewerStateManager;

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
//   Cyrus Harrison, Thu Feb 21 15:04:14 PST 2008
//   Added control to suppress messages. 
//
//   Brad Whitlock, Tue Apr 29 11:11:04 PDT 2008
//   Converted most messaging to QString.
//
//   Brad Whitlock, Fri May  9 14:34:56 PDT 2008
//   Qt 4.
//
//   Brad Whitlock, Tue Jun 24 14:33:17 PDT 2008
//   Added methods to return the plugin managers.
//
//   Brad Whitlock, Tue Apr 14 11:12:07 PDT 2009
//   I added GetViewerProperties. I made the message suppression stuff use the
//   new ViewerProperties.
//
//   Brad Whitlock, Fri Aug 22 11:51:12 PDT 2014
//   I added plot and operator factories and state manager.
//
// ****************************************************************************

class VIEWERCORE_API ViewerBase
{
public:
    ViewerBase();
    virtual ~ViewerBase();

    static void SetViewerFactory(ViewerFactory *);

    static ViewerFactory                *GetViewerFactory();
    static ViewerState                  *GetViewerState();
    static ViewerMethods                *GetViewerMethods();
    static ViewerMessaging              *GetViewerMessaging();
    static PlotPluginManager            *GetPlotPluginManager();
    static ViewerPlotFactory            *GetPlotFactory();
    static OperatorPluginManager        *GetOperatorPluginManager();
    static ViewerOperatorFactory        *GetOperatorFactory();
    static ViewerProperties             *GetViewerProperties();
    static ViewerStateManager           *GetViewerStateManager();
    static ViewerFileServerInterface    *GetViewerFileServer();
    static ViewerEngineManagerInterface *GetViewerEngineManager();

protected:
    static void SetProcessEventsCallback(void (*cb)(void*), void *cbdata);
    static void (*ProcessEventsCB)(void *);
    static void  *ProcessEventsCBData;

private:
    static ViewerFactory                *base_viewerFactory;
    static ViewerMethods                *base_viewerMethods;
    static ViewerState                  *base_viewerState;
    static PlotPluginManager            *base_plotPlugins;
    static ViewerPlotFactory            *base_plotFactory;
    static OperatorPluginManager        *base_operatorPlugins;
    static ViewerOperatorFactory        *base_operatorFactory;
    static ViewerProperties             *base_viewerProperties;
    static ViewerStateManager           *base_viewerStateManager;
    static ViewerMessaging              *base_viewerMessaging;
    static ViewerFileServerInterface    *base_viewerFileServer;
    static ViewerEngineManagerInterface *base_viewerEngineManager;
};

#endif
