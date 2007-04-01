#ifndef DATABASE_EXPORTS_H
#define DATABASE_EXPORTS_H

#if defined(_WIN32)
#ifdef DATABASE_EXPORTS
#define DATABASE_API __declspec(dllexport)
#else
#define DATABASE_API __declspec(dllimport)
#endif
#if defined(USING_MSVC6) || defined(USING_MSVC7)
// Turn off warning about possible loss of data
#pragma warning(disable:4244)
// Turn off warning about inheritance via dominance
#pragma warning(disable:4250)
// Turn off warning about lack of DLL interface
#pragma warning(disable:4251)
// Turn off warning non-dll class is base for dll-interface class.
#pragma warning(disable:4275)
// Turn off warning initializing float with truncated const double
#pragma warning(disable:4305)
// Turn off warning about identifier truncation
#pragma warning(disable:4786)
#endif
#else
#define DATABASE_API
#endif

#endif
