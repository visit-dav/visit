// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ****************************************************************************
//  File: RemapPluginInfo.h
// ****************************************************************************

#ifndef REMAP_PLUGIN_INFO_H
#define REMAP_PLUGIN_INFO_H
#include <OperatorPluginInfo.h>
#include <operator_plugin_exports.h>

class RemapAttributes;

// ****************************************************************************
//  Class: RemapPluginInfo
//
//  Purpose:
//    Five classes that provide all the information about an Remap operator
//
//  Programmer: generated by xml2info
//  Creation:   omitted
//
//  Modifications:
//
// ****************************************************************************

class RemapGeneralPluginInfo : public virtual GeneralOperatorPluginInfo
{
  public:
    virtual const char *GetName() const;
    virtual const char *GetVersion() const;
    virtual const char *GetID() const;
    virtual bool  EnabledByDefault() const;
    virtual const char *GetCategoryName() const;
};

class RemapCommonPluginInfo : public virtual CommonOperatorPluginInfo, public virtual RemapGeneralPluginInfo
{
  public:
    virtual AttributeSubject *AllocAttributes();
    virtual void CopyAttributes(AttributeSubject *to, AttributeSubject *from);
};

class RemapGUIPluginInfo : public virtual GUIOperatorPluginInfo, public virtual RemapCommonPluginInfo
{
  public:
    virtual QString *GetMenuName() const;
    virtual QvisPostableWindowObserver *CreatePluginWindow(int type,
        AttributeSubject *attr, const QString &caption, const QString &shortName,
        QvisNotepadArea *notepad);
};

class RemapViewerEnginePluginInfo : public virtual ViewerEngineOperatorPluginInfo, public virtual RemapCommonPluginInfo
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
    static RemapAttributes *defaultAtts;
    static RemapAttributes *clientAtts;
};

class RemapViewerPluginInfo : public virtual ViewerOperatorPluginInfo, public virtual RemapViewerEnginePluginInfo
{
  public:
};

class RemapEnginePluginInfo : public virtual EngineOperatorPluginInfo, public virtual RemapViewerEnginePluginInfo
{
  public:
    virtual avtPluginFilter *AllocAvtPluginFilter();
};

class RemapScriptingPluginInfo : public virtual ScriptingOperatorPluginInfo, public virtual RemapCommonPluginInfo
{
  public:
    virtual void InitializePlugin(AttributeSubject *subj, void *data);
    virtual void *GetMethodTable(int *nMethods);
    virtual bool TypesMatch(void *pyobject);
    virtual char *GetLogString();
    virtual void SetDefaults(const AttributeSubject *atts);
};

#endif
