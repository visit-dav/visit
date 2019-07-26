// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//  File: IsosurfacePluginInfo.h
// ************************************************************************* //

#ifndef ISOSURFACE_PLUGIN_INFO_H
#define ISOSURFACE_PLUGIN_INFO_H
#include <OperatorPluginInfo.h>
#include <operator_plugin_exports.h>

class IsosurfaceAttributes;

// ****************************************************************************
//  Class: IsosurfacePluginInfo
//
//  Purpose:
//    Five classes that provide all the information about an Isosurface operator
//
//  Programmer: generated by xml2info
//  Creation:   omitted
//
//  Modifications:
//
// ****************************************************************************

class IsosurfaceGeneralPluginInfo : public virtual GeneralOperatorPluginInfo
{
  public:
    virtual const char *GetName() const;
    virtual const char *GetVersion() const;
    virtual const char *GetID() const;
    virtual bool  EnabledByDefault() const;
    virtual const char *GetCategoryName() const;
};

class IsosurfaceCommonPluginInfo : public virtual CommonOperatorPluginInfo, public virtual IsosurfaceGeneralPluginInfo
{
  public:
    virtual AttributeSubject *AllocAttributes();
    virtual void CopyAttributes(AttributeSubject *to, AttributeSubject *from);
};

class IsosurfaceGUIPluginInfo : public virtual GUIOperatorPluginInfo, public virtual IsosurfaceCommonPluginInfo
{
  public:
    virtual QString *GetMenuName() const;
    virtual QvisPostableWindowObserver *CreatePluginWindow(int type,
        AttributeSubject *attr, const QString &caption, const QString &shortName,
        QvisNotepadArea *notepad);
    virtual const char **XPMIconData() const;
};

class IsosurfaceViewerEnginePluginInfo : public virtual ViewerEngineOperatorPluginInfo, public virtual IsosurfaceCommonPluginInfo
{
  public:
    virtual AttributeSubject *GetClientAtts();
    virtual AttributeSubject *GetDefaultAtts();
    virtual void SetClientAtts(AttributeSubject *atts);
    virtual void GetClientAtts(AttributeSubject *atts);

    virtual void InitializeOperatorAtts(AttributeSubject *atts,
                                        const avtPlotMetaData &plot,
                                        const bool fromDefault);
    virtual void UpdateOperatorAtts(AttributeSubject *atts,
                                    const avtPlotMetaData &plot);
    virtual const char *GetMenuName() const;

    static void InitializeGlobalObjects();
  private:
    static IsosurfaceAttributes *defaultAtts;
    static IsosurfaceAttributes *clientAtts;
};

class IsosurfaceViewerPluginInfo : public virtual ViewerOperatorPluginInfo, public virtual IsosurfaceViewerEnginePluginInfo
{
  public:
    virtual const char **XPMIconData() const;
};

class IsosurfaceEnginePluginInfo : public virtual EngineOperatorPluginInfo, public virtual IsosurfaceViewerEnginePluginInfo
{
  public:
    virtual avtPluginFilter *AllocAvtPluginFilter();
};

class IsosurfaceScriptingPluginInfo : public virtual ScriptingOperatorPluginInfo, public virtual IsosurfaceCommonPluginInfo
{
  public:
    virtual void InitializePlugin(AttributeSubject *subj, void *data);
    virtual void *GetMethodTable(int *nMethods);
    virtual bool TypesMatch(void *pyobject);
    virtual char *GetLogString();
    virtual void SetDefaults(const AttributeSubject *atts);
};

#endif
