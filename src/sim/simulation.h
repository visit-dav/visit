#ifndef SIMULATION_H
#define SIMULATION_H

// ****************************************************************************
//  File:  simulation.h
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
// ****************************************************************************


int  AttemptToCompleteConnection();
void InitializeSocketAndDumpSimFile(const char *name);
void ProcessEngineCommand();
void AddVisItLibraryPaths(int argc, char *argv[]);
void Disconnect();
void TimeStepChanged();

#endif
