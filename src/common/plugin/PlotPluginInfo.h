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

// ************************************************************************* //
//                              PlotPluginInfo.h                             //
// ************************************************************************* //

#ifndef PLOT_PLUGIN_INFO_H
#define PLOT_PLUGIN_INFO_H
#include <plugin_exports.h>
#include <plugin_vartypes.h>
#include <stdio.h>

#include <string>

// Forward declarations.
class AttributeSubject;
class QString;
class QvisNotepadArea;
class QvisPostableWindowObserver;
class QvisWizard;
class QWidget;
class avtDatabaseMetaData;
class ExpressionList;
class avtPlot;
class ViewerPlot;

// ****************************************************************************
//  Class: *PluginInfo
//
//  Purpose:
//    Four classes that provide all the information about a plot plugin.
//    The classes are all pure virtual.  The information is broken up into
//    four classes since portions of it are only relevant to particular
//    components within visit.  There is the general information which all
//    the components are interested in, the gui information which the gui
//    is interested in, the viewer information which the viewer is interested
//    in, and the engine information which the engine is interested in.
//
//  Programmer: Eric Brugger
//  Creation:   March 7, 2001
//
//  Modifications:
//    Brad Whitlock, Mon Mar 26 11:42:53 PDT 2001
//    Added a method to the GUIPluginInfo class that indicates what variable
//    types should appear in the variable list for the plot.
//
//    Brad Whitlock, Thur Jul 12 17:47:34 PST 2001
//    Added scripting plugins.
//
//    Jeremy Meredith, Thu Jul 26 09:33:12 PDT 2001
//    Renamed plugin info classes.
//
//    Jeremy Meredith, Fri Sep 28 13:31:21 PDT 2001
//    Added Common info.  Made the specific ones inherit from the generic ones.
//
//    Kathleen Bonnell, Fri Oct 19 15:33:35 PDT 2001 
//    Added subset var category, ResetPlotAtts method to ViewerPlotPluginInfo.
//
//    Jeremy Meredith, Mon Dec 17 15:18:05 PST 2001
//    Added species var category.
//
//    Brad Whitlock, Mon May 6 14:18:15 PST 2002
//    I added the SetDefaults method for scripting plugins.
//
//    Kathleen Bonnell, Thu Dec  5 16:53:22 PST 2002 
//    Added ReInitializePlotAtts to ViewerPlotPluginInfo. 
//
//    Brad Whitlock, Wed Mar 12 14:14:57 PST 2003
//    I added the XPMIconData method for GUI and Viewer plugins. I also added
//    a GetVariableTypes for viewer plugins.
//
//    Jeremy Meredith, Tue Sep 23 17:08:20 PDT 2003
//    Added tensor and symmetric tensor.
//
//    Jeremy Meredith, Wed Nov  5 10:28:29 PST 2003
//    Added ability to disable plugins by default.
//
//    Brad Whitlock, Fri Mar 26 00:40:29 PDT 2004
//    I changed the interface to several methods in the viewer plugin.
//
//    Brad Whitlock, Tue Dec 14 09:42:33 PDT 2004
//    I added support for plot wizards.
//
//    Hank Childs, Tue Mar 22 16:06:15 PST 2005
//    Made destructor virtual.
//
//    Hank Childs, Tue Jul 19 14:23:56 PDT 2005
//    Added VAR_CATEGORY_ARRAY.
//
//    Brad Whitlock, Thu Jan 5 14:40:42 PST 2006
//    I removed explicit support for logging from the scripting plugins.
//
//    Mark Blair, Mon Aug 21 18:29:00 PDT 2006
//    Added to information passed to plot wizard when created.
//
//    Brad Whitlock, Wed Feb 7 15:53:14 PST 2007
//    Added methods for alternate display in the viewer plugin info.
//
//    Brad Whitlock, Tue Mar 20 11:44:35 PDT 2007
//    Added ProvidesLegend method on the viewer plugin info.
//
//    Brad Whitlock, Fri Apr 25 10:09:24 PDT 2008
//    Made the GUI plugin info return QString so we can internationalize
//    plot names. Added const to strings returned from GeneralPlotPluginInfo.
//
// ****************************************************************************

class PLUGIN_API GeneralPlotPluginInfo
{
  public:
    virtual ~GeneralPlotPluginInfo() { ; };
    virtual const char *GetName() const = 0;
    virtual const char *GetVersion() const = 0;
    virtual const char *GetID() const = 0;
    virtual bool  EnabledByDefault() const { return true; }
};

class PLUGIN_API CommonPlotPluginInfo : public virtual GeneralPlotPluginInfo
{
  public:
    virtual AttributeSubject *AllocAttributes() = 0;
    virtual void CopyAttributes(AttributeSubject *to,
                                AttributeSubject *from) = 0;
};

class PLUGIN_API GUIPlotPluginInfo : public virtual CommonPlotPluginInfo
{
  public:
    virtual QString *GetMenuName() const = 0;
    virtual int GetVariableTypes() const = 0;
    virtual QvisPostableWindowObserver *CreatePluginWindow(int type,
        AttributeSubject *attr, const QString &caption, const QString &shortName,
        QvisNotepadArea *notepad) = 0;
    virtual QvisWizard *CreatePluginWizard(AttributeSubject *attr, QWidget *parent,
        const std::string &varName, const avtDatabaseMetaData *md,
        const ExpressionList *expList, const char *name =0)
    {
        return 0;
    }
    virtual const char **XPMIconData() const { return 0; }
};

class PLUGIN_API ViewerPlotPluginInfo : public virtual CommonPlotPluginInfo
{
  public:
    virtual AttributeSubject *GetClientAtts() = 0;
    virtual AttributeSubject *GetDefaultAtts() = 0;
    virtual void SetClientAtts(AttributeSubject *atts) = 0;
    virtual void GetClientAtts(AttributeSubject *atts) = 0;

    virtual avtPlot *AllocAvtPlot() = 0;

    virtual bool ProvidesLegend() const { return true; }

    virtual void InitializePlotAtts(AttributeSubject *atts, ViewerPlot *) = 0;
    virtual void ReInitializePlotAtts(AttributeSubject *atts, 
        ViewerPlot *) { ; }

    virtual void ResetPlotAtts(AttributeSubject *atts,
        ViewerPlot *) { ; }

    virtual QString *GetMenuName() const = 0;
    virtual const char **XPMIconData() const { return 0; }
    virtual int GetVariableTypes() const = 0;

    virtual void *AlternateDisplayCreate(ViewerPlot *plot) { return 0; }
    virtual void AlternateDisplayDestroy(void *dpy) {; }
    virtual void AlternateDisplayClear(void *dpy) {; }
    virtual void AlternateDisplayHide(void *dpy) {; }
    virtual void AlternateDisplayShow(void *dpy) {; }
    virtual void AlternateDisplayIconify(void *dpy) {; }
    virtual void AlternateDisplayDeIconify(void *dpy) {; }
};

class PLUGIN_API EnginePlotPluginInfo : public virtual CommonPlotPluginInfo
{
  public:
    virtual avtPlot *AllocAvtPlot() = 0;
};

class PLUGIN_API ScriptingPlotPluginInfo : public virtual CommonPlotPluginInfo
{
  public:
    virtual void InitializePlugin(AttributeSubject *subj, void *data) = 0;
    virtual void *GetMethodTable(int *nMethods) = 0;
    virtual char *GetLogString() = 0;
    virtual bool TypesMatch(void *) { return false; }
    virtual void SetDefaults(const AttributeSubject *) = 0;
};

#endif
