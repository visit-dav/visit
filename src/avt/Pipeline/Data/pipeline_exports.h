// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#ifndef PIPELINE_EXPORTS_H
#define PIPELINE_EXPORTS_H

#if defined(_WIN32)
#if !defined(VISIT_STATIC)
#if defined(AVTPIPELINE_EXPORTS) || defined(avtpipeline_ser_EXPORTS) || defined(avtpipeline_par_EXPORTS)
#define PIPELINE_API __declspec(dllexport)
#else
#define PIPELINE_API __declspec(dllimport)
#endif
#else
#define PIPELINE_API
#endif
#if defined(_MSC_VER)
// Turn off warning about inheritance by dominance.
#pragma warning(disable:4250)
// Turn off warning about lack of DLL interface
#pragma warning(disable:4251)
// Turn off warning non-dll class is base for dll-interface class.
#pragma warning(disable:4275)
// Turn off warning about identifier truncation
#pragma warning(disable:4786)
// Turn off warning about forcing value to bool 'true' or 'false'
#pragma warning(disable:4800)

// Define VISIT_LONG_LONG so the Windows compiler can handle it.
#ifndef VISIT_LONG_LONG
#if defined(_MSC_VER) && (_MSC_VER <= 1200)
#define VISIT_LONG_LONG __int64
#else
#define VISIT_LONG_LONG long long
#endif
#endif

#endif
#else
# if __GNUC__ >= 4 && (defined(AVTPIPELINE_EXPORTS) || defined(avtpipeline_ser_EXPORTS) || defined(avtpipeline_par_EXPORTS))
#   define PIPELINE_API __attribute__ ((visibility("default")))
# else
#   define PIPELINE_API /* hidden by default */
# endif
#ifndef VISIT_LONG_LONG
#define VISIT_LONG_LONG long long
#endif
#endif

#endif
