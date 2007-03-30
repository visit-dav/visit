#ifndef VTKQT_EXPORTS_H
#define VTKQT_EXPORTS_H

#if defined(_WIN32)
#ifdef VTKQT_EXPORTS
#define VTKQT_API __declspec(dllexport)
#else
#define VTKQT_API __declspec(dllimport)
#endif
#else
#define VTKQT_API
#endif

#endif
