#ifndef SPLASH_RPC_EXPORTS_H
#define SPLASH_RPC_EXPORTS_H

#if defined(_WIN32)
#ifdef SPLASH_RPC_EXPORTS
#define SPLASH_RPC_API __declspec(dllexport)
#else
#define SPLASH_RPC_API __declspec(dllimport)
#endif
#else
#define SPLASH_RPC_API
#endif

#endif
