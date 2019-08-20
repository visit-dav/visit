// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//  File: LineSamplerPluginInfo.h
// ************************************************************************* //

#ifndef LINESAMPLER_PLUGIN_INFO_H
#define LINESAMPLER_PLUGIN_INFO_H
#include <OperatorPluginInfo.h>
#include <operator_plugin_exports.h>

class LineSamplerAttributes;

// ****************************************************************************
//  Class: LineSamplerPluginInfo
//
//  Purpose:
//    Five classes that provide all the information about an LineSampler operator
//
//  Programmer: generated by xml2info
//  Creation:   omitted
//
//  Modifications:
//
// ****************************************************************************

class LineSamplerGeneralPluginInfo : public virtual GeneralOperatorPluginInfo
{
  public:
    virtual const char *GetName() const;
    virtual const char *GetVersion() const;
    virtual const char *GetID() const;
    virtual bool  EnabledByDefault() const;
    virtual const char *GetCategoryName() const;
};

class LineSamplerCommonPluginInfo : public virtual CommonOperatorPluginInfo, public virtual LineSamplerGeneralPluginInfo
{
  public:
    virtual AttributeSubject *AllocAttributes();
    virtual void CopyAttributes(AttributeSubject *to, AttributeSubject *from);
};

class LineSamplerGUIPluginInfo : public virtual GUIOperatorPluginInfo, public virtual LineSamplerCommonPluginInfo
{
  public:
    virtual QString *GetMenuName() const;
    virtual QvisPostableWindowObserver *CreatePluginWindow(int type,
        AttributeSubject *attr, const QString &caption, const QString &shortName,
        QvisNotepadArea *notepad);
    virtual const char **XPMIconData() const;
};

class LineSamplerViewerEnginePluginInfo : public virtual ViewerEngineOperatorPluginInfo, public virtual LineSamplerCommonPluginInfo
{
  public:
    virtual AttributeSubject *GetClientAtts();
    virtual AttributeSubject *GetDefaultAtts();
    virtual void SetClientAtts(AttributeSubject *atts);
    virtual void GetClientAtts(AttributeSubject *atts);
    virtual void GetClientAtts(AttributeSubject *atts, const bool, const bool);

    virtual void InitializeOperatorAtts(AttributeSubject *atts,
                                        const avtPlotMetaData &plot,
                                        const bool fromDefault);
    virtual void UpdateOperatorAtts(AttributeSubject *atts,
                                    const avtPlotMetaData &plot);
    virtual const char *GetMenuName() const;

    static void InitializeGlobalObjects();
  private:
    static LineSamplerAttributes *defaultAtts;
    static LineSamplerAttributes *clientAtts;
};

class LineSamplerViewerPluginInfo : public virtual ViewerOperatorPluginInfo, public virtual LineSamplerViewerEnginePluginInfo
{
  public:
    virtual const char **XPMIconData() const;
};

class LineSamplerEnginePluginInfo : public virtual EngineOperatorPluginInfo, public virtual LineSamplerViewerEnginePluginInfo
{
  public:
    virtual avtPluginFilter *AllocAvtPluginFilter();
};

class LineSamplerScriptingPluginInfo : public virtual ScriptingOperatorPluginInfo, public virtual LineSamplerCommonPluginInfo
{
  public:
    virtual void InitializePlugin(AttributeSubject *subj, void *data);
    virtual void *GetMethodTable(int *nMethods);
    virtual bool TypesMatch(void *pyobject);
    virtual char *GetLogString();
    virtual void SetDefaults(const AttributeSubject *atts);
};

#endif
