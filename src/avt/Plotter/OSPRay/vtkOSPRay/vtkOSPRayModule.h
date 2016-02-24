
#ifndef VTKOSPRAY_EXPORT_H
#define VTKOSPRAY_EXPORT_H

#ifdef VTKOSPRAY_STATIC_DEFINE
#  define VTKOSPRAY_EXPORT
#  define VTKOSPRAY_NO_EXPORT
#else
#  ifndef VTKOSPRAY_EXPORT
#    ifdef vtkOSPRay_EXPORTS
        /* We are building this library */
#      define VTKOSPRAY_EXPORT __attribute__((visibility("default")))
#    else
        /* We are using this library */
#      define VTKOSPRAY_EXPORT __attribute__((visibility("default")))
#    endif
#  endif

#  ifndef VTKOSPRAY_NO_EXPORT
#    define VTKOSPRAY_NO_EXPORT __attribute__((visibility("hidden")))
#  endif
#endif

#ifndef VTKOSPRAY_DEPRECATED
#  define VTKOSPRAY_DEPRECATED __attribute__ ((__deprecated__))
#  define VTKOSPRAY_DEPRECATED_EXPORT VTKOSPRAY_EXPORT __attribute__ ((__deprecated__))
#  define VTKOSPRAY_DEPRECATED_NO_EXPORT VTKOSPRAY_NO_EXPORT __attribute__ ((__deprecated__))
#endif

#define DEFINE_NO_DEPRECATED 0
#if DEFINE_NO_DEPRECATED
# define VTKOSPRAY_NO_DEPRECATED
#endif

/* AutoInit dependencies.  */
// #include "vtkFiltersCoreModule.h"
// #include "vtkFiltersHybridModule.h"
//#include "vtkPVClientServerCoreRenderingModule.h"
//#include "vtkRenderingOpenGLModule.h"

#endif
