// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#ifndef PREP_EXPORTS_H
#define PREP_EXPORTS_H

#if defined(_WIN32)
#ifdef PREP_EXPORTS
#define PREP_API __declspec(dllexport)
#else
#define PREP_API __declspec(dllimport)
#endif
#else
# if __GNUC__ >= 4 && defined(PREP_EXPORTS)
#   define PREP_API __attribute__ ((visibility("default")))
# else
#   define PREP_API /* hidden by default */
# endif
#endif

#endif
