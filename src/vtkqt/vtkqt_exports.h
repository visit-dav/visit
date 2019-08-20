// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#ifndef VTKQT_EXPORTS_H
#define VTKQT_EXPORTS_H

#if defined(_WIN32)
#if defined(VTKQT_EXPORTS) || defined(vtkqt_EXPORTS)
#define VTKQT_API __declspec(dllexport)
#else
#define VTKQT_API __declspec(dllimport)
#endif
#else
# if __GNUC__ >= 4 && (defined(VTKQT_EXPORTS) || defined(vtkqt_EXPORTS))
#   define VTKQT_API __attribute__ ((visibility("default")))
# else
#   define VTKQT_API /* hidden by default */
# endif
#endif

#endif
