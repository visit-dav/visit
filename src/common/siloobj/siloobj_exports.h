#ifndef SILOOBJ_EXPORTS_H
#define SILOOBJ_EXPORTS_H

#if defined(_WIN32)
#ifdef SILOOBJ_EXPORTS
#define SILOOBJ_API __declspec(dllexport)
#else
#define SILOOBJ_API __declspec(dllimport)
#endif
#else
#define SILOOBJ_API
#endif

#endif
