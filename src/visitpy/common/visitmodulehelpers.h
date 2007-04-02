#ifndef VISIT_MODULE_HELPERS_H
#define VISIT_MODULE_HELPERS_H
#include <ViewerProxy.h>

//
// Visible Prototypes.
//
void VisItErrorFunc(const char *errString);
ViewerProxy   *GetViewerProxy();
ViewerState   *GetViewerState();
ViewerMethods *GetViewerMethods();

#endif
