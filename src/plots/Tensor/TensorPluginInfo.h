// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//  TensorPluginInfo.h
// ************************************************************************* //

#ifndef TENSOR_PLUGIN_INFO_H
#define TENSOR_PLUGIN_INFO_H
#include <PlotPluginInfo.h>
#include <plot_plugin_exports.h>

class TensorAttributes;

// ****************************************************************************
//  Class: TensorPluginInfo
//
//  Purpose:
//    Five classes that provide all the information about a Tensor
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

class TensorGeneralPluginInfo: public virtual GeneralPlotPluginInfo
{
  public:
    virtual const char *GetName() const;
    virtual const char *GetVersion() const;
    virtual const char *GetID() const;
    virtual bool  EnabledByDefault() const;
};

class TensorCommonPluginInfo : public virtual CommonPlotPluginInfo, public virtual TensorGeneralPluginInfo
{
  public:
    virtual AttributeSubject *AllocAttributes();
    virtual void CopyAttributes(AttributeSubject *to, AttributeSubject *from);
    virtual int GetVariableTypes() const;
};

class TensorGUIPluginInfo : public virtual GUIPlotPluginInfo, public virtual TensorCommonPluginInfo
{
  public:
    virtual QString *GetMenuName() const;
    virtual QvisPostableWindowObserver *CreatePluginWindow(int type,
        AttributeSubject *attr, const QString &caption, const QString &shortName,
        QvisNotepadArea *notepad);
    virtual const char **XPMIconData() const;
};

class TensorViewerEnginePluginInfo : public virtual ViewerEnginePlotPluginInfo, public virtual TensorCommonPluginInfo
{
  public:
    virtual AttributeSubject *GetClientAtts();
    virtual AttributeSubject *GetDefaultAtts();
    virtual void SetClientAtts(AttributeSubject *atts);
    virtual void GetClientAtts(AttributeSubject *atts);

    virtual avtPlot *AllocAvtPlot();

    virtual void InitializePlotAtts(AttributeSubject *atts, const avtPlotMetaData &);
    virtual bool SupportsAnimation() const;
    virtual bool AnimationReset(AttributeSubject *atts, const avtPlotMetaData &plot);
    virtual bool AnimationStep(AttributeSubject *atts, const avtPlotMetaData  &plot);
    virtual const char *GetMenuName() const;
    static void InitializeGlobalObjects();
  private:
    static TensorAttributes *defaultAtts;
    static TensorAttributes *clientAtts;
};

class TensorViewerPluginInfo : public virtual ViewerPlotPluginInfo, public virtual TensorViewerEnginePluginInfo
{
  public:
    virtual const char **XPMIconData() const;

};

class TensorEnginePluginInfo : public virtual EnginePlotPluginInfo, public virtual TensorViewerEnginePluginInfo
{
  public:
};

class TensorScriptingPluginInfo : public virtual ScriptingPlotPluginInfo, public virtual TensorCommonPluginInfo
{
  public:
    virtual void InitializePlugin(AttributeSubject *subj, void *data);
    virtual void *GetMethodTable(int *nMethods);
    virtual bool TypesMatch(void *pyobject);
    virtual char *GetLogString();
    virtual void SetDefaults(const AttributeSubject *atts);
};

#endif
