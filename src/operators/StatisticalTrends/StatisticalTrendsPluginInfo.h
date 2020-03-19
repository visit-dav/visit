// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ****************************************************************************
//  File: StatisticalTrendsPluginInfo.h
// ****************************************************************************

#ifndef STATISTICALTRENDS_PLUGIN_INFO_H
#define STATISTICALTRENDS_PLUGIN_INFO_H
#include <OperatorPluginInfo.h>
#include <operator_plugin_exports.h>

class StatisticalTrendsAttributes;

// ****************************************************************************
//  Class: StatisticalTrendsPluginInfo
//
//  Purpose:
//    Five classes that provide all the information about an StatisticalTrends operator
//
//  Programmer: generated by xml2info
//  Creation:   omitted
//
//  Modifications:
//
// ****************************************************************************

class StatisticalTrendsGeneralPluginInfo : public virtual GeneralOperatorPluginInfo
{
  public:
    virtual const char *GetName() const;
    virtual const char *GetVersion() const;
    virtual const char *GetID() const;
    virtual bool  EnabledByDefault() const;
    virtual const char *GetCategoryName() const;
};

class StatisticalTrendsCommonPluginInfo : public virtual CommonOperatorPluginInfo, public virtual StatisticalTrendsGeneralPluginInfo
{
  public:
    virtual AttributeSubject *AllocAttributes();
    virtual void CopyAttributes(AttributeSubject *to, AttributeSubject *from);
    virtual ExpressionList *GetCreatedExpressions(const avtDatabaseMetaData *) const;
};

class StatisticalTrendsGUIPluginInfo : public virtual GUIOperatorPluginInfo, public virtual StatisticalTrendsCommonPluginInfo
{
  public:
    virtual QString *GetMenuName() const;
    virtual QvisPostableWindowObserver *CreatePluginWindow(int type,
        AttributeSubject *attr, const QString &caption, const QString &shortName,
        QvisNotepadArea *notepad);
    virtual const char **XPMIconData() const;
};

class StatisticalTrendsViewerEnginePluginInfo : public virtual ViewerEngineOperatorPluginInfo, public virtual StatisticalTrendsCommonPluginInfo
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
    virtual std::string GetOperatorVarDescription(AttributeSubject *atts,
                                                  const avtPlotMetaData &plot);
    virtual const char *GetMenuName() const;

    static void InitializeGlobalObjects();
  private:
    static StatisticalTrendsAttributes *defaultAtts;
    static StatisticalTrendsAttributes *clientAtts;
};

class StatisticalTrendsViewerPluginInfo : public virtual ViewerOperatorPluginInfo, public virtual StatisticalTrendsViewerEnginePluginInfo
{
  public:
    virtual const char **XPMIconData() const;
};

class StatisticalTrendsEnginePluginInfo : public virtual EngineOperatorPluginInfo, public virtual StatisticalTrendsViewerEnginePluginInfo
{
  public:
    virtual avtPluginFilter *AllocAvtPluginFilter();
};

class StatisticalTrendsScriptingPluginInfo : public virtual ScriptingOperatorPluginInfo, public virtual StatisticalTrendsCommonPluginInfo
{
  public:
    virtual void InitializePlugin(AttributeSubject *subj, void *data);
    virtual void *GetMethodTable(int *nMethods);
    virtual bool TypesMatch(void *pyobject);
    virtual char *GetLogString();
    virtual void SetDefaults(const AttributeSubject *atts);
};

#endif
