/*****************************************************************************
* Copyright (c) 2014 Intelligent Light. All rights reserved.
* Work partially supported by DOE Grant SC0007548.
* FieldView XDB Export is provided expressly for use within VisIt.
* All other uses are strictly forbidden.
*****************************************************************************/
#ifndef VXDB_EXPORTS_H
#define VXDB_EXPORTS_H

#if defined(_WIN32)
#if defined(VXDB_ser_EXPORTS) || defined(VXDB_par_EXPORTS)
#define VXDB_API __declspec(dllexport)
#else
#define VXDB_API __declspec(dllimport)
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
# if __GNUC__ >= 4 && (defined(VXDB_ser_EXPORTS) || defined(VXDB_par_EXPORTS))
#   define VXDB_API __attribute__ ((visibility("default")))
# else
#   define VXDB_API /* hidden by default */
# endif
#endif

#endif
