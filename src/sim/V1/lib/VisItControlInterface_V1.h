/*****************************************************************************
*
* Copyright (c) 2000 - 2009, Lawrence Livermore National Security, LLC
* Produced at the Lawrence Livermore National Laboratory
* LLNL-CODE-400124
* All rights reserved.
*
* This file is  part of VisIt. For  details, see https://visit.llnl.gov/.  The
* full copyright notice is contained in the file COPYRIGHT located at the root
* of the VisIt distribution or at http://www.llnl.gov/visit/copyright.html.
*
* Redistribution  and  use  in  source  and  binary  forms,  with  or  without
* modification, are permitted provided that the following conditions are met:
*
*  - Redistributions of  source code must  retain the above  copyright notice,
*    this list of conditions and the disclaimer below.
*  - Redistributions in binary form must reproduce the above copyright notice,
*    this  list of  conditions  and  the  disclaimer (as noted below)  in  the
*    documentation and/or other materials provided with the distribution.
*  - Neither the name of  the LLNS/LLNL nor the names of  its contributors may
*    be used to endorse or promote products derived from this software without
*    specific prior written permission.
*
* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT  HOLDERS AND CONTRIBUTORS "AS IS"
* AND ANY EXPRESS OR  IMPLIED WARRANTIES, INCLUDING,  BUT NOT  LIMITED TO, THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND  FITNESS FOR A PARTICULAR  PURPOSE
* ARE  DISCLAIMED. IN  NO EVENT  SHALL LAWRENCE  LIVERMORE NATIONAL  SECURITY,
* LLC, THE  U.S.  DEPARTMENT OF  ENERGY  OR  CONTRIBUTORS BE  LIABLE  FOR  ANY
* DIRECT,  INDIRECT,   INCIDENTAL,   SPECIAL,   EXEMPLARY,  OR   CONSEQUENTIAL
* DAMAGES (INCLUDING, BUT NOT  LIMITED TO, PROCUREMENT OF  SUBSTITUTE GOODS OR
* SERVICES; LOSS OF  USE, DATA, OR PROFITS; OR  BUSINESS INTERRUPTION) HOWEVER
* CAUSED  AND  ON  ANY  THEORY  OF  LIABILITY,  WHETHER  IN  CONTRACT,  STRICT
* LIABILITY, OR TORT  (INCLUDING NEGLIGENCE OR OTHERWISE)  ARISING IN ANY  WAY
* OUT OF THE  USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH
* DAMAGE.
*
*****************************************************************************/

#ifndef VISIT_CONTROL_INTERFACE_V1_H
#define VISIT_CONTROL_INTERFACE_V1_H

/*****************************************************************************
 *  File:  VisItControlInterfave_V1.h
 *
 *  Purpose:
 *    Abstraction of VisIt Engine wrapper library.  Handles the
 *    grunt work of actually connecting to visit that must be done
 *    outside of the VisItEngine DLL.
 *
 *  Programmer:  Jeremy Meredith
 *  Creation:    April  4, 2005
 *
 *  Modifications:
 *    Shelly Prevost, Wed Jan 25 08:52:18 PST 2006
 *    Added a guifile argument to VisItInitializeSocketAndDumpSimFile.
 *
 *    Brad Whitlock, Thu Jan 25 14:53:11 PST 2007
 *    Added VisItUpdatePlots and VisItExecuteCommand.
 *
 *****************************************************************************/
