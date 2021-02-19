// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#ifndef AVTEXCEPTION_EXPORTS_H
#define AVTEXCEPTION_EXPORTS_H

#if defined(_WIN32)
#if !defined(VISIT_STATIC)
# if defined(AVTEXCEPTION_EXPORTS) || defined(visitcommon_EXPORTS)
#   define AVTEXCEPTION_API __declspec(dllexport)
# else
#   define AVTEXCEPTION_API __declspec(dllimport)
# endif
#else
#define AVTEXCEPTION_API
#endif
# if defined(_MSC_VER)
// Turn off warning about lack of DLL interface
#   pragma warning(disable:4251)
// Turn off warning non-dll class is base for dll-interface class.
#   pragma warning(disable:4275)
// Turn off warning about identifier truncation
#   pragma warning(disable:4786)
# endif
#else
// Exceptions are a special case: they must always be public, for proper
// typeinfo lookups!
# if __GNUC__ >= 4
#   define AVTEXCEPTION_API __attribute__ ((visibility("default")))
# else
#   define AVTEXCEPTION_API /* no visibility support */
# endif
#endif

#endif
