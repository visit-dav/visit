/*****************************************************************************
*
* Copyright (c) 2000 - 2015, Lawrence Livermore National Security, LLC
* Produced at the Lawrence Livermore National Laboratory
* LLNL-CODE-442911
* All rights reserved.
*
* This file is  part of VisIt. For  details, see https://visit.llnl.gov/.  The
* full copyright notice is contained in the file COPYRIGHT located at the root
* of the VisIt distribution or at http://www.llnl.gov/visit/copyright.html.
*
* Redistribution  and  use  in  source  and  binary  forms,  with  or  without
* modification, are permitted provided that the following conditions are met:
*
*  - Redistributions of  source code must  retain the above  copyright notice,
*    this list of conditions and the disclaimer below.
*  - Redistributions in binary form must reproduce the above copyright notice,
*    this  list of  conditions  and  the  disclaimer (as noted below)  in  the
*    documentation and/or other materials provided with the distribution.
*  - Neither the name of  the LLNS/LLNL nor the names of  its contributors may
*    be used to endorse or promote products derived from this software without
*    specific prior written permission.
*
* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT  HOLDERS AND CONTRIBUTORS "AS IS"
* AND ANY EXPRESS OR  IMPLIED WARRANTIES, INCLUDING,  BUT NOT  LIMITED TO, THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND  FITNESS FOR A PARTICULAR  PURPOSE
* ARE  DISCLAIMED. IN  NO EVENT  SHALL LAWRENCE  LIVERMORE NATIONAL  SECURITY,
* LLC, THE  U.S.  DEPARTMENT OF  ENERGY  OR  CONTRIBUTORS BE  LIABLE  FOR  ANY
* DIRECT,  INDIRECT,   INCIDENTAL,   SPECIAL,   EXEMPLARY,  OR   CONSEQUENTIAL
* DAMAGES (INCLUDING, BUT NOT  LIMITED TO, PROCUREMENT OF  SUBSTITUTE GOODS OR
* SERVICES; LOSS OF  USE, DATA, OR PROFITS; OR  BUSINESS INTERRUPTION) HOWEVER
* CAUSED  AND  ON  ANY  THEORY  OF  LIABILITY,  WHETHER  IN  CONTRACT,  STRICT
* LIABILITY, OR TORT  (INCLUDING NEGLIGENCE OR OTHERWISE)  ARISING IN ANY  WAY
* OUT OF THE  USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH
* DAMAGE.
*
*****************************************************************************/
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
