// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#ifndef VISIT_CONTROL_INTERFACE_V2_H
#define VISIT_CONTROL_INTERFACE_V2_H
#include <VisItInterfaceTypes_V2.h>

/*****************************************************************************
 *  File:  VisItControlInterface_V2.h
 *
 *  Purpose:
 *    Abstraction of VisIt Engine wrapper library.  Handles the
 *    grunt work of actually connecting to visit that must be done
 *    outside of the VisItEngine DLL.
 *
 *  Programmer:  Brad Whitlock,
 *  Creation:    Thu Mar 11 14:42:24 PST 2010
 *
 *  Modifications:
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
/* DEPRECATED */
void  VisItSetBroadcastIntFunction(int (*bicb)(int *, int));

/******************************************************************************
 * Function: VisItSetBroadcastIntFunction2
 *
 * Purpose:
 *   This function installs a callback function that allows libsim to broadcast
 *   an integer from the root process to slave processes.
 *
 * Arguments: A pointer to a callback function with prototype:
 *            int func(int *, int, void *);
 *
 * Returns:    None
 *
 * Note:       All processors must call this function and install a callback
 *             function after initializing MPI. The callback function must be
 *             installed prior to VisItDetectInput.
 *
 *             Sample callback function:
 *             static int visit_broadcast_int_callback(int *value, int sender,
 *                                                     void *ptr)
 *             {
 *                 simdata_t *sim = (simdata_t *)ptr;
 *                 return MPI_Bcast(value, 1, MPI_INT, sender, sim->communicator);
 *             }
 *
 * ****************************************************************************/
void  VisItSetBroadcastIntFunction2(int (*cb)(int *, int, void *), void *);

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
/* DEPRECATED */
void  VisItSetBroadcastStringFunction(int (*bscb)(char *, int, int));

/******************************************************************************
 * Function: VisItSetBroadcastStringFunction2
 *
 * Purpose:
 *   This function installs a callback function that allows libsim to broadcast
 *   a character string from the root process to slave processes.
 *
 * Arguments: A pointer to a callback function with prototype:
 *            int func(char *, int, int, void *);
 *
 * Returns:    None
 *
 * Note:       All processors must call this function and install a callback
 *             function after initializing MPI. The callback function must be
 *             installed prior to VisItDetectInput.
 *
 *             Sample callback function:
 *             static int visit_broadcast_string_callback(char *str, int len,
 *                                                        int sender, void *ptr)
 *             {
 *                 simdata_t *sim = (simdata_t *)ptr;
 *                 return MPI_Bcast(str, len, MPI_CHAR, sender, sim->communicator);
 *             }
 *
 * ****************************************************************************/
void  VisItSetBroadcastStringFunction2(int (*cb)(char *, int, int, void *), void *);

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
 * Returns:   TRUE, FALSE
 *
 * Note:      All processors must call this function and they must call it
 *            before VisItInitializeSocketAndDumpSimFile and VisItDetectInput.
 *
 *            If is recommended that parallel simulations call these functions
 *            beforehand so this function will be able to use a collective
 *            broadcast internally to speed up sending the environment to
 *            other processors: VisItSetParallel, VisItSetParallelRank,
 *            VisItSetBroadcastStringFunction.
 *
 * ****************************************************************************/
int   VisItSetupEnvironment(void);

/******************************************************************************
 * Function: VisItSetupEnvironment2
 *
 * Purpose:
 *   Sets up the environment so VisIt can be loaded into libsim when the
 *   VisIt client wants to connect. This function does the same thing as
 *   VisItSetupEnvironment but it lets you pass in the environment string
 *   discovered by VisItGetEnvironment.
 *
 *   You would use this version when your MPI does not like to let your
 *   application spawn processes to discover the environment.
 *
 * Arguments: env : The environment returned by
 *
 * Returns:   TRUE, FALSE
 *
 * Note:      All processors must call this function and they must call it
 *            before VisItInitializeSocketAndDumpSimFile and VisItDetectInput.
 *
 *            Non-rank 0 processors may pass NULL for the environment if they
 *            wish to inherit the environment read by processor 0.
 *
 *            If is recommended that parallel simulations call these functions
 *            beforehand so this function will be able to use a collective
 *            broadcast internally to speed up sending the environment to
 *            other processors: VisItSetParallel, VisItSetParallelRank,
 *            VisItSetBroadcastStringFunction.
 *
 *            Non-rank 0 processors may pass NULL for the environment if they
 *            wish to inherit the environment read by processor 0. (Only if
 *            you have set up collective broadcast). Otherwise, passing NULL
 *            will cause an implicit call to VisItGetEnvironment.
 *
 * ****************************************************************************/
