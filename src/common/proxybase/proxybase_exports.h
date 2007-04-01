#ifndef PROXYBASE_EXPORTS_H
#define PROXYBASE_EXPORTS_H

#if defined(_WIN32)
#ifdef PROXYBASE_EXPORTS
#define PROXYBASE_API __declspec(dllexport)
#else
#define PROXYBASE_API __declspec(dllimport)
#endif
#if defined(USING_MSVC6) || defined(USING_MSVC7)
// Turn off warning about lack of DLL interface
#pragma warning(disable:4251)
// Turn off warning non-dll class is base for dll-interface class.
#pragma warning(disable:4275)
// Turn off warning about identifier truncation
#pragma warning(disable:4786)
#endif
#else
#define PROXYBASE_API
#endif

#endif
