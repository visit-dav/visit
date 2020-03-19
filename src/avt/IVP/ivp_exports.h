// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#ifndef IVP_EXPORTS_H
#define IVP_EXPORTS_H

#if defined(_WIN32)
#if !defined(VISIT_STATIC)
#if defined(AVTIVP_EXPORTS) || defined(avtivp_ser_EXPORTS) || defined(avtivp_par_EXPORTS)
#define IVP_API __declspec(dllexport)
#else
#define IVP_API __declspec(dllimport)
#endif
#else
#define IVP_API
#endif
#if defined(_MSC_VER)
// Turn off warning about lack of DLL interface
#pragma warning(disable:4251)
// Turn off warning non-dll class is base for dll-interface class.
#pragma warning(disable:4275)
// Turn off warning about identifier truncation
#pragma warning(disable:4786)
#endif
#else
# if __GNUC__ >= 4 && (defined(AVTIVP_EXPORTS) || defined(avtivp_ser_EXPORTS) || defined(avtivp_par_EXPORTS))
#   define IVP_API __attribute__ ((visibility("default")))
# else
#   define IVP_API /* hidden by default */
# endif
#endif

#endif