int   VisItSetupEnvironment2(char *env);

/******************************************************************************
 * Function: VisItGetEnvironment
 *
 * Purpose:
 *   Discover the environment needed so VisIt can be loaded into libsim when
 *   the VisIt client wants to connect.
 *
 * Arguments: None
 *
 * Returns:   A user-owned character buffer containing the environment strings.
 *
 * ****************************************************************************/
char *VisItGetEnvironment(void);

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
int   VisItDetectInputWithTimeout(int blocking, int timeout_usec, int consoledesc);

#ifndef _WIN32
/*******************************************************************************
* Function: VisItGetSockets
*
* Purpose:
*   VisItDetectInput detects input from VisIt's listen socket, a client socket,
*   and optionally, a console file descriptor. This function lets you obtain the
*   file descriptors to which VisItDetectInput would have listened, allowing you
*   to pass the socket descriptors to your own listening routines should you
*   need to do so.
*
* Arguments:
*   lSocket : Pass the address of an integer that will contain the listen socket
*             file descriptor. When the function returns, the address will contain
*             the listen file descriptor or -1 if you should not ignore the
*             result.
*
*   cSocket : Pass the address of an integer that will contain the client socket
*             file descriptor. When the function returns, the address will contain
*             the client file descriptor or -1 if you should not ignore the
*             result.
*
* Returns:    VISIT_OKAY on success, VISIT_ERROR on failure.
*
* Note:       This function is not available on Windows because input is handled
*             differently than on UNIX and VisItDetectInput should not be
*             circumvented. In any case, only use this function if you REALLY
*             know what you are doing. You should prefer using VisItDetectInput
*             whenever possible.
*
*******************************************************************************/
int VisItGetSockets(int *lSocket, int *cSocket);
#endif

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
 *
 * ****************************************************************************/
int   VisItAttemptToCompleteConnection(void);

/******************************************************************************
 * Function: VisItReadConsole
 *
 * Purpose:
 *   Read characters from the console.
 *
 * Arguments:
 *   maxlen : The size of the buffer.
 *   buffer : the buffer into which the values will be read.
 *
 * Returns:   VISIT_OKAY on success; VISIT_ERROR on failure
 *
 * Note:      This function should be called when VisItDetectInput returns 3.
 *            Only the root processor should call this function.
 *
 * ****************************************************************************/
int VisItReadConsole(int maxlen, char *buffer);

/******************************************************************************
 * Function: VisItSetSlaveProcessCallback
 *
 * Purpose:
 *   Set the callback function used to inform slave processes that they should
 *   call VisItfor ProcessEngineCommand. The provided callback function is used
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
void  VisItSetSlaveProcessCallback(void(*spcb)(void));

/******************************************************************************
 * Function: VisItSetSlaveProcessCallback2
 *
 * Purpose:
 *   Set the callback function used to inform slave processes that they should
 *   call VisItProcessEngineCommand. The provided callback function is used
 *   internally in libsim
 *
 *
 * Arguments:  A pointer to a function with prototype: void func(void *);
 *             A pointer to some callback function data.
 *
 * Returns:    None
 *
 * Note:       The slave process callback is required for a parallel simulation.
 *             This function should be called when VisItAttemptToCompleteConnection
 *             returns successfully.
 *
 *             MPI simulations may define the callback like this:
 *                 void slave_process_callback(void *ptr)
 *                 {
 *                     int command = 0;
 *                     simdata_t *sim = (simdata_t *)ptr;
 *                     MPI_BCast(&command, 1, MPI_INT, 0, sim->communicator);
 *                 }
 *
 * ****************************************************************************/
void  VisItSetSlaveProcessCallback2(void(*cb)(void *), void *);

/******************************************************************************
 * Function: VisItSetCommandCallback
 *
 * Purpose:
 *   Set the callback for processing control commands (these are set up in
 *   the simulation's metadata).
 *
 * Arguments: A pointer to a function with prototype:
 *            void func(const char*,const char*,void*);
 *
 *            The callback function arguments are:
 *            cmd   : The name of the command being called.
 *            sdata : string argument (reserved)
 *            void* : User callback data.
 *
 * Returns:    None
 *
 * Note:       This function must be called on all processors to install the
 *             callback function. This function should be called when
 *             VisItAttemptToCompleteConnection returns successfully.
 *
 * ****************************************************************************/
