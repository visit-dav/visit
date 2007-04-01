#ifndef PIPELINE_EXPORTS_H
#define PIPELINE_EXPORTS_H

#if defined(_WIN32)
#ifdef PIPELINE_EXPORTS
#define PIPELINE_API __declspec(dllexport)
#else
#define PIPELINE_API __declspec(dllimport)
#endif
#if defined(USING_MSVC6) || defined(USING_MSVC7)
// Turn off warning about inheritance by dominance.
#pragma warning(disable:4250)
// Turn off warning about lack of DLL interface
#pragma warning(disable:4251)
// Turn off warning non-dll class is base for dll-interface class.
#pragma warning(disable:4275)
// Turn off warning about identifier truncation
#pragma warning(disable:4786)
// Turn off warning about forcing value to bool 'true' or 'false'
#pragma warning(disable:4800)

// Define VISIT_LONG_LONG so the Windows compiler can handle it.
#ifndef VISIT_LONG_LONG
#if defined(USING_MSVC6)
#define VISIT_LONG_LONG __int64
#else
#define VISIT_LONG_LONG long long
#endif
#endif

#endif
#else
#define PIPELINE_API
#ifndef VISIT_LONG_LONG
#define VISIT_LONG_LONG long long
#endif
#endif

#endif
