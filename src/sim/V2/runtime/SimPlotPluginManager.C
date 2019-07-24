// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#include <SimPlotPluginManager.h>

#include <PlotPluginInfo.h>

// ****************************************************************************
// Class: ViewerWrappedEnginePlotPluginInfo
//
// Purpose:
//   Lets an EnginePlotPluginInfo pretend to be a ViewerPlotPluginInfo.
//
// Notes:    
//
// Programmer: Brad Whitlock
// Creation:   Wed Sep 17 16:16:11 PDT 2014
//
// Modifications:
//
// ****************************************************************************

class ViewerWrappedEnginePlotPluginInfo : public ViewerPlotPluginInfo
{
public:
    ViewerWrappedEnginePlotPluginInfo(EnginePlotPluginInfo *e) : eInfo(e)
    {
    }

    virtual ~ViewerWrappedEnginePlotPluginInfo()
    {
    }

    // GeneralPlotPluginInfo methods
    virtual const char *GetName() const
    {
        return eInfo->GetName();
    }

    virtual const char *GetVersion() const
    {
        return eInfo->GetVersion();
    }

    virtual const char *GetID() const
    {
        return eInfo->GetID();
    }

    virtual bool  EnabledByDefault() const
    {
        return eInfo->EnabledByDefault();
    }

    // CommonPlotPluginInfo methods
    virtual AttributeSubject *AllocAttributes()
    {
        return eInfo->AllocAttributes();
    }

    virtual void CopyAttributes(AttributeSubject *to,
                                AttributeSubject *from)
    {
        eInfo->CopyAttributes(to, from);
    }

    virtual int GetVariableTypes() const 
    {
        return eInfo->GetVariableTypes();
    }

    // ViewerEnginePlotPluginInfo methods.
    virtual AttributeSubject *GetClientAtts()
    {
        return eInfo->GetClientAtts();
    }

    virtual AttributeSubject *GetDefaultAtts()
    {
        return eInfo->GetDefaultAtts();
    }

    virtual void SetClientAtts(AttributeSubject *atts)
    {
        eInfo->SetClientAtts(atts);
    }

    virtual void GetClientAtts(AttributeSubject *atts)
    {
        eInfo->GetClientAtts(atts);
    }

    virtual avtPlot *AllocAvtPlot()
    {
        return eInfo->AllocAvtPlot();
    }

    virtual bool ProvidesLegend() const
    {
        return eInfo->ProvidesLegend();
    }

    virtual bool PermitsCurveViewScaling() const
    {
        return eInfo->PermitsCurveViewScaling();
    }

    virtual bool Permits2DViewScaling() const
    {
        return eInfo->Permits2DViewScaling();
    }

    virtual void InitializePlotAtts(AttributeSubject *atts,
                                    const avtPlotMetaData &plot)
    {
        eInfo->InitializePlotAtts(atts, plot);
    }

    virtual void ReInitializePlotAtts(AttributeSubject *atts, 
                                      const avtPlotMetaData &plot)
    {
        eInfo->ReInitializePlotAtts(atts, plot);
    }

    virtual void ResetPlotAtts(AttributeSubject *atts,
                               const avtPlotMetaData &plot)
    {
        eInfo->ResetPlotAtts(atts, plot);
    }

    virtual const char *GetMenuName() const
    {
        return eInfo->GetMenuName();
    }

    // We don't need to override ViewerPlotPluginInfo default methods
private:
    EnginePlotPluginInfo *eInfo;
};

// ****************************************************************************
// Method: SimPlotPluginManager::SimPlotPluginManager
//
// Purpose:
//   Constructor.
//
// Arguments:
//   mgr : The engine plugin manager.
//
// Returns:    
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Wed Sep 17 16:32:07 PDT 2014
//
// Modifications:
//
// ****************************************************************************

SimPlotPluginManager::SimPlotPluginManager() : PlotPluginManager(), wrapped()
{
}

// ****************************************************************************
// Method: SimPlotPluginManager::~SimPlotPluginManager
//
// Purpose:
//   Destructor.
//
// Programmer: Brad Whitlock
// Creation:   Wed Sep 17 16:32:07 PDT 2014
//
// Modifications:
//
// ****************************************************************************

SimPlotPluginManager::~SimPlotPluginManager()
{
    std::map<std::string, ViewerWrappedEnginePlotPluginInfo *>::iterator it;
    for(it = wrapped.begin(); it != wrapped.end(); ++it)
        delete it->second;
}

// ****************************************************************************
// Method: SimPlotPluginManager::GetViewerPluginInfo
//
// Purpose:
//   Retrieves a viewer plugin info object (internally using the engine plugin 
//   manager and engine plugins).
//
// Arguments:
//   id : The id of the plugin for which we want a viewer plugin.
//
// Returns:    A viewer plugin implemented by an engine plugin.
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Wed Sep 17 16:33:01 PDT 2014
//
// Modifications:
//
// ****************************************************************************

ViewerPlotPluginInfo *
SimPlotPluginManager::GetViewerPluginInfo(const std::string &id)
{
    std::map<std::string, ViewerWrappedEnginePlotPluginInfo *>::iterator it;
    it = wrapped.find(id);
    if(it == wrapped.end())
    {
        // Get the engine plugin from the other plot manager.
        EnginePlotPluginInfo *eInfo = GetEnginePluginInfo(id);

        // Wrap the info we looked up.
        wrapped[id] = new ViewerWrappedEnginePlotPluginInfo(eInfo);
        it = wrapped.find(id);
    }

    return it->second;   
}