void  VisItSetCommandCallback(void(*cb)(const char*,const char*,void*), void *cbdata1);

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
 * Note:      This function has no effect when called by non-master
 *            processes. When VisIt is connected to the simulation, calling
 *            this function on the master process will send commands to process
 *            to VisIt's viewer. The sim will then process VisIt commands in
 *            a synchronization loop if synchronization is enabled. In that case,
 *            ALL processors must call this function. During the synchronization
 *            loop, VisIt commands will be handled via the normal simulation
 *            callback functions. Once synchronization is complete, the function
 *            returns. If synchronization is not enabled then this function
 *            returns immediately and relies on the simulation's main loop
 *            for processing VisIt commands that result from the update.
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
 *            This function has no effect when called by non-master
 *            processes. When VisIt is connected to the simulation, calling
 *            this function on the master process will send commands to process
 *            to VisIt's viewer. The sim will then process VisIt commands in
 *            a synchronization loop if synchronization is enabled. In that case,
 *            ALL processors must call this function. During the synchronization
 *            loop, VisIt commands will be handled via the normal simulation
 *            callback functions. Once synchronization is complete, the function
 *            returns. If synchronization is not enabled then this function
 *            returns immediately and relies on the simulation's main loop
 *            for processing VisIt commands that result from the command.
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
 * Function: VisItIsConnected
 *
 * Purpose:
 *   This function returns 1 if VisIt is connected; 0 otherwise.
 *
 * Arguments: None
 *
 * Returns:   1 if VisIt is connected; 0 otherwise.
 *
 * Note:
 *
 * ****************************************************************************/
int   VisItIsConnected(void);

/******************************************************************************
 * Function: VisItIsRuntimeLoaded
 *
 * Purpose:
 *   This function returns 1 if the VisIt runtime is loaded; 0 otherwise.
 *
 * Arguments: None
 *
 * Returns:   1 if VisIt runtime is loaded; 0 otherwise.
 *
 * Note:
 *
 * ****************************************************************************/
int   VisItIsRuntimeLoaded(void);

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

/******************************************************************************
 * Function: VisItSynchronize
 *
 * Purpose:
 *   This function blocks the simulation, creating a new event loop to handle
 *   requests from VisIt until VisIt returns a synchronize tag.
 *
 * Arguments: None
 *
 * Returns:   None
 *
 * Note:      Do not call recursively!
 *
 * ****************************************************************************/
int   VisItSynchronize(void);

/******************************************************************************
 * Function: VisItEnableSynchronize
 *
 * Purpose:
 *   This function sets whether synchronization is enabled for functions such as
 *   VisItUpdatePlots and VisItExecuteCommand.
 *
 * Arguments:
 *   sync : Non-zero values enable synchronization, zero disables it.
 *
 * Returns:   None
 *
 * Note:
 *
 * ****************************************************************************/
void  VisItEnableSynchronize(int);

/******************************************************************************
 * Function: VisItDebug
 *
 * Purpose:
 *   These functions let you write a printf-style output to VisIt's debugging
 *   logs.
 *
 * Arguments:
 *   format : A format string.
 *
 * Returns:   None
 *
 * Note:      These functions have no effect until VisIt has connected.
 *
 * ****************************************************************************/
void  VisItDebug1(const char *format, ...);
void  VisItDebug2(const char *format, ...);
void  VisItDebug3(const char *format, ...);
void  VisItDebug4(const char *format, ...);
void  VisItDebug5(const char *format, ...);

/******************************************************************************
 * Function: VisItOpenTraceFile
 *
 * Purpose:
 *   This function opens a trace file that contains a trace of all of SimV2's
 *   function calls. The trace file is meant to aid in debugging.
 *
 * Arguments:
 *   filename : The name of the trace file.
 *
 * Returns:   None
 *
 * Note:
 *
 * ****************************************************************************/

void  VisItOpenTraceFile(const char *filename);

/******************************************************************************
 * Function: VisItCloseTraceFile
 *
 * Purpose:
 *   This function closes the trace file.
 *
 * Arguments: None
 *
 * Returns:   None
 *
 * Note:
 *
 * ****************************************************************************/
void  VisItCloseTraceFile(void);

