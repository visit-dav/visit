//
// Define some boiler plate macros that wrap blocking RPCs.
//
// EngineProxy Macro Modifications:
//    Jeremy Meredith, Wed Mar 17 15:09:07 PST 2004
//    Inserted "false" before numRestarts so that it wouldn't take numRestarts
//    as the value for skipChooser. 
//
//    Jeremy Meredith, Mon Mar 22 17:15:44 PST 2004
//    Added a setting of retry=false when creating the engine in the first
//    section of code, because if the user cancels the launch of the engine,
//    it could otherwise get into an infinite loop.
//
//    Jeremy Meredith, Fri Apr  2 14:29:25 PST 2004
//    Made restartArguments be saved on a per-host (per-enginekey) basis.
//
//    Brad Whitlock, Wed Aug 4 17:33:08 PST 2004
//    Changed EngineMap.
//
//    Brad Whitlock, Wed Feb 23 16:47:36 PST 2005
//    Added ENGINE_PROXY_RPC_BEGIN_NOSTART that does not try to launch
//    a compute engine before checking for its existence.

#define ENGINE_PROXY_RPC_BEGIN_NOSTART(rpcname) \
    bool retval = false; \
    bool retry = false; \
    int  numAttempts = 0; \
    do \
    { \
        if (EngineExists(ek)) \
        { \
            TRY \
            { \
                EngineProxy *engine = engines[ek].proxy; \
                debug3 << "Calling " << rpcname << " RPC on " \
                       << ek.HostName().c_str() << "'s engine." << endl;


#define ENGINE_PROXY_RPC_BEGIN(rpcname)  \
    bool retval = false; \
    bool retry = false; \
    int  numAttempts = 0; \
    do \
    { \
        if (!EngineExists(ek)) \
        { \
            if (ek.SimName() == "") \
            { \
                debug1 << "****\n**** Trying to execute the " << rpcname \
                       << " RPC before an engine was started" << endl \
                       << "**** on " << ek.HostName().c_str() \
                       << ". Starting an engine on " \
                       << ek.HostName().c_str() << ".\n****" << endl; \
                CreateEngine(ek, restartArguments[ek], false, numRestarts); \
                retry = false; \
            } \
            else \
            { \
                LaunchMessage(ek); \
                retry = false; \
            } \
        } \
        if (EngineExists(ek)) \
        { \
            TRY \
            { \
                EngineProxy *engine = engines[ek].proxy; \
                debug3 << "Calling " << rpcname << " RPC on " \
                       << ek.HostName().c_str() << "'s engine." << endl;


#define ENGINE_PROXY_RPC_END  \
                retval = true; \
                retry = false; \
            } \
            CATCH(LostConnectionException) \
            { \
                if (ek.SimName() != "") \
                { \
                    LaunchMessage(ek); \
                    RemoveFailedEngine(ek); \
                    retry = false; \
                } \
                else if (numAttempts < numRestarts) \
                { \
                   retry = true; \
                   RemoveFailedEngine(ek); \
                   LaunchMessage(ek); \
                   CreateEngine(ek, restartArguments[ek],false,numRestarts); \
                   ++numAttempts; \
                } \
                else \
                { \
                   retry = false; \
                   retval = false; \
                } \
            } \
            CATCH(VisItException) \
            { \
                retry = false; \
                retval = false; \
            } \
            ENDTRY \
        } \
    } while(retry && numAttempts < numRestarts); \
    if(!retval || (retry && retval)) \
        UpdateEngineList(); \
    return retval;


#define ENGINE_PROXY_RPC_END_NORESTART_RETHROW  \
                retval = true; \
                retry = false; \
            } \
            CATCH(LostConnectionException) \
            { \
                retry = false; \
                retval = false; \
                RemoveFailedEngine(ek); \
            } \
            CATCH(VisItException) \
            { \
                retry = false; \
                retval = false; \
                RETHROW; \
            } \
            ENDTRY \
        } \
    } while(retry && numAttempts < numRestarts); \
    if(!retval || (retry && retval)) \
        UpdateEngineList(); \
    return retval;


#define ENGINE_PROXY_RPC_END_NORESTART_RETHROW2  \
                retval = true; \
                retry = false; \
            } \
            CATCH(LostConnectionException) \
            { \
                retry = false; \
                retval = false; \
                RemoveFailedEngine(ek); \
                RETHROW; \
            } \
            CATCH(VisItException) \
            { \
                retry = false; \
                retval = false; \
                RETHROW; \
            } \
            ENDTRY \
        } \
        else \
        { \
            retry = false; \
            retval = false; \
            EXCEPTION0(NoEngineException); \
        } \
    } while(retry && numAttempts < numRestarts); \
    if(!retval || (retry && retval)) \
        UpdateEngineList(); \
    return retval;


#define ENGINE_PROXY_RPC_END_NORESTART  \
                retval = true; \
                retry = false; \
            } \
            CATCH(LostConnectionException) \
            { \
                retry = false; \
                retval = false; \
                RemoveFailedEngine(ek); \
            } \
            CATCH(VisItException) \
            { \
                retry = false; \
                retval = false; \
            } \
            ENDTRY \
        } \
    } while(retry && numAttempts < numRestarts); \
    if(!retval || (retry && retval)) \
        UpdateEngineList(); \
    return retval;
