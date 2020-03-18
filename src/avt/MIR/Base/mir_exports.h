// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#ifndef MIR_EXPORTS_H
#define MIR_EXPORTS_H

#if defined(_WIN32)
#if !defined(VISIT_STATIC)
#if defined(AVTMIR_EXPORTS) || defined(avtmir_ser_EXPORTS) || defined(avtmir_par_EXPORTS)
#define MIR_API __declspec(dllexport)
#else
#define MIR_API __declspec(dllimport)
#endif
#else
#define MIR_API
#endif
#if defined(_MSC_VER)
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
# if __GNUC__ >= 4 && (defined(AVTMIR_EXPORTS) || defined(avtmir_ser_EXPORTS) || defined(avtmir_par_EXPORTS))
#   define MIR_API __attribute__ ((visibility("default")))
# else
#   define MIR_API /* hidden by default */
# endif
#endif

#endif
