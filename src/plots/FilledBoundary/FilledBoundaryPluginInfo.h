// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                     FilledBoundaryPluginInfo.h                            //
// ************************************************************************* //

#ifndef FILLEDBOUNDARY_PLUGIN_INFO_H
#define FILLEDBOUNDARY_PLUGIN_INFO_H
#include <PlotPluginInfo.h>
#include <plot_plugin_exports.h>

class FilledBoundaryAttributes;

// ****************************************************************************
//  Class: FilledBoundaryPluginInfo
//
//  Purpose:
//    Five classes that provide all the information about a FilledBoundary
//    plot plugin.  The information is broken up into five classes since
//    portions of it are only relevant to particular components within
//    visit.  There is the general information which all the components
//    are interested in, the gui information which the gui is interested in,
//    the viewer information which the viewer is interested in, the
//    engine information which the engine is interested in, and finally a.
//    scripting portion that enables the Python VisIt extension to use the
//    plugin.
//
//  Programmer: generated by xml2info
//  Creation:   omitted
//
// ****************************************************************************

class FilledBoundaryGeneralPluginInfo: public virtual GeneralPlotPluginInfo
{
  public:
    virtual const char *GetName() const;
    virtual const char *GetVersion() const;
    virtual const char *GetID() const;
    virtual bool  EnabledByDefault() const;
};

class FilledBoundaryCommonPluginInfo : public virtual CommonPlotPluginInfo, public virtual FilledBoundaryGeneralPluginInfo
{
  public:
    virtual AttributeSubject *AllocAttributes();
    virtual void CopyAttributes(AttributeSubject *to, AttributeSubject *from);
    virtual int GetVariableTypes() const;
};

class FilledBoundaryGUIPluginInfo : public virtual GUIPlotPluginInfo, public virtual FilledBoundaryCommonPluginInfo
{
  public:
    virtual QString *GetMenuName() const;
    virtual QvisPostableWindowObserver *CreatePluginWindow(int type,
        AttributeSubject *attr, const QString &caption, const QString &shortName,
        QvisNotepadArea *notepad);
    virtual const char **XPMIconData() const;
};

class FilledBoundaryViewerEnginePluginInfo : public virtual ViewerEnginePlotPluginInfo, public virtual FilledBoundaryCommonPluginInfo
{
  public:
    virtual AttributeSubject *GetClientAtts();
    virtual AttributeSubject *GetDefaultAtts();
    virtual void SetClientAtts(AttributeSubject *atts);
    virtual void GetClientAtts(AttributeSubject *atts);

    virtual avtPlot *AllocAvtPlot();

    virtual void InitializePlotAtts(AttributeSubject *atts, const avtPlotMetaData &);
    virtual void ReInitializePlotAtts(AttributeSubject *atts, const avtPlotMetaData &);
    virtual void ResetPlotAtts(AttributeSubject *atts, const avtPlotMetaData &);
    virtual const char *GetMenuName() const;
    static void InitializeGlobalObjects();
  private:
    static FilledBoundaryAttributes *defaultAtts;
    static FilledBoundaryAttributes *clientAtts;
    // User-defined functions
  private:
    void   PrivateSetPlotAtts(AttributeSubject *atts, const avtPlotMetaData &);
};

class FilledBoundaryViewerPluginInfo : public virtual ViewerPlotPluginInfo, public virtual FilledBoundaryViewerEnginePluginInfo
{
  public:
    virtual const char **XPMIconData() const;

};

class FilledBoundaryEnginePluginInfo : public virtual EnginePlotPluginInfo, public virtual FilledBoundaryViewerEnginePluginInfo
{
  public:
};

class FilledBoundaryScriptingPluginInfo : public virtual ScriptingPlotPluginInfo, public virtual FilledBoundaryCommonPluginInfo
{
  public:
    virtual void InitializePlugin(AttributeSubject *subj, void *data);
    virtual void *GetMethodTable(int *nMethods);
    virtual bool TypesMatch(void *pyobject);
    virtual char *GetLogString();
    virtual void SetDefaults(const AttributeSubject *atts);
};

#endif
