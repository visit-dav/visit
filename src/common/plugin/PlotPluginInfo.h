// ************************************************************************* //
//                              PlotPluginInfo.h                             //
// ************************************************************************* //

#ifndef PLOT_PLUGIN_INFO_H
#define PLOT_PLUGIN_INFO_H
#include <plugin_exports.h>
#include <stdio.h>

#define VAR_CATEGORY_MESH     0x01
#define VAR_CATEGORY_SCALAR   0x02
#define VAR_CATEGORY_MATERIAL 0x04
#define VAR_CATEGORY_VECTOR   0x08
#define VAR_CATEGORY_SUBSET   0x10
#define VAR_CATEGORY_SPECIES  0x20

// Forward declarations.
class AttributeSubject;
class QvisNotepadArea;
class QvisPostableWindowObserver;
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
// ****************************************************************************

class PLUGIN_API GeneralPlotPluginInfo
{
  public:
    virtual char *GetName() const = 0;
    virtual char *GetVersion() const = 0;
    virtual char *GetID() const = 0;
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
        const char *hostName, const char *databaseName,
        const char *variableName) = 0;

    virtual void ReInitializePlotAtts(AttributeSubject *atts,
        const char *hostName, const char *databaseName,
        const char *variableName) { ; };

    virtual void ResetPlotAtts(AttributeSubject *atts,
        const char *hostName, const char *databaseName,
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
    virtual void InitializePlugin(AttributeSubject *subj, FILE *logFile) = 0;
    virtual void *GetMethodTable(int *nMethods) = 0;
    virtual bool TypesMatch(void *) { return false; }
    virtual void SetLogging(bool val) = 0;
    virtual void SetDefaults(const AttributeSubject *) = 0;
};

#endif