/******************************************************************************
 * Function: VisItSaveWindow
 *
 * Purpose:
 *   This function saves the last plot to an image file.
 *
 * Arguments:
 *   filename : The name of the image file.
 *   width    : The width of the saved image.
 *   height   : The height of the saved image.
 *   format   : The format in which to save the image. (e.g. VISIT_IMAGEFORMAT_JPEG)
 *
 * Returns:   VISIT_OKAY on success; otherwise VISIT_ERROR
 *
 * Note:      A plot must have already been created. Call this function on all
 *            processors.
 *
 * ****************************************************************************/
int VisItSaveWindow(const char *filename, int width, int height, int format);

/******************************************************************************
 * Function: VisItBeginCinema
 *
 * Purpose:
 *   This function saves parameters for a new Cinema database. This makes it
 *   so we don't pass contradictory parameters later on and we also allocate
 *   some internal storage for this Cinema database.
 *
 * Arguments:
 *   h        : A pointer to the visit_handle that we'll allocate.
 *   file_cdb : The name of the .cdb directory that contains the Cinema data.
 *   dbspec   : The database specification VISIT_CINEMA_SPEC_A, C, D.
 *   composite : 0 for images, 1 for composite images.
 *   imgformat: The format in which to save the image. (e.g. VISIT_IMAGEFORMAT_JPEG)
 *   width    : The width of the saved image.
 *   height   : The height of the saved image.
 *   cameratype: The camera type. VISIT_CINEMA_CAMERA_STATIC, VISIT_CINEMA_CAMERA_PHI_THETA.
 *   nphi      : The number of divisions in phi (for VISIT_CINEMA_CAMERA_PHI_THETA).
 *   ntheta    : The number of divisions in theta (for VISIT_CINEMA_CAMERA_PHI_THETA).
 *   varnames  : A namelist object containing variable names. This parameter is
 *               optional. If you do not want to specify additional variable
 *               names, pass VISIT_INVALID_HANDLE.
 *
 * Returns:   VISIT_OKAY on success; otherwise VISIT_ERROR
 *
 * Note:      A plot must have already been created. Call this function on all
 *            processors.
 *
 * ****************************************************************************/
int VisItBeginCinema(visit_handle *h,
                     const char *file_cdb, int dbspec, int composite,
                     int imgformat, int width, int height,
                     int cameratype, int nphi, int ntheta,
                     visit_handle varnames);

/******************************************************************************
 * Function: VisItSaveCinema
 *
 * Purpose:
 *   This function saves the current plots to the file_cdb Cinema database.
 *
 * Arguments:
 *   h     : The handle returned from VisItBeginCinema.
 *   time  : The time value for the current time step we're saving.
 *
 * Returns:   VISIT_OKAY on success; otherwise VISIT_ERROR
 *
 * Note:      A plot must have already been created. Call this function on all
 *            processors.
 *
 * ****************************************************************************/
int VisItSaveCinema(visit_handle h,  double time);

/******************************************************************************
 * Function: VisItEndCinema
 *
 * Purpose:
 *   This function ends a Cinema database.
 *
 * Arguments:
 *   h : The handle returned from VisItBeginCinema.
 *
 * Returns:   VISIT_OKAY on success; otherwise VISIT_ERROR
 *
 * Note:
 *
 * ****************************************************************************/
int VisItEndCinema(visit_handle h);

/******************************************************************************
 * Function: VisItSetMPICommunicator
 *
 * Purpose:
 *   This function sets the communicator that VisIt should use for parallel
 *   operations. Note that you may want to temporarily install a communicator
 *   for vis operations and then revert to the default communicator for when
 *   VisIt needs to communicate commands to other processes.
 *
 * Arguments:
 *   mpicom : A pointer to the communicator that we should use.
 *
 * Returns:   VISIT_OKAY on success; otherwise VISIT_ERROR
 *
 * Note:      Passing NULL lets VisIt revert to its copy of MPI_COMM_WORLD.
 *
 * ****************************************************************************/
int VisItSetMPICommunicator(void *mpicom);

/******************************************************************************
 * Function: VisItSetMPICommunicator_f
 *
 * Purpose:
 *   This function sets the communicator that VisIt should use for parallel
 *   operations from a Fortran handle. Note that you may want to temporarily
 *   install a communicator for vis operations and then revert to the default
 *   communicator for when VisIt needs to communicate commands to other
 *   processes.
 *
 * Arguments:
 *   mpicom : A pointer to the FORTRAN communicator that we should use.
 *
 * Returns:   VISIT_OKAY on success; otherwise VISIT_ERROR
 *
 * Note:      use VisItSetMPICommunicator to revert to its copy of MPI_COMM_WORLD.
 *
 * ****************************************************************************/
