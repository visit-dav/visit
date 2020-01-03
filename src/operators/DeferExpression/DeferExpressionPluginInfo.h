// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ****************************************************************************
//  File: DeferExpressionPluginInfo.h
// ****************************************************************************

#ifndef DEFEREXPRESSION_PLUGIN_INFO_H
#define DEFEREXPRESSION_PLUGIN_INFO_H
#include <OperatorPluginInfo.h>
#include <operator_plugin_exports.h>

class DeferExpressionAttributes;

// ****************************************************************************
//  Class: DeferExpressionPluginInfo
//
//  Purpose:
//    Five classes that provide all the information about an DeferExpression operator
//
//  Programmer: generated by xml2info
//  Creation:   omitted
//
//  Modifications:
//
// ****************************************************************************

class DeferExpressionGeneralPluginInfo : public virtual GeneralOperatorPluginInfo
{
  public:
    virtual const char *GetName() const;
    virtual const char *GetVersion() const;
    virtual const char *GetID() const;
    virtual bool  EnabledByDefault() const;
    virtual const char *GetCategoryName() const;
};

class DeferExpressionCommonPluginInfo : public virtual CommonOperatorPluginInfo, public virtual DeferExpressionGeneralPluginInfo
{
  public:
    virtual AttributeSubject *AllocAttributes();
    virtual void CopyAttributes(AttributeSubject *to, AttributeSubject *from);
};

class DeferExpressionGUIPluginInfo : public virtual GUIOperatorPluginInfo, public virtual DeferExpressionCommonPluginInfo
{
  public:
    virtual QString *GetMenuName() const;
    virtual QvisPostableWindowObserver *CreatePluginWindow(int type,
        AttributeSubject *attr, const QString &caption, const QString &shortName,
        QvisNotepadArea *notepad);
};

class DeferExpressionViewerEnginePluginInfo : public virtual ViewerEngineOperatorPluginInfo, public virtual DeferExpressionCommonPluginInfo
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
    static DeferExpressionAttributes *defaultAtts;
    static DeferExpressionAttributes *clientAtts;
};

class DeferExpressionViewerPluginInfo : public virtual ViewerOperatorPluginInfo, public virtual DeferExpressionViewerEnginePluginInfo
{
  public:
};

class DeferExpressionEnginePluginInfo : public virtual EngineOperatorPluginInfo, public virtual DeferExpressionViewerEnginePluginInfo
{
  public:
    virtual avtPluginFilter *AllocAvtPluginFilter();
};

class DeferExpressionScriptingPluginInfo : public virtual ScriptingOperatorPluginInfo, public virtual DeferExpressionCommonPluginInfo
{
  public:
    virtual void InitializePlugin(AttributeSubject *subj, void *data);
    virtual void *GetMethodTable(int *nMethods);
    virtual bool TypesMatch(void *pyobject);
    virtual char *GetLogString();
    virtual void SetDefaults(const AttributeSubject *atts);
};

#endif
