#ifndef ENGINE_PARSTATE_EXPORTS_H
#define ENGINE_PARSTATE_EXPORTS_H

#if defined(_WIN32)
#ifdef ENGINE_PARSTATE_EXPORTS
#define ENGINE_PARSTATE_API __declspec(dllexport)
#else
#define ENGINE_PARSTATE_API __declspec(dllimport)
#endif
#else
#define ENGINE_PARSTATE_API
#endif

#endif
