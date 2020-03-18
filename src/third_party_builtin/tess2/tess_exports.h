// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#if defined(_WIN32)
  #if !defined(VISIT_STATIC)
    #if defined(tess2_EXPORTS)
      #define TESS2_API __declspec(dllexport)
    #else
      #define TESS2_API __declspec(dllimport)
    #endif
  #else
    #define TESS2_API
  #endif
#else
  #ifdef __cplusplus
    #if __GNUC__ >= 4 && defined(tess2_EXPORTS)
    # define TESS2_API  extern "C" __attribute__ ((visibility("default")))
    #else
    # define TESS2_API  extern "C"
    #endif
  #else
    #define TESS2_API
  #endif
#endif