#ifdef __cplusplus
extern "C" {
#endif

/******************************************************************************
 * Function: VisItSetBroadcastIntFunction
 *
 * Purpose: 
 *   This function installs a callback function that allows libsim to broadcast
 *   an integer from the root process to slave processes.
 *
 * Arguments: A pointer to a callback function with prototype:
 *            int func(int *, int);
 *
 * Returns:    None
 *
 * Note:       All processors must call this function and install a callback
 *             function after initializing MPI. The callback function must be
 *             installed prior to VisItDetectInput.
 *
 *             Sample callback function:
 *             static int visit_broadcast_int_callback(int *value, int sender)
 *             {
 *                 return MPI_Bcast(value, 1, MPI_INT, sender, MPI_COMM_WORLD);
 *             }
 *
 * ****************************************************************************/
void  VisItSetBroadcastIntFunction(int (*)(int *, int));

/******************************************************************************
 * Function: VisItSetBroadcastStringFunction
 *
 * Purpose: 
 *   This function installs a callback function that allows libsim to broadcast
 *   a character string from the root process to slave processes.
 *
 * Arguments: A pointer to a callback function with prototype:
 *            int func(char *, int, int);
 *
 * Returns:    None
 *
 * Note:       All processors must call this function and install a callback
 *             function after initializing MPI. The callback function must be
 *             installed prior to VisItDetectInput.
 *
 *             Sample callback function:
 *             static int visit_broadcast_string_callback(char *str, int len, int sender)
 *             {
 *                 return MPI_Bcast(str, len, MPI_CHAR, sender, MPI_COMM_WORLD);
 *             }      
 *
 * ****************************************************************************/
void  VisItSetBroadcastStringFunction(int (*)(char *, int, int));

/******************************************************************************
 * Function: VisItSetParallel
 *
 * Purpose: 
 *   Set whether or not libsim will be operating in parallel.
 *
 * Arguments:
 *   flag : Pass a non-zero value to indicate parallel; 0 for serial operation.
 *
 * Returns:    None
 *
 * Note:       All processors must call this function. Only parallel simulations
 *             need to call this function.
 *
 * ****************************************************************************/
void  VisItSetParallel(int);

/******************************************************************************
 * Function: VisItSetParallelRank
 *
 * Purpose: 
 *   Set the rank of the current process within its MPI communicator.
 *
 * Arguments:
 *   rank : The MPI rank of the process.
 *
 * Returns:    None
 *
 * Note:       All processors must call this function. Only parallel simulations
 *             need to call this function.
 *
 * ****************************************************************************/
void  VisItSetParallelRank(int);

/******************************************************************************
 * Function: VisItSetDirectory
 *
 * Purpose: 
 *   Set the path to the top level directory where VisIt is installed. This lets
 *   libsim load runtime libraries from a specific version of VisIt. If you never 
 *   call this function, libsim will obtain VisIt runtime information using the 
 *   "visit" script in your path. If that is the case, the latest runtime libraries
 *   will be used but it may not match the version of the client that is trying
 *   to connect to VisIt. Version mismatches are avoided (but not eliminated) by
 *   only using installed versions of VisIt to connect to a simulation.
 *
 * Arguments:
 *   path : The path to the top level VisIt directory. This does not include the
 *          "bin/visit" part of the path; just "/path/to/visitdir".
 *
 * Returns:    None
 *
 * Note:       All processors must call this function. This function must be
 *             called before VisItSetupEnvironment.
 *
 * ****************************************************************************/

void  VisItSetDirectory(char *);

/******************************************************************************
 * Function: VisItSetOptions
 *
 * Purpose: 
 *   Pass command line arguments that will be used when calling out to VisIt to
 *   determine the runtime libraries that libsim will need to load when VisIt
 *   connects. You can use this function to pass arguments such as 
 *   "-forceversion 1.12.0" to force a specific version of VisIt.
 *
 * Arguments:
 *   args : A null-terminated string containing additional arguments will be
 *          passed to VisIt when determining the runtime libraries that need to
 *          be loaded when VisIt connects to the simulation.
 *
 * Returns:    None
 *
 * Note:       All processors must call this function. This function must be
 *             called before VisItSetupEnvironment.
 *
 * ****************************************************************************/
void  VisItSetOptions(char *);

/******************************************************************************
 * Function: VisItSetupEnvironment
 *
 * Purpose: 
 *   Sets up the environment so VisIt can be loaded into libsim when the
 *   VisIt client wants to connect
 *
 * Arguments: None
 *
 * Returns:   None
 *
 * Note:      All processors must call this function and they must call it
 *            before VisItInitializeSocketAndDumpSimFile and VisItDetectInput.
 *
 * ****************************************************************************/
int   VisItSetupEnvironment(void);

/******************************************************************************
 * Function: VisItInitializeSocketAndDumpSimFile
 *
 * Purpose: 
 *   This function makes the simulation start listening for inbound VisIt
 *   socket connections and it writes a .sim1 file that tells VisIt how to
 *   connect to the simulation.
 *
 * Arguments:
 *   name             : The name of the simulation, which will be used in the 
 *                      created filename.
 *   comment          : A comment describing the simulation.
 *   path             : The path where the simulation was started.
 *   inputfile        : reserved, pass NULL
 *   guifile          : reserved, pass NULL
 *   absoluteFilename : reserved, pass NULL
 *
 * Returns:    1 on success, 0 on failure
 *
 * Note:       Only the root processor should call this function. This function
 *             should be called early on and before the calls to VisItDetectInput.
 *
 * ****************************************************************************/
int   VisItInitializeSocketAndDumpSimFile(const char *name,
                                          const char *comment,
                                          const char *path,
                                          const char *inputfile,
                                          const char *guifile,
                                          const char *absoluteFilename);

/******************************************************************************
 * Function: VisItDetectInput
 *
 * Purpose: 
 *   Simulations call this function to detect input from the listen socket,
 *   client socket, or console. Call this function in a loop to form the main
 *   event loop for a simulation.
 *
 * Arguments:
 *   blocking : Pass a non-zero value to tell the function to wait until there
 *              is input before returning. If you want to run the simulation
 *              without waiting for input, you will want to pass 0 so the 
 *              function times out and returns whether or not there is input
 *              to be handled.
 *
 *  consoledesc : If you want to handle input from another file in addition
 *                to the VisIt sockets, pass a file descriptor for the file
 *                that you want to monitor. If you want to monitor the console
 *                for typed commands, pass fileno(stdin). If you do not want
 *                to monitor other files, pass -1 for the descriptor.
 *                
 * Returns:
 *              -5: Logic error (fell through all cases)
 *              -4: Logic error (no descriptors but blocking)
 *              -3: Logic error (a socket was selected but not one we set)
 *              -2: Unknown error in select
 *              -1: Interrupted by EINTR in select
 *               0: Okay - Timed out
 *               1: Listen  socket input
 *               2: Engine  socket input
 *               3: Console socket input
 *
 * Note:        This function should only be called by the root process in
 *              parallel. The results of this function should be broadcast to
 *              other processors so that all may follow the same general call
 *              pattern.
 *
 *              What to do with a return value:
 *              0 : The function timed out; execute a time step
 *              1 : An inbound VisIt connection is being made. Call the
 *                  VisItAttemptToCompleteConnection function.
 *              2 : The VisIt viewer sent instructions to the simulation so
 *                  call the VisItProcessEngineCommand function.
 *              3 : Console input was detected so read stdin and handle it.
 *              other : error out, stop calling VisItDetectInput.
 *              
 * ****************************************************************************/
int   VisItDetectInput(int blocking, int consoledesc);

/******************************************************************************
 * Function: VisItAttemptToCompleteConnection
 *
 * Purpose: 
 *   Accept the inbound VisIt connection socket, verify security keys, get the 
 *   connection parameters from the client, load the VisIt engine library,
 *   create the Engine and connect back to the VisIt viewer.
 *
 * Arguments: None
 *
 * Returns:   1 on success; 0 on failure
 *
 * Note:      This function should be called when VisItDetectInput returns 1.
 *            Only the root processor should call this function.
 *
 * ****************************************************************************/
int   VisItAttemptToCompleteConnection(void);

/******************************************************************************
 * Function: VisItSetSlaveProcessCallback
 *
 * Purpose: 
 *   Set the callback function used to inform slave processes that they should
 *   call VisItProcessEngineCommand. The provided callback function is used 
 *   internally in libsim
 *   
 *
 * Arguments:  A pointer to a function with prototype: void func(void);
 *
 * Returns:    None
 *
 * Note:       The slave process callback is required for a parallel simulation.
 *             This function should be called when VisItAttemptToCompleteConnection
 *             returns successfully.
 *
 *             MPI simulations may define the callback like this:
 *                 void slave_process_callback()
 *                 {
 *                     int command = 0;
 *                     MPI_BCast(&command, 1, MPI_INT, 0, MPI_COMM_WORLD);
 *                 }
 *
 * ****************************************************************************/
void  VisItSetSlaveProcessCallback(void(*)(void));

/******************************************************************************
 * Function: VisItSetCommandCallback
 *
 * Purpose: 
 *   Set the callback for processing control commands (these are set up in
 *   the simulation's metadata).
 *
 * Arguments: A pointer to a function with prototype: 
 *            void func(const char*,int,float,const char*);
 *
 *            The callback function arguments are:
 *            cmd   : The name of the command being called.
 *            idata : integer argument (reserved)
 *            fdata : float argument (reserved)
 *            sdata : string argument (reserved)
 *
 * Returns:    None
 *
 * Note:       This function must be called on all processors to install the
 *             callback function. This function should be called when 
 *             VisItAttemptToCompleteConnection returns successfully.
 *
 * ****************************************************************************/
void  VisItSetCommandCallback(void(*)(const char*,int,float,const char*));

/******************************************************************************
 * Function: VisItProcessEngineCommand
 *
 * Purpose: 
 *   This function reads input from VisIt's viewer and executes the requests.
 *   The VisItProcessEngineCommand function needs to be called from the
 *   simulation's event loop when VisItDetectInput return 2, indicating that
 *   there is input to be processed.
 *
 * Arguments: None
 *
 * Returns:   1 on success; 0 on failure.
 *
 * Note:      All processors must call this function.
 *
 * ****************************************************************************/
int   VisItProcessEngineCommand(void);

/******************************************************************************
 * Function: VisItTimeStepChanged
 *
 * Purpose: 
 *   The simulation can use this function to tell VisIt that it has changed to
 *   a new time step. This causes the simulation to send new metadata to 
 *   VisIt.
 *
 * Arguments: None
 *
 * Returns:   None
 *
 * Note:      All processors must call this function.
 *
 * ****************************************************************************/
void  VisItTimeStepChanged(void);

/******************************************************************************
 * Function: VisItUpdatePlots
 *
 * Purpose: 
 *   The simulation can use this function to tell VisIt to update its plots
 *   using new data from the simulation. Calling this function only serves as 
 *   a trigger to VisIt that it needs to update its plots. The simulation should
 *   take care to pause until VisIt has made all of its requests from the
 *   simulation or multiple plots may contain data from different time steps
 *   if the simulation has kept running.
 *
 * Arguments: None
 *
 * Returns:   None
 *
 * Note:      All processors must call this function.
 *
 * ****************************************************************************/
void  VisItUpdatePlots(void);

/******************************************************************************
 * Function: VisItExecuteCommand
 *
 * Purpose: 
 *   This simulation can use this function to tell VisIt to execute VisIt CLI
 *   Python commands. The commands are sent to VisIt in a non-blocking fashion
 *   and VisIt later translates the commands into requests to the simulation.
 *
 * Arguments: A null-terminated character string containing the commands to
 *            be executed.
 *
 * Returns:   None
 *
 * Note:      And example call to this function could look like:
 *            VisItExecuteCommand("AddPlot(\"Pseudocolor\", \"zonal\")\n");
 *
 *            Only the root processor should call this function.
 *
 * ****************************************************************************/
void  VisItExecuteCommand(const char *);

/******************************************************************************
 * Function: VisItDisconnect
 *
 * Purpose: 
 *   This function disconnects the simulation from VisIt. It should be called
 *   from the simulation's event loop when VisItProcessEngineCommand returns 
 *   failure.
 *
 * Arguments: None
 *
 * Returns:   None
 *
 * Note:      All processors must call this function.
 *
 * ****************************************************************************/
void  VisItDisconnect(void);

/******************************************************************************
 * Function: VisItGetLastError
 *
 * Purpose: 
 *   This function returns a pointer to a character string that contains the
 *   last error that libsim encountered.
 *
 * Arguments: None
 *
 * Returns:    A null-terminated character string that contains the last error.
 *
 * Note:       
 *
 * ****************************************************************************/
char *VisItGetLastError(void);

#ifdef __cplusplus
}
#endif

#endif
