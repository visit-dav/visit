#ifndef PREP_EXPORTS_H
#define PREP_EXPORTS_H

#if defined(_WIN32)
#ifdef PREP_EXPORTS
#define PREP_API __declspec(dllexport)
#else
#define PREP_API __declspec(dllimport)
#endif
#else
#define PREP_API
#endif

#endif
