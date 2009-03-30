#ifndef VISIT_DATA_INTERFACE_RUNTIME_P_H
#define VISIT_DATA_INTERFACE_RUNTIME_P_H
#include <stdlib.h>
#include <VisItInterfaceTypes_V2.h>

/* This file contains prototypes of functions that are used internally 
   in the data interface. These functions are never exposed beyond the
   runtime.
 */
#define VISIT_DOMAIN_BOUNDARIES 13
#define VISIT_DOMAIN_NESTING    14

#define VISIT_OBJECT_HEAD                int objectType;
#define VISIT_OBJECT_CHECK_TYPE(obj,ID)  (obj->objectType == ID)
#define VISIT_OBJECT_INITIALIZE(obj, ID) obj->objectType = ID
#define VISIT_OBJECT_ALLOCATE(T)         (T *)calloc(1, sizeof(T))

void        *VisItGetPointer(visit_handle h);
void         VisItFreePointer(visit_handle h);
visit_handle VisItStorePointer(void *ptr);

void VisItError(const char *msg);

#endif
