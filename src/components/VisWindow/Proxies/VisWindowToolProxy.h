// ************************************************************************* //
//                         VisWindowToolProxy.h                              //
// ************************************************************************* //

#ifndef VIS_WINDOW_TOOL_PROXY_H
#define VIS_WINDOW_TOOL_PROXY_H
#include <viswindow_exports.h>
#include <VisWindowProtectionProxy.h>

// ****************************************************************************
// Class: VisWindowToolProxy
//
// Purpose:
//   This is a proxy specific to VisWindow tools.  They may use it
//   to access the VisWindow.  This object is not a friend to VisWindow,
//   but it may access it through its base class' methods and friend 
//   status.
//
// Programmer: Brad Whitlock
// Creation:   Tue Oct 2 16:52:41 PST 2001
//
// Modifications:
//
//   Hank Childs, Wed Sep  4 08:40:47 PDT 2002
//   Added RecalculateRenderOrder.
//
//   Kathleen Bonnell, Fri Jun  6 15:31:37 PDT 2003  
//   Added GetFullFrameMode and GetScaleFactorAndType. 
//
//   Kathleen Bonnell, Wed Dec  3 16:49:14 PST 2003
//   Added TransparenciesExist. 
//
// ****************************************************************************

class VISWINDOW_API VisWindowToolProxy : public VisWindowProtectionProxy
{
  public:
                        VisWindowToolProxy(VisWindow *vw) 
                             : VisWindowProtectionProxy(vw) {;};

    void                DisableUpdates()
                                  { ProxiedDisableUpdates(); };

    void                EnableUpdates()
                                  { ProxiedEnableUpdates(); };

    void                EndBoundingBox()
                                  { ProxiedEndBoundingBox(); };

    void                GetForegroundColor(float *fg)
                                  { ProxiedGetForegroundColor(fg); };

    vtkRenderer        *GetBackground()
                                  { return ProxiedGetBackground(); };

    void                GetBounds(float bounds[6])
                                  { ProxiedGetBounds(bounds); };

    vtkRenderer        *GetCanvas()
                                  { return ProxiedGetCanvas(); };

    vtkRenderer        *GetForeground()
                                  { return ProxiedGetForeground(); };

    WINDOW_MODE         GetMode() { return ProxiedGetMode(); };

    bool                HasPlots(){ return ProxiedHasPlots(); };

    void                Render()  { ProxiedRender(); };
    void                RecalculateRenderOrder()
                                  { ProxiedRecalculateRenderOrder(); };

    void                StartBoundingBox()
                                  { ProxiedStartBoundingBox(); };

    bool                UpdatesEnabled()
                                  { return ProxiedUpdatesEnabled(); };

    bool                GetFullFrameMode()
                                  { return ProxiedGetFullFrameMode(); };

    void                GetScaleFactorAndType(double &scale, int &type)
                            { ProxiedGetScaleFactorAndType(scale, type); };

    bool                TransparenciesExist()
                                  { return ProxiedTransparenciesExist(); };
};


#endif


