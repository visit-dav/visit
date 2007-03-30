#ifndef VIS_CALLBACK_H
#define VIS_CALLBACK_H

#include <avtDataObject.h>

//
// This type is used for callbacks that need to return an avtDataObject
//

typedef   void  (VisCallbackWithDob)(void *, avtDataObject_p&);

//
// This is a type used for simple callback functions.
//

typedef void (VisCallback)(void *);

#endif
