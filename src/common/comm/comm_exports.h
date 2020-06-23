// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#ifndef COMM_EXPORTS_H
#define COMM_EXPORTS_H

#if defined(_WIN32)
# define DESCRIPTOR unsigned int
#if !defined(VISIT_STATIC)
# if defined(COMM_EXPORTS) || defined(visitcommon_EXPORTS)
#   define COMM_API  __declspec(dllexport)
#   define COMM_API2 __declspec(dllexport)
# else
#   define COMM_API  __declspec(dllimport)
#   define COMM_API2 __declspec(dllimport)
# endif
#else
#define COMM_API
#define COMM_API2
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
# define DESCRIPTOR int
# if __GNUC__ >= 4
#   if (defined(COMM_EXPORTS) || defined(visitcommon_EXPORTS))
#     define COMM_API __attribute__ ((visibility("default")))
#   else
#     define COMM_API /* hidden by default */
#   endif
#   define COMM_API2 __attribute__ ((visibility("default"))) /* Always visible */
# else
#   define COMM_API  /* hidden by default */
#   define COMM_API2 /* hidden by default */
# endif
#endif

#endif
