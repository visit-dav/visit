#ifndef MISC_EXPORTS_H
#define MISC_EXPORTS_H

#if defined(_WIN32)
#ifdef MISC_EXPORTS
#define MISC_API __declspec(dllexport)
#else
#define MISC_API __declspec(dllimport)
#endif
#ifdef USING_MSVC6
// Turn off warning about lack of DLL interface
#pragma warning(disable:4251)
// Turn off warning non-dll class is base for dll-interface class.
#pragma warning(disable:4275)
// Turn off warning about identifier truncation
#pragma warning(disable:4786)
#endif
#else
#define MISC_API
#endif

#endif