int VisItSetMPICommunicator_f(int *mpicom);

/******************************************************************************
 * Function: VisItInitializeRuntime
 *
 * Purpose:
 *   Initialize the VisIt runtime without the client being connected.
 *
 * Arguments:
 *
 *
 * Returns:   VISIT_OKAY on success; otherwise VISIT_ERROR
 *
 * Note:      Call this function on all processors.
 *
 * ****************************************************************************/
int VisItInitializeRuntime(void);

/******************************************************************************
 * Function: VisItGetMemory
 *
 * Purpose:
 *   Measure memory usage for the current processor.
 *
 * Arguments:
 *   m_size : The number of MiB in use.
 *   m_rss  : The resident set size in MiB.
 *
 * Returns:   VISIT_OKAY on success; otherwise VISIT_ERROR
 *
 * Note:      This function can be called in individual processors.
 *
 * ****************************************************************************/
int VisItGetMemory(double *m_size, double *m_rss);

/******************************************************************************
 * Function: VisItExportDatabase
 *
 * Purpose:
 *   Export the active plots to database files.
 *
 * Arguments:
 *   filename : The name of the file to save. If it does not have a path then
 *              the current working directory will be where files are saved.
 *              File format writers are free to append their own file extensions
 *              to this name.
 *   format   : The name of a database plugin for the export. This can be either
 *              the plugin name or id (e.g. "Silo" or "Silo_1.0").
 *   variables: A name list containing the names of the variables to be exported.
 *              If an empty list or VISIT_INVALID_HANDLE are passed then the
 *              "default" variables will be exported.
 *
 * Returns:   VISIT_OKAY on success; otherwise VISIT_ERROR
 *
 * Note:      The active plots in the plot list are exported just like in
 *            typical export usage from VisIt.
 *
 *            Call this function on all processors.
 *
 * ****************************************************************************/
int VisItExportDatabase(const char *filename, const char *format,
                        visit_handle variables);

/******************************************************************************
 * Function: VisItExportDatabaseWithOptions
 *
 * Purpose:
 *   Export the active plots to database files.
 *
 * Arguments:
 *   filename : The name of the file to save. If it does not have a path then
 *              the current working directory will be where files are saved.
 *              File format writers are free to append their own file extensions
 *              to this name.
 *   format   : The name of a database plugin for the export. This can be either
 *              the plugin name or id (e.g. "Silo" or "Silo_1.0").
 *   variables: A name list containing the names of the variables to be exported.
 *              If an empty list or VISIT_INVALID_HANDLE are passed then the
 *              "default" variables will be exported.
 *   options  : A handle to an optional optionlist object (contaisn key/value
 *              pairs) or VISIT_INVALID_HANDLE if no options are to be passed.
 *
 * Returns:   VISIT_OKAY on success; otherwise VISIT_ERROR
 *
 * Note:      The active plots in the plot list are exported just like in
 *            typical export usage from VisIt.
 *
 *            Call this function on all processors.
 *
 * ****************************************************************************/
int VisItExportDatabaseWithOptions(const char *filename, const char *format,
                                   visit_handle variables, visit_handle options);

/******************************************************************************
 * Function: VisItRestoreSession
 *
 * Purpose:
 *   Restores a session file to set up the visualization that will be saved
 *   from operations such as VisItSaveWindow and VisItExportDatabase.
 *
 * Arguments:
 *   filename : The path to the session file.
 *
 * Returns:   VISIT_OKAY on success; otherwise VISIT_ERROR
 *
 * Note:      Call this function on all processors.
 *
 * ****************************************************************************/
int VisItRestoreSession(const char *filename);


/******************************************************************************
*******************************************************************************
***
***       Functions to install data access callback functions
***
*******************************************************************************
******************************************************************************/

/******************************************************************************
 * Function: VisItSetActivateTimestep
 *
 * Purpose:
 *   This function installs a callback function that does collective
 *   communication before other data access callback functions are called.
 *
 * Arguments:
 *   cb     : The callback function
 *   cbdata1 : A pointer to data to pass to the callback function.
 *
 * Returns:   VISIT_OKAY on success; otherwise VISIT_ERROR
 *
 * Note:      This function has no effect unless called after a successful
 *            call to VisItAttemptToCompleteConnection
 *
 * ****************************************************************************/
