#ifndef SILOOBJ_VTK_EXPORTS_H
#define SILOOBJ_VTK_EXPORTS_H

#if defined(_WIN32)
#ifdef SILOOBJ_VTK_EXPORTS
#define SILOOBJ_VTK_API __declspec(dllexport)
#else
#define SILOOBJ_VTK_API __declspec(dllimport)
#endif
#else
#define SILOOBJ_VTK_API
#endif

#endif
