// ************************************************************************* //
//                           VisWindowRenderProxy.h                          //
// ************************************************************************* //

#ifndef VIS_WINDOW_RENDER_PROXY_H
#define VIS_WINDOW_RENDER_PROXY_H
#include <viswindow_exports.h>


#include <VisWindowProtectionProxy.h>


// ****************************************************************************
//  Class: VisWindowRenderProxy
//
//  Purpose:
//      This is a proxy specific to constructs that aid in VisWindow's 
//      rendering.  They may use it to access the VisWindow.  This object is 
//      not a friend to VisWindow, but it may access it through its base class'
//      methods and friend status.
//
//  Programmer: Hank Childs
//  Creation:   June 9, 2000
//
// ****************************************************************************

class VISWINDOW_API VisWindowRenderProxy : public VisWindowProtectionProxy
{
  public:
                        VisWindowRenderProxy(VisWindow *vw) 
                             : VisWindowProtectionProxy(vw) {;};

    void                StartRender(void) { ProxiedStartRender(); };
};


#endif