int VisItSetActivateTimestep(int (*cb)(void *), void *cbdata1);

/******************************************************************************
 * Function: VisItSetGetMetaData
 *
 * Purpose:
 *   This function installs a callback function that gets simulation metadata.
 *
 * Arguments:
 *   cb     : The callback function
 *   cbdata1 : A pointer to data to pass to the callback function.
 *
 *            The callback function arguments are:
 *               void* : User-supplied callback data.
 *
 * Returns:   VISIT_OKAY on success; otherwise VISIT_ERROR
 *
 * Note:      This function has no effect unless called after a successful
 *            call to VisItAttemptToCompleteConnection
 *
 * ****************************************************************************/
int VisItSetGetMetaData(visit_handle (*cb)(void *), void *cbdata1);

/******************************************************************************
 * Function: VisItSetGetMesh
 *
 * Purpose:
 *   This function installs a callback function that gets a mesh.
 *
 * Arguments:
 *   cb     : The callback function
 *   cbdata1 : A pointer to data to pass to the callback function.
 *
 *            The callback function arguments are:
 *               int         : The domain number
 *               const char* : name of mesh
 *               void*       : User-supplied callback data.
 *
 * Returns:   VISIT_OKAY on success; otherwise VISIT_ERROR
 *
 * Note:      This function has no effect unless called after a successful
 *            call to VisItAttemptToCompleteConnection
 *
 * ****************************************************************************/
int VisItSetGetMesh(visit_handle (*cb)(int, const char *, void *), void *cbdata1);

/******************************************************************************
 * Function: VisItSetGetMaterial
 *
 * Purpose:
 *   This function installs a callback function that gets a material.
 *
 * Arguments:
 *   cb     : The callback function
 *   cbdata1 : A pointer to data to pass to the callback function.
 *
 *            The callback function arguments are:
 *               int         : The domain number
 *               const char* : name of material
 *               void*       : User-supplied callback data.
 *
 * Returns:   VISIT_OKAY on success; otherwise VISIT_ERROR
 *
 * Note:      This function has no effect unless called after a successful
 *            call to VisItAttemptToCompleteConnection
 *
 * ****************************************************************************/
int VisItSetGetMaterial(visit_handle (*cb)(int, const char *, void *), void *cbdata1);

/******************************************************************************
 * Function: VisItSetGetSpecies
 *
 * Purpose:
 *   This function installs a callback function that gets species data.
 *
 * Arguments:
 *   cb     : The callback function
 *   cbdata1 : A pointer to data to pass to the callback function.
 *
 *            The callback function arguments are:
 *               int         : The domain number
 *               const char* : name of species
 *               void*       : User-supplied callback data.
 *
 * Returns:   VISIT_OKAY on success; otherwise VISIT_ERROR
 *
 * Note:      This function has no effect unless called after a successful
 *            call to VisItAttemptToCompleteConnection
 *
 * ****************************************************************************/
int VisItSetGetSpecies(visit_handle (*cb)(int, const char *, void *), void *cbdata1);

/******************************************************************************
 * Function: VisItSetGetVariable
 *
 * Purpose:
 *   This function installs a callback function that gets variable data.
 *
 * Arguments:
 *   cb     : The callback function
 *   cbdata1 : A pointer to data to pass to the callback function.
 *
 *            The callback function arguments are:
 *               int         : The domain number
 *               const char* : name of variable
 *               void*       : User-supplied callback data.
 *
 * Returns:   VISIT_OKAY on success; otherwise VISIT_ERROR
 *
 * Note:      This function has no effect unless called after a successful
 *            call to VisItAttemptToCompleteConnection
 *
 * ****************************************************************************/
int VisItSetGetVariable(visit_handle (*cb)(int, const char *, void *), void *cbdata1);

/******************************************************************************
 * Function: VisItSetGetMixedVariable
 *
 * Purpose:
 *   This function installs a callback function that gets mixed variable data.
 *
 * Arguments:
 *   cb     : The callback function
 *   cbdata1 : A pointer to data to pass to the callback function.
 *
 *            The callback function arguments are:
 *               int         : The domain number
 *               const char* : name of mixed variable
 *               void*       : User-supplied callback data.

 * Returns:   VISIT_OKAY on success; otherwise VISIT_ERROR
 *
 * Note:      This function has no effect unless called after a successful
 *            call to VisItAttemptToCompleteConnection
 *
 * ****************************************************************************/
