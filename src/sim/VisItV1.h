#ifndef VisItV1_H
#define VisItV1_H

/* ****************************************************************************
//  File:  VisItV1.h
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
//  Creation:    April  4, 2005
//
// ***************************************************************************/
#ifdef __cplusplus
extern "C" {
#endif

int   VisItAttemptToCompleteConnection(void);
void  VisItInitializeSocketAndDumpSimFile(char *name,
                                         char *comment,
                                         char *path,
                                         char *inputfile);
int   VisItGetListenSocket(void);
int   VisItGetEngineSocket(void);
int   VisItProcessEngineCommand(void);
void  VisItAddLibraryPaths(int argc, char *argv[]);
void  VisItDisconnect(void);
void  VisItTimeStepChanged(void);
void  VisItSetupEnvironment(void);
void  VisItSetSlaveProcessCallback(void(*)());
void  VisItSetCommandCallback(void(*)(const char*,int,float,const char*));

#ifdef __cplusplus
}
#endif

#endif
