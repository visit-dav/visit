#ifndef REMOTE_COMMAND_H
#define REMOTE_COMMAND_H

#ifdef __cplusplus
extern "C" {
#endif

#if defined(_WIN32)
#ifdef REMOTE_COMMAND_EXPORTS
#define REMOTE_COMMAND_API __declspec(dllexport)
#else
#define REMOTE_COMMAND_API __declspec(dllimport)
#endif
#else
#define REMOTE_COMMAND_API
#endif

/* Types. */
typedef const char *passwordCallback(const char *host, int *okay);

/* Prototypes. */
REMOTE_COMMAND_API int RunRemoteCommand(const char *username,
    const char *host, const char *commands[], int nCommands,
    passwordCallback *cb, int initSockets);

#ifdef __cplusplus
};
#endif

#endif