int VisItSetGetMixedVariable(visit_handle (*cb)(int, const char *, void *), void *cbdata1);

/******************************************************************************
 * Function: VisItSetGetCurve
 *
 * Purpose:
 *   This function installs a callback function that gets curve data.
 *
 * Arguments:
 *   cb     : The callback function
 *   cbdata1 : A pointer to data to pass to the callback function.
 *
 *            The callback function arguments are:
 *               const char* : name of curve
 *               void*       : User-supplied callback data.
 * Returns:   VISIT_OKAY on success; otherwise VISIT_ERROR
 *
 * Note:      This function has no effect unless called after a successful
 *            call to VisItAttemptToCompleteConnection
 *
 * ****************************************************************************/
int VisItSetGetCurve(visit_handle (*cb)(const char *, void *), void *cbdata1);

/******************************************************************************
 * Function: VisItSetGetDomainList
 *
 * Purpose:
 *   This function installs a callback function that gets a domain list.
 *
 * Arguments:
 *   cb     : The callback function
 *   cbdata1 : A pointer to data to pass to the callback function.
 *
 *            The callback function arguments are:
 *               const char* : name of mesh for domain list (reserved)
 *               void*       : User-supplied callback data.
 *
 * Returns:   VISIT_OKAY on success; otherwise VISIT_ERROR
 *
 * Note:      This function has no effect unless called after a successful
 *            call to VisItAttemptToCompleteConnection
 *
 *            The domain list callback function must be implemented in order
 *            for a parallel simulation to work.
 *
 * ****************************************************************************/
int VisItSetGetDomainList(visit_handle (*cb)(const char *, void *), void *cbdata1);

/******************************************************************************
 * Function: VisItSetGetDomainBoundaries
 *
 * Purpose:
 *   This function installs a callback function that gets domain boundaries.
 *   Implementing domain boundaries callback function allows VisIt to create
 *   ghost zones between domains.
 *
 * Arguments:
 *   cb     : The callback function
 *   cbdata1 : A pointer to data to pass to the callback function.
 *
 *            The callback function arguments are:
 *               int         : The domain number
 *               const char* : name of mesh for domain boundaries
 *               void*       : User-supplied callback data.
 *
 * Returns:   VISIT_OKAY on success; otherwise VISIT_ERROR
 *
 * Note:      This function has no effect unless called after a successful
 *            call to VisItAttemptToCompleteConnection
 *
 * ****************************************************************************/
int VisItSetGetDomainBoundaries(visit_handle (*cb)(const char *, void *), void *cbdata1);

/******************************************************************************
 * Function: VisItSetGetDomainNesting
 *
 * Purpose:
 *   This function installs a callback function that returns a domain nesting
 *   object, which is used to tell VisIt how AMR patches are nested.
 *
 * Arguments:
 *   cb     : The callback function
 *   cbdata1 : A pointer to data to pass to the callback function.
 *
 *            The callback function arguments are:
 *               int         : The domain number
 *               const char* : name of mesh for domain nesting
 *               void*       : User-supplied callback data.
 *
 * Returns:   VISIT_OKAY on success; otherwise VISIT_ERROR
 *
 * Note:      This function has no effect unless called after a successful
 *            call to VisItAttemptToCompleteConnection
 *
 * ****************************************************************************/
int VisItSetGetDomainNesting(visit_handle (*cb)(const char *, void *), void *cbdata1);

/* Functions that install data writer callback functions */

/******************************************************************************
 * Function: VisItSetWriteBegin
 *
 * Purpose:
 *   This function installs a callback function that can be used to start
 *   exporting from VisIt back to the simulation.
 *
 * Arguments:
 *   cb     : The callback function
 *   cbdata1 : A pointer to data to pass to the callback function.
 *
 *            The callback function arguments are:
 *               const char* : name of mesh
 *               void*       : User-supplied callback data.
 *
 * Returns:   VISIT_OKAY on success; otherwise VISIT_ERROR
 *
 * Note:      This function has no effect unless called after a successful
 *            call to VisItAttemptToCompleteConnection
 *
 * ****************************************************************************/
int VisItSetWriteBegin(int (*cb)(const char *, void *), void *cbdata1);

