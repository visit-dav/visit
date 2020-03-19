// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#ifndef MISC_EXPORTS_H
#define MISC_EXPORTS_H

#if defined(_WIN32)
#if !defined(VISIT_STATIC)
# if defined(MISC_EXPORTS) || defined(visitcommon_EXPORTS)
#   define MISC_API  __declspec(dllexport)
#   define MISC_API2 __declspec(dllexport)
# else
#   define MISC_API  __declspec(dllimport)
#   define MISC_API2 __declspec(dllimport)
# endif
#else
#define MISC_API
#define MISC_API2
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
# if __GNUC__ >= 4
#    if (defined(MISC_EXPORTS) || defined(visitcommon_EXPORTS))
#      define MISC_API __attribute__ ((visibility("default")))
#    else
#      define MISC_API /* hidden by default */
#    endif
#    define MISC_API2 __attribute__ ((visibility("default"))) /* Always visible */
# else
#   define MISC_API  /* hidden by default */
#   define MISC_API2 /* hidden by default */
# endif
#endif

#endif
