#ifndef COMM_EXPORTS_H
#define COMM_EXPORTS_H

#if defined(_WIN32)
#define DESCRIPTOR unsigned int
#ifdef COMM_EXPORTS
#define COMM_API __declspec(dllexport)
#else
#define COMM_API __declspec(dllimport)
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
#define DESCRIPTOR int
#define COMM_API
#endif

#endif