/******************************************************************************
 * Function: VisItSetWriteEnd
 *
 * Purpose:
 *   This function installs a callback function that can be called at the
 *   end of exporting from VisIt to the simulation.
 *
 * Arguments:
 *   cb     : The callback function
 *   cbdata1 : A pointer to data to pass to the callback function.
 *
 *            The callback function arguments are:
 *               const char* : name of mesh
 *               void*       : User-supplied callback data.
 *
 * Returns:   VISIT_OKAY on success; otherwise VISIT_ERROR
 *
 * Note:      This function has no effect unless called after a successful
 *            call to VisItAttemptToCompleteConnection
 *
 * ****************************************************************************/
int VisItSetWriteEnd(int (*cb)(const char *, void *), void *cbdata1);

/******************************************************************************
 * Function: VisItSetWriteMesh
 *
 * Purpose:
 *   This function installs a callback function that is called to export a
 *   chunk of mesh data from VisIt to the simulation.
 *
 * Arguments:
 *   cb     : The callback function
 *   cbdata1 : A pointer to data to pass to the callback function.
 *
 *            The callback function arguments are:
 *               const char*  : name of mesh
 *               int          : The domain number
 *               int          : The mesh type
 *               visit_handle : A handle to a mesh data object.
 *               visit_handle : A handle to mesh metadata
 *               void*        : User-supplied callback data.
 *
 * Returns:   VISIT_OKAY on success; otherwise VISIT_ERROR
 *
 * Note:      This function has no effect unless called after a successful
 *            call to VisItAttemptToCompleteConnection
 *
 * ****************************************************************************/
int VisItSetWriteMesh(int (*cb)(const char *, int, int, visit_handle, visit_handle, void *), void *cbdata1);

/******************************************************************************
 * Function: VisItSetWriteVariable
 *
 * Purpose:
 *   This function installs a callback function that is called to export a chunk
 *   of variable data from VisIt to the simulation.
 *
 * Arguments:
 *   cb     : The callback function
 *   cbdata1 : A pointer to data to pass to the callback function.
 *
 *            The callback function arguments are:
 *               const char*  : name of mesh
 *               const char*  : name of variable
 *               int          : The domain number
 *               visit_handle : A handle to variable data
 *               visit_handle : A handle to variable metadata.
 *               void*        : User-supplied callback data.
 *
 * Returns:   VISIT_OKAY on success; otherwise VISIT_ERROR
 *
 * Note:      This function has no effect unless called after a successful
 *            call to VisItAttemptToCompleteConnection
 *
 * ****************************************************************************/
int VisItSetWriteVariable(int (*cb)(const char *, const char *, int, visit_handle, visit_handle, void *), void *cbdata1);


/* UI-related functions */
int VisItUI_clicked(const char *name, void (*cb)(void*), void *cbdata2);
int VisItUI_stateChanged(const char *name, void (*cb)(int,void*), void *cbdata2);
int VisItUI_valueChanged(const char *name, void (*cb)(int,void*), void *cbdata2);
int VisItUI_textChanged(const char *name, void (*cb)(char *,void*), void *cbdata2);
int VisItUI_cellChanged(const char *name, void (*cb)(char *,void*), void *cbdata2);

int VisItUI_setValueI(const char *name, int value, int enabled);
int VisItUI_setValueD(const char *name, double value, int enabled);
int VisItUI_setValueV(const char *name, double x, double y, double z, int enabled);
int VisItUI_setValueS(const char *name, const char *value, int enabled);


int VisItUI_setListValueI(const char *name,
                          int row, int value, int editable, int enabled);
int VisItUI_setListValueD(const char *name,
                          int row, double value, int editable, int enabled);
int VisItUI_setListValueV(const char *name,
                          int row, double x, double y, double z, int editable, int enabled);
int VisItUI_setListValueS(const char *name,
                          int row, const char *value, int editable, int enabled);


int VisItUI_setTableValueI(const char *name,
                           int row, int col, int value, int enabled);
int VisItUI_setTableValueD(const char *name,
                           int row, int col, double value, int enabled);
int VisItUI_setTableValueV(const char *name,
                           int row, int col, double x, double y, double z, int enabled);
int VisItUI_setTableValueS(const char *name,
                           int row, int col, const char *value, int enabled);


int VisItUI_addStripChartPoint(int chart, int curve, int cycle, double value);

int VisItUI_addStripChartPoints(int chart, int curve, int npts, int *cycles, double *values);

/* Include some experimental plotting functions */
#include <VisItControlInterface_V2_plotting.h>

#ifdef __cplusplus
}
#endif

#endif
