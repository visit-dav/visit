#ifndef VisIt_H
#define VisIt_H

/* ****************************************************************************
//  File:  VisIt.h
//
//  Purpose:
//    Abstraction of VisIt Engine wrapper library.  Handles the
//    grunt work of actually connecting to visit that must be done
//    outside of the VisItEngine DLL.
//       1) setting up a non-blocking listen socket
//       2) writing a .sim file
//       3) opening the VisItEngine .so and retrieving the functions from it
//       4) accepting an incoming socket connection
//       5) removing the .sim file when the program exits
//
//  Programmer:  Jeremy Meredith
//  Creation:    August 25, 2004
//
// ***************************************************************************/
#ifdef __cplusplus
extern "C" {
#endif

int  VisItAttemptToCompleteConnection(void);
void VisItInitializeSocketAndDumpSimFile(char *name);
int  VisItGetListenSocket(void);
int  VisItGetEngineSocket(void);
int  VisItProcessEngineCommand(void);
void VisItAddLibraryPaths(int argc, char *argv[]);
void VisItDisconnect(void);
void VisItTimeStepChanged(void);
void VisItSetupEnvironment(void);
void VisItSetSlaveProcessCallback(void(*)());

#ifdef __cplusplus
}
#endif

#endif
