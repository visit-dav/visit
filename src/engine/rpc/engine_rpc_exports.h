// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#ifndef ENGINE_RPC_EXPORTS_H
#define ENGINE_RPC_EXPORTS_H

#if defined(_WIN32)
#if !defined(VISIT_STATIC)
#if defined(ENGINE_RPC_EXPORTS) || defined (enginerpc_EXPORTS)
#define ENGINE_RPC_API __declspec(dllexport)
#else
#define ENGINE_RPC_API __declspec(dllimport)
#endif
#else
#define ENGINE_RPC_API
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
# if __GNUC__ >= 4 && (defined(ENGINE_RPC_EXPORTS) || defined (enginerpc_EXPORTS))
#   define ENGINE_RPC_API __attribute__ ((visibility("default")))
# else
#   define ENGINE_RPC_API /* hidden by default */
# endif
#endif

#endif
