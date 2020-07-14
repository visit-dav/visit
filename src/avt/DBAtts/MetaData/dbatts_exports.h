// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#ifndef DBATTS_EXPORTS_H
#define DBATTS_EXPORTS_H

#if defined(_WIN32)
#if !defined(VISIT_STATIC)
#if defined(AVTDBATTS_EXPORTS) || defined(avtdbatts_EXPORTS)
#define DBATTS_API __declspec(dllexport)
#else
#define DBATTS_API __declspec(dllimport)
#endif
#else
#define DBATTS_API
#endif
#if defined(_MSC_VER)
// Turn off warning about signed/unsigned comparison.
#pragma warning(disable:4018)
// Turn off warning about lack of DLL interface.
#pragma warning(disable:4251)
// Turn off warning non-dll class is base for dll-interface class.
#pragma warning(disable:4275)
// Turn off warning about identifier truncation.
#pragma warning(disable:4786)
#endif
#else
# if __GNUC__ >= 4 && (defined(AVTDBATTS_EXPORTS) || defined(avtdbatts_EXPORTS))
#   define DBATTS_API __attribute__ ((visibility("default")))
# else
#   define DBATTS_API /* hidden by default */
# endif
#endif

#endif
