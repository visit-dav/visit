// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//  File: ExternalSurfacePluginInfo.h
// ************************************************************************* //

#ifndef EXTERNALSURFACE_PLUGIN_INFO_H
#define EXTERNALSURFACE_PLUGIN_INFO_H
#include <OperatorPluginInfo.h>
#include <operator_plugin_exports.h>

class ExternalSurfaceAttributes;

// ****************************************************************************
//  Class: ExternalSurfacePluginInfo
//
//  Purpose:
//    Five classes that provide all the information about an ExternalSurface operator
//
//  Programmer: generated by xml2info
//  Creation:   omitted
//
//  Modifications:
//
// ****************************************************************************

class ExternalSurfaceGeneralPluginInfo : public virtual GeneralOperatorPluginInfo
{
  public:
    virtual const char *GetName() const;
    virtual const char *GetVersion() const;
    virtual const char *GetID() const;
    virtual bool  EnabledByDefault() const;
    virtual const char *GetCategoryName() const;
};

class ExternalSurfaceCommonPluginInfo : public virtual CommonOperatorPluginInfo, public virtual ExternalSurfaceGeneralPluginInfo
{
  public:
    virtual AttributeSubject *AllocAttributes();
    virtual void CopyAttributes(AttributeSubject *to, AttributeSubject *from);
};

class ExternalSurfaceGUIPluginInfo : public virtual GUIOperatorPluginInfo, public virtual ExternalSurfaceCommonPluginInfo
{
  public:
    virtual QString *GetMenuName() const;
    virtual QvisPostableWindowObserver *CreatePluginWindow(int type,
        AttributeSubject *attr, const QString &caption, const QString &shortName,
        QvisNotepadArea *notepad);
};

class ExternalSurfaceViewerEnginePluginInfo : public virtual ViewerEngineOperatorPluginInfo, public virtual ExternalSurfaceCommonPluginInfo
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
    static ExternalSurfaceAttributes *defaultAtts;
    static ExternalSurfaceAttributes *clientAtts;
};

class ExternalSurfaceViewerPluginInfo : public virtual ViewerOperatorPluginInfo, public virtual ExternalSurfaceViewerEnginePluginInfo
{
  public:
};

class ExternalSurfaceEnginePluginInfo : public virtual EngineOperatorPluginInfo, public virtual ExternalSurfaceViewerEnginePluginInfo
{
  public:
    virtual avtPluginFilter *AllocAvtPluginFilter();
};

class ExternalSurfaceScriptingPluginInfo : public virtual ScriptingOperatorPluginInfo, public virtual ExternalSurfaceCommonPluginInfo
{
  public:
    virtual void InitializePlugin(AttributeSubject *subj, void *data);
    virtual void *GetMethodTable(int *nMethods);
    virtual bool TypesMatch(void *pyobject);
    virtual char *GetLogString();
    virtual void SetDefaults(const AttributeSubject *atts);
};

#endif
