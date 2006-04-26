// ************************************************************************* //
//                              PlotPluginInfo.h                             //
// ************************************************************************* //

#ifndef PLOT_PLUGIN_INFO_H
#define PLOT_PLUGIN_INFO_H
#include <plugin_exports.h>
#include <plugin_vartypes.h>
#include <stdio.h>

// Forward declarations.
class AttributeSubject;
class QvisNotepadArea;
class QvisPostableWindowObserver;
class QvisWizard;
class QWidget;
class avtDatabaseMetaData;
class avtPlot;

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
// ****************************************************************************

class PLUGIN_API GeneralPlotPluginInfo
{
  public:
    virtual ~GeneralPlotPluginInfo() { ; };
    virtual char *GetName() const = 0;
    virtual char *GetVersion() const = 0;
    virtual char *GetID() const = 0;
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
    virtual const char *GetMenuName() const = 0;
    virtual int GetVariableTypes() const = 0;
    virtual QvisPostableWindowObserver *CreatePluginWindow(int type,
        AttributeSubject *attr, QvisNotepadArea *notepad) = 0;
    virtual QvisWizard *CreatePluginWizard(AttributeSubject *attr,
        QWidget *parent, const char *name =0)
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

    virtual void InitializePlotAtts(AttributeSubject *atts,
        const avtDatabaseMetaData *,
        const char *variableName) = 0;

    virtual void ReInitializePlotAtts(AttributeSubject *atts,
        const avtDatabaseMetaData *,
        const char *variableName) { ; };

    virtual void ResetPlotAtts(AttributeSubject *atts,
        const avtDatabaseMetaData *md,
        const char *variableName) { ; } ;

    virtual const char **XPMIconData() const { return 0; }
    virtual int GetVariableTypes() const = 0;
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
