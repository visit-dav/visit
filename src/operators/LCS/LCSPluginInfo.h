// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//  File: LCSPluginInfo.h
// ************************************************************************* //

#ifndef LCS_PLUGIN_INFO_H
#define LCS_PLUGIN_INFO_H
#include <OperatorPluginInfo.h>
#include <operator_plugin_exports.h>

class LCSAttributes;

// ****************************************************************************
//  Class: LCSPluginInfo
//
//  Purpose:
//    Five classes that provide all the information about an LCS operator
//
//  Programmer: generated by xml2info
//  Creation:   omitted
//
//  Modifications:
//
// ****************************************************************************

class LCSGeneralPluginInfo : public virtual GeneralOperatorPluginInfo
{
  public:
    virtual const char *GetName() const;
    virtual const char *GetVersion() const;
    virtual const char *GetID() const;
    virtual bool  EnabledByDefault() const;
    virtual const char *GetCategoryName() const;
};

class LCSCommonPluginInfo : public virtual CommonOperatorPluginInfo, public virtual LCSGeneralPluginInfo
{
  public:
    virtual AttributeSubject *AllocAttributes();
    virtual void CopyAttributes(AttributeSubject *to, AttributeSubject *from);
    virtual ExpressionList *GetCreatedExpressions(const avtDatabaseMetaData *) const;
};

class LCSGUIPluginInfo : public virtual GUIOperatorPluginInfo, public virtual LCSCommonPluginInfo
{
  public:
    virtual QString *GetMenuName() const;
    virtual QvisPostableWindowObserver *CreatePluginWindow(int type,
        AttributeSubject *attr, const QString &caption, const QString &shortName,
        QvisNotepadArea *notepad);
    virtual const char **XPMIconData() const;
};

class LCSViewerEnginePluginInfo : public virtual ViewerEngineOperatorPluginInfo, public virtual LCSCommonPluginInfo
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
    static LCSAttributes *defaultAtts;
    static LCSAttributes *clientAtts;
};

class LCSViewerPluginInfo : public virtual ViewerOperatorPluginInfo, public virtual LCSViewerEnginePluginInfo
{
  public:
    virtual const char **XPMIconData() const;
};

class LCSEnginePluginInfo : public virtual EngineOperatorPluginInfo, public virtual LCSViewerEnginePluginInfo
{
  public:
    virtual avtPluginFilter *AllocAvtPluginFilter();
};

class LCSScriptingPluginInfo : public virtual ScriptingOperatorPluginInfo, public virtual LCSCommonPluginInfo
{
  public:
    virtual void InitializePlugin(AttributeSubject *subj, void *data);
    virtual void *GetMethodTable(int *nMethods);
    virtual bool TypesMatch(void *pyobject);
    virtual char *GetLogString();
    virtual void SetDefaults(const AttributeSubject *atts);
};

#endif
