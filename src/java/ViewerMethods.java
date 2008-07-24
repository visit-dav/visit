// ****************************************************************************
//
// Copyright (c) 2000 - 2008, Lawrence Livermore National Security, LLC
// Produced at the Lawrence Livermore National Laboratory
// LLNL-CODE-400142
// All rights reserved.
//
// This file is  part of VisIt. For  details, see https://visit.llnl.gov/.  The
// full copyright notice is contained in the file COPYRIGHT located at the root
// of the VisIt distribution or at http://www.llnl.gov/visit/copyright.html.
//
// Redistribution  and  use  in  source  and  binary  forms,  with  or  without
// modification, are permitted provided that the following conditions are met:
//
//  - Redistributions of  source code must  retain the above  copyright notice,
//    this list of conditions and the disclaimer below.
//  - Redistributions in binary form must reproduce the above copyright notice,
//    this  list of  conditions  and  the  disclaimer (as noted below)  in  the
//    documentation and/or other materials provided with the distribution.
//  - Neither the name of  the LLNS/LLNL nor the names of  its contributors may
//    be used to endorse or promote products derived from this software without
//    specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT  HOLDERS AND CONTRIBUTORS "AS IS"
// AND ANY EXPRESS OR  IMPLIED WARRANTIES, INCLUDING,  BUT NOT  LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND  FITNESS FOR A PARTICULAR  PURPOSE
// ARE  DISCLAIMED. IN  NO EVENT  SHALL LAWRENCE  LIVERMORE NATIONAL  SECURITY,
// LLC, THE  U.S.  DEPARTMENT OF  ENERGY  OR  CONTRIBUTORS BE  LIABLE  FOR  ANY
// DIRECT,  INDIRECT,   INCIDENTAL,   SPECIAL,   EXEMPLARY,  OR   CONSEQUENTIAL
// DAMAGES (INCLUDING, BUT NOT  LIMITED TO, PROCUREMENT OF  SUBSTITUTE GOODS OR
// SERVICES; LOSS OF  USE, DATA, OR PROFITS; OR  BUSINESS INTERRUPTION) HOWEVER
// CAUSED  AND  ON  ANY  THEORY  OF  LIABILITY,  WHETHER  IN  CONTRACT,  STRICT
// LIABILITY, OR TORT  (INCLUDING NEGLIGENCE OR OTHERWISE)  ARISING IN ANY  WAY
// OUT OF THE  USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH
// DAMAGE.
//
// ****************************************************************************
package llnl.visit;

import java.util.Vector;

/**
 * ViewerMethods contains all of the methods that are mapped into VieweRPC
 * requests, which control VisIt's viewer. The order of methods matches the
 * order of methods in ViewerMethods.C, the C++ implementation so it will be
 * easier to reconcile the two implementations during future maintenance.
 *
 * @author Brad Whitlock
 */
public class ViewerMethods
{
    /**
     * Constructor for the ViewerMethods class.
     *
     * @param p The ViewerProxy that contains this object.
     */
    public ViewerMethods(ViewerProxy p)
    {
        proxy = p;
    }

    //
    // Map function calls to ViewerRPC calls. The order is the same as in 
    // ViewerMethods.C
    //

    /**
     * Tells the viewer to close.
     *
     * @return true on success; false otherwise.
     */
    public boolean Close()
    {
        GetViewerState().GetViewerRPC().SetRPCType(ViewerRPC.VIEWERRPCTYPE_CLOSERPC);
        GetViewerState().GetViewerRPC().Notify();
        return true;
    }

    /**
     * Tells the viewer to detach this client. This can be useful when you have 
     * multiple clients attached to the same viewer and this one wants to detach
     * itself while not disturbing the other clients.
     *
     * @return true on success; false otherwise.
     */
    public boolean Detach()
    {
        GetViewerState().GetViewerRPC().SetRPCType(ViewerRPC.VIEWERRPCTYPE_DETACHRPC);
        GetViewerState().GetViewerRPC().Notify();
        return true;
    }

    /**
     * Adds a new visualization window
     *
     * @return true on success; false otherwise.
     */
    public boolean AddWindow()
    {
        GetViewerState().GetViewerRPC().SetRPCType(ViewerRPC.VIEWERRPCTYPE_ADDWINDOWRPC);
        GetViewerState().GetViewerRPC().Notify();
        return Synchronize();
    }

    /**
     * Adds a new visualization window, copying all settings and plots from the
     * currently active visualization window.
     *
     * @return true on success; false otherwise.
     */
    public boolean CloneWindow()
    {
        GetViewerState().GetViewerRPC().SetRPCType(ViewerRPC.VIEWERRPCTYPE_CLONEWINDOWRPC);
        GetViewerState().GetViewerRPC().Notify();
        return Synchronize();
    }

    /**
     * Tells the viewer to close the active visualization window and delete it.
     *
     * @return true on success; false otherwise.
     */
    public boolean DeleteWindow()
    {
        GetViewerState().GetViewerRPC().SetRPCType(ViewerRPC.VIEWERRPCTYPE_DELETEWINDOWRPC);
        GetViewerState().GetViewerRPC().Notify();
        return Synchronize();
    }

    /**
     * Tells the viewer to set the window layout, which is the number of windows
     * on the screen in a particular arrangement.
     *
     * @param layout The layout value. Valid values are: 1,2,4,9,16.
     * @return true on success; false otherwise.
     */
    public boolean SetWindowLayout(int layout)
    {
        GetViewerState().GetViewerRPC().SetRPCType(ViewerRPC.VIEWERRPCTYPE_SETWINDOWLAYOUTRPC);
        GetViewerState().GetViewerRPC().SetWindowLayout(layout);
        GetViewerState().GetViewerRPC().Notify();
        return Synchronize();
    }

    /**
     * Sets the active visualization window.
     *
     * @param windowId The number of the visualization window >= 1.
     * @return true on success; false otherwise.
     */
    public boolean SetActiveWindow(int windowId)
    {
        GetViewerState().GetViewerRPC().SetRPCType(ViewerRPC.VIEWERRPCTYPE_SETACTIVEWINDOWRPC);
        GetViewerState().GetViewerRPC().SetWindowId(windowId);
        GetViewerState().GetViewerRPC().Notify();
        return Synchronize();
    }

    /**
     * Iconifies all of the viewer windows.
     *
     * @return true on success; false otherwise.
     */
    public boolean IconifyAllWindows()
    {
        GetViewerState().GetViewerRPC().SetRPCType(ViewerRPC.VIEWERRPCTYPE_ICONIFYALLWINDOWSRPC);
        GetViewerState().GetViewerRPC().Notify();
        return Synchronize();
    }

    /**
     * Brings all viewer windows back from an iconified state.
     *
     * @return true on success; false otherwise.
     */
    public boolean DeIconifyAllWindows()
    {
        GetViewerState().GetViewerRPC().SetRPCType(ViewerRPC.VIEWERRPCTYPE_DEICONIFYALLWINDOWSRPC);
        GetViewerState().GetViewerRPC().Notify();
        return Synchronize();
    }

    /**
     * Show the viewer windows. You must call this method to make the viewer
     * show its windows after being launched or no windows will be shown.
     *
     * @return true on success; false otherwise.
     */
    public void ShowAllWindows()
    {
        GetViewerState().GetViewerRPC().SetRPCType(ViewerRPC.VIEWERRPCTYPE_SHOWALLWINDOWSRPC);
        GetViewerState().GetViewerRPC().Notify();
    }

    /**
     * Hides all of the visualization windows (unmap them).
     *
     * @return true on success; false otherwise.
     */
    public boolean HideAllWindows()
    {
        GetViewerState().GetViewerRPC().SetRPCType(ViewerRPC.VIEWERRPCTYPE_HIDEALLWINDOWSRPC);
        GetViewerState().GetViewerRPC().Notify();
        return Synchronize();
    }

    /**
     * Clear the plots from a visualization window. This does not delete plots;
     * rather their resources are freed and they are put back into the new state
     * so they can be regenerated.
     *
     * @param clearAllPlots True to clear all plots in the plot list. If false,
     *                      only the active plots are cleared.
     * @return true on success; false otherwise.
     */
    public boolean ClearWindow(boolean clearAllPlots)
    {
        GetViewerState().GetViewerRPC().SetRPCType(ViewerRPC.VIEWERRPCTYPE_CLEARWINDOWRPC);
        GetViewerState().GetViewerRPC().SetBoolFlag(clearAllPlots);
        GetViewerState().GetViewerRPC().Notify();
        return Synchronize();
    }

    /**
     * Clear the plots in all visualization windows.
     *
     * @return true on success; false otherwise.
     */
    public boolean ClearAllWindows()
    {
        GetViewerState().GetViewerRPC().SetRPCType(ViewerRPC.VIEWERRPCTYPE_CLEARALLWINDOWSRPC);
        GetViewerState().GetViewerRPC().Notify();
        return Synchronize();
    }

    /**
     * Tells the viewer to tell the metadata server to connect back to this client. A 
     * Metadata server browses remote file systems and opens files to get their metadata.
     *
     * NOTE: This method is not fully supported since the Java client has no MDServerProxy.
     *
     * @param hostName The name of the host computer on which to launch the metadata server.
     * @param args A vector of strings containing command line arguments that the metadata
     *             server would use to connect back to this client.
     * @return true on success; false otherwise.
     */
    public boolean ConnectToMetaDataServer(String hostName, Vector args)
    {
        GetViewerState().GetViewerRPC().SetRPCType(ViewerRPC.VIEWERRPCTYPE_CONNECTTOMETADATASERVERRPC);
        GetViewerState().GetViewerRPC().SetProgramHost(hostName);
        GetViewerState().GetViewerRPC().SetProgramOptions(args);
        GetViewerState().GetViewerRPC().Notify();
        return Synchronize();
    }

    /**
     * Opens a database
     *
     * @param database The name of the database to open -- a filename. If the
     *                 file exists on a remote computer, then you can supply a
     *                 name of the form:  host:/path/file.ext
     * @return true on success; false otherwise.
     */
    public boolean OpenDatabase(String database)
    {
        GetViewerState().GetViewerRPC().SetRPCType(ViewerRPC.VIEWERRPCTYPE_OPENDATABASERPC);
        GetViewerState().GetViewerRPC().SetDatabase(database);
        GetViewerState().GetViewerRPC().SetIntArg1(0);
        GetViewerState().GetViewerRPC().SetBoolFlag(true);
        GetViewerState().GetViewerRPC().SetStringArg1("");
        GetViewerState().GetViewerRPC().Notify();
        return Synchronize();
    }

    /**
     * Opens a database at a given time state
     *
     * @param database The name of the database to open -- a filename. If the
     *                 file exists on a remote computer, then you can supply a
     *                 name of the form:  host:/path/file.ext
     * @param timeState The time state at which we're opening the database (>=0).
     *                  Pass 0 if you don't prefer a later time state.
     * @return true on success; false otherwise.
     */
    public boolean OpenDatabase(String database, int timeState)
    {
        GetViewerState().GetViewerRPC().SetRPCType(ViewerRPC.VIEWERRPCTYPE_OPENDATABASERPC);
        GetViewerState().GetViewerRPC().SetDatabase(database);
        GetViewerState().GetViewerRPC().SetIntArg1(timeState);
        GetViewerState().GetViewerRPC().SetBoolFlag(true);
        GetViewerState().GetViewerRPC().SetStringArg1("");
        GetViewerState().GetViewerRPC().Notify();
        return Synchronize();
    }

    /**
     * Opens a database at a given time state, with the opportunity for 
     * setting additional options related to the addition of default plots
     * or a forced file type.
     *
     * @param database The name of the database to open -- a filename. If the
     *                 file exists on a remote computer, then you can supply a
     *                 name of the form:  host:/path/file.ext
     * @param timeState The time state at which we're opening the database (>=0).
     *                  Pass 0 if you don't prefer a later time state.
     * @param addDefaultPlots True to add default plots if the database supports
     *                        them or false to prevent default plots from
     *                        being added.
     * @param forcedFileType The name of a database plugin that should be used
     *                       to open the database (e.g. "Silo_1.0"). If you
     *                       don't care which plugin opens the database, pass "".
     * @return true on success; false otherwise.
     */
    public boolean OpenDatabase(String database, int timeState,
                                boolean addDefaultPlots, String forcedFileType)
    {
        GetViewerState().GetViewerRPC().SetRPCType(ViewerRPC.VIEWERRPCTYPE_OPENDATABASERPC);
        GetViewerState().GetViewerRPC().SetDatabase(database);
        GetViewerState().GetViewerRPC().SetIntArg1(timeState);
        GetViewerState().GetViewerRPC().SetBoolFlag(addDefaultPlots);
        GetViewerState().GetViewerRPC().SetStringArg1(forcedFileType);
        GetViewerState().GetViewerRPC().Notify();
        return Synchronize();
    }

    /**
     * Closes the named database -- if it is not being used.
     *
     * @param database The name of the database to open -- a filename. If the
     *                 file exists on a remote computer, then you can supply a
     *                 name of the form:  host:/path/file.ext
     * @return true on success; false otherwise.
     */
    public boolean CloseDatabase(String database)
    {
        GetViewerState().GetViewerRPC().SetRPCType(ViewerRPC.VIEWERRPCTYPE_CLOSEDATABASERPC);
        GetViewerState().GetViewerRPC().SetDatabase(database);
        GetViewerState().GetViewerRPC().Notify();
        return Synchronize();
    }

    /**
     * Makes the named database be the active database, which is the database that
     * can be used to create plots and is the database whose variables are shown
     * in the plot menus. A database can only be made active if it has been
     * previously opened via OpenDatabase().
     *
     * @param database The name of the database to open -- a filename. If the
     *                 file exists on a remote computer, then you can supply a
     *                 name of the form:  host:/path/file.ext
     * @return true on success; false otherwise.
     */
    public boolean ActivateDatabase(String database)
    {
        GetViewerState().GetViewerRPC().SetRPCType(ViewerRPC.VIEWERRPCTYPE_ACTIVATEDATABASERPC);
        GetViewerState().GetViewerRPC().SetDatabase(database);
        GetViewerState().GetViewerRPC().Notify();
        return Synchronize();
    }

    /**
     * If the database is a virtual database (collection of single related files
     * that have been grouped into a time-varying database) then this method
     * checks to see if there are any new files that need to be added to the
     * database. This method is cheaper than ReOpenDatabase since it adds new
     * files to a database without causing plots to be regenerated. This method
     * is used when you are monitoring the progress of a simulation that keeps
     * writing out files that you want to plot.
     *
     * @param database The name of the database to open -- a filename. If the
     *                 file exists on a remote computer, then you can supply a
     *                 name of the form:  host:/path/file.ext
     * @return true on success; false otherwise.
     */
    public boolean CheckForNewStates(String database)
    {
        GetViewerState().GetViewerRPC().SetRPCType(ViewerRPC.VIEWERRPCTYPE_CHECKFORNEWSTATESRPC);
        GetViewerState().GetViewerRPC().SetDatabase(database);
        GetViewerState().GetViewerRPC().Notify();
        return Synchronize();
    }

    /**
     * Reopens a database that has been opened before, adding new time states if
     * they exist and causing all plots to be regenerated using the new data.
     * This method can be useful when you overwrite a database that VisIt was 
     * plotting and you want to plot the new data.
     *
     * @param database The name of the database to open -- a filename. If the
     *                 file exists on a remote computer, then you can supply a
     *                 name of the form:  host:/path/file.ext
     * @return true on success; false otherwise.
     */
    public boolean ReOpenDatabase(String database, boolean forceClose)
    {
        GetViewerState().GetViewerRPC().SetRPCType(ViewerRPC.VIEWERRPCTYPE_REOPENDATABASERPC);
        GetViewerState().GetViewerRPC().SetDatabase(database);
        GetViewerState().GetViewerRPC().SetIntArg1(forceClose?1:0);
        GetViewerState().GetViewerRPC().Notify();
        return Synchronize();
    }

    /**
     * Replaces the database in the current plots with the specified database.
     * Note that if the new database does not contain the variables for a plot
     * then the plot will not have its database changed to the database provided
     * by this method. ReplaceDatabase can be useful in the situation where you
     * have set up a visualization using results from one simulation and then
     * you want to replace the plots' database with the results of a related but
     * different simulation.
     *
     * @param database The name of the database to open -- a filename. If the
     *                 file exists on a remote computer, then you can supply a
     *                 name of the form:  host:/path/file.ext
     * @param timeState The time state at which to open the database (>=0). Pass
     *                  0 if you don't prefer a later time state.
     * @return true on success; false otherwise.
     */
    public boolean ReplaceDatabase(String database, int timeState)
    {
        GetViewerState().GetViewerRPC().SetRPCType(ViewerRPC.VIEWERRPCTYPE_REPLACEDATABASERPC);
        GetViewerState().GetViewerRPC().SetDatabase(database);
        GetViewerState().GetViewerRPC().SetIntArg1(timeState);
        GetViewerState().GetViewerRPC().Notify();
        return Synchronize();
    }

    /**
     * Opens the specified database and creates duplicates of all of the existing
     * plots using the new database.
     *
     * @param database The name of the database to open -- a filename. If the
     *                 file exists on a remote computer, then you can supply a
     *                 name of the form:  host:/path/file.ext
     * @param timeState The time state at which to open the database (>=0). Pass
     *                  0 if you don't prefer a later time state.
     * @return true on success; false otherwise.
     */
    public boolean OverlayDatabase(String database, int timeState)
    {
        GetViewerState().GetViewerRPC().SetRPCType(ViewerRPC.VIEWERRPCTYPE_OVERLAYDATABASERPC);
        GetViewerState().GetViewerRPC().SetDatabase(database);
        GetViewerState().GetViewerRPC().SetIntArg1(timeState);
        GetViewerState().GetViewerRPC().Notify();
        return Synchronize();
    }

    /**
     * Requests metadata for the specified database at a particular time state.
     * You can use the metadata to determine information such as which variables
     * are in the database. The metadata comes back in the avtDatabaseMetaData 
     * object in ViewerState.This method does not disrupt the active database. 
     *
     * @param database The name of the database to open -- a filename. If the
     *                 file exists on a remote computer, then you can supply a
     *                 name of the form:  host:/path/file.ext
     * @param timeState The time state at which to open the database (>=0). You
     *                  can provide a time state since some databases can provide
     *                  different metadata depending on the time state. If you
     *                  don't know or don't care about the time state then pass
     *                  0.
     * @return true on success; false otherwise.
     */
    public boolean RequestMetaData(String database, int timeState)
    {
        GetViewerState().GetViewerRPC().SetRPCType(ViewerRPC.VIEWERRPCTYPE_REQUESTMETADATARPC);
        GetViewerState().GetViewerRPC().SetDatabase(database);
        GetViewerState().GetViewerRPC().SetStateNumber(timeState);
        GetViewerState().GetViewerRPC().Notify();
        return Synchronize();
    }

    /**
     * Tell the viewer to export the data for the current plot as a database file.
     * You should first set the options in the ExportDBAttributes in ViewerState
     * before calling this method.
     *
     * Example:<br>
     *   <p class="example">
     *    ExportDBAttributes atts = proxy.GetViewerState().GetExportDBAttributes();<br>
     *    // Modify the atts here... (not shown)<br>
     *    atts.Notify();<br>
     *    proxy.GetViewerMethods().ExportDatabase();</p>
     *
     * @return true on success; false otherwise.
     */
    public boolean ExportDatabase()
    {
        GetViewerState().GetViewerRPC().SetRPCType(ViewerRPC.VIEWERRPCTYPE_EXPORTDBRPC);
        GetViewerState().GetViewerRPC().Notify();
        return Synchronize();
    }

    /**
     * Tell VisIt to construct a DDF (derived data function), which is often a 
     * statistical variable derived from your data after binning.
     * You should first set the options in the ConstructDDFAttributes in ViewerState
     * before calling this method.
     *
     * Example:<br>
     *   <p class="example">
     *    ConstructDDFAttributes atts = proxy.GetViewerState().GetConstructDDFAttributes();<br>
     *    // Modify the atts here... (not shown)<br>
     *    atts.Notify();<br>
     *    proxy.GetViewerMethods().ConstructDDF();</p>
     *
     * @return true on success; false otherwise.
     */
    public boolean ConstructDDF()
    {
        GetViewerState().GetViewerRPC().SetRPCType(ViewerRPC.VIEWERRPCTYPE_CONSTRUCTDDFRPC);
        GetViewerState().GetViewerRPC().Notify();
        return Synchronize();
    }

    /**
     * Tell VisIt to clear its cache for the compute engine on the specified host.
     * This method can free up considerable memory resources for compute engines
     * that have processed many complex requests.
     *
     * @param hostName The name of the computer where the compute engine is running.
     * @param simName The name of the simulation. If you're not running a 
     *                simulation, pass "".
     * @return true on success; false otherwise.
     */
    public boolean ClearCache(String hostName, String simName)
    {
        GetViewerState().GetViewerRPC().SetRPCType(ViewerRPC.VIEWERRPCTYPE_CLEARCACHERPC);
        GetViewerState().GetViewerRPC().SetProgramHost(hostName);
        GetViewerState().GetViewerRPC().SetProgramSim(simName);
        GetViewerState().GetViewerRPC().Notify();
        return Synchronize();
    }

    /**
     * Tell VisIt to clear its cache for all compute engines.
     * This method can free up considerable memory resources for compute engines
     * that have processed many complex requests.
     *
     * @return true on success; false otherwise.
     */
    public boolean ClearCacheForAllEngines()
    {
        GetViewerState().GetViewerRPC().SetRPCType(ViewerRPC.VIEWERRPCTYPE_CLEARCACHEFORALLENGINESRPC);
        GetViewerState().GetViewerRPC().Notify();
        return Synchronize();
    }

    /**
     * Tell VisIt to send the database plugin information for the specified 
     * host. This method lets us determine which database plugins are available
     * various computers.
     *
     * @param hostName The name of the computer for which we want database
     *                 plugin information.
     * @return true on success; false otherwise.
     */
    public boolean UpdateDBPluginInfo(String hostName)
    {
        GetViewerState().GetViewerRPC().SetRPCType(ViewerRPC.VIEWERRPCTYPE_UPDATEDBPLUGININFORPC);
        GetViewerState().GetViewerRPC().SetProgramHost(hostName);
        GetViewerState().GetViewerRPC().Notify();
        return Synchronize();
    }

    /**
     * Create a database correlation, which is a mapping that relates 
     * several time-varying databases so they can be controlled by a single
     * time slider. Creating a database correlation causes a new time slider
     * to be created.
     *
     * @param name The name of the new database correlation.
     * @param dbs  A vector of String objects containing the names of the databases
     *             to include in the database correlation.
     * @param method See correlation methods in DatabaseCorrelation class.
     * @param nStates The number of time states that the new database correlation should have.
     * @return true on success; false otherwise.
     */
    public boolean CreateDatabaseCorrelation(String name, Vector dbs, int method, int nStates)
    {
        GetViewerState().GetViewerRPC().SetRPCType(ViewerRPC.VIEWERRPCTYPE_CREATEDATABASECORRELATIONRPC);
        GetViewerState().GetViewerRPC().SetDatabase(name);
        GetViewerState().GetViewerRPC().SetProgramOptions(dbs);
        GetViewerState().GetViewerRPC().SetIntArg1(method);
        GetViewerState().GetViewerRPC().SetIntArg2(nStates);
        GetViewerState().GetViewerRPC().Notify();
        return Synchronize();
    }

    /**
     * Alters an existing database correlation.
     *
     * @param name The name of the database correlation.
     * @param dbs  A vector of String objects containing the names of the databases
     *             to include in the database correlation.
     * @param method See correlation methods in DatabaseCorrelation class.
     * @param nStates The number of time states that the new database correlation should have.
     * @return true on success; false otherwise.
     */
    public boolean AlterDatabaseCorrelation(String name, Vector dbs, int method, int nStates)
    {
        GetViewerState().GetViewerRPC().SetRPCType(ViewerRPC.VIEWERRPCTYPE_ALTERDATABASECORRELATIONRPC);
        GetViewerState().GetViewerRPC().SetDatabase(name);
        GetViewerState().GetViewerRPC().SetProgramOptions(dbs);
        GetViewerState().GetViewerRPC().SetIntArg1(method);
        GetViewerState().GetViewerRPC().SetIntArg2(nStates);
        GetViewerState().GetViewerRPC().Notify();
        return Synchronize();
    }

    /**
     * Deletes an existing database correlation.
     *
     * @param name The name of the database correlation to delete.
     * @return true on success; false otherwise.
     */
    public boolean DeleteDatabaseCorrelation(String name)
    {
        GetViewerState().GetViewerRPC().SetRPCType(ViewerRPC.VIEWERRPCTYPE_DELETEDATABASECORRELATIONRPC);
        GetViewerState().GetViewerRPC().SetDatabase(name);
        GetViewerState().GetViewerRPC().Notify();
        return Synchronize();
    }

    /**
     * Tells VisIt's viewer to launch a compute engine on the specified host using 
     * the designated command line arguments that affect how the compute engine
     * is launched.
     *
     * @param hostName The name of the host computer where the compute engine will
     *                 be launched. Note that only one compute engine may be launched
     *                 per host.
     * @param argv Command line arguments for the compute engine that affect how it
     *             will be launched.
     * @return true on success; false otherwise.
     */
    public boolean OpenComputeEngine(String hostName, Vector argv)
    {
        GetViewerState().GetViewerRPC().SetRPCType(ViewerRPC.VIEWERRPCTYPE_OPENCOMPUTEENGINERPC);
        GetViewerState().GetViewerRPC().SetProgramHost(hostName);
        GetViewerState().GetViewerRPC().SetProgramOptions(argv);
        GetViewerState().GetViewerRPC().Notify();
        return Synchronize();
    }

    /**
     * Tells VisIt's viewer to close the compute engine on the specified host.
     *
     * @param hostName The name of the host computer where the compute engine is
     *                 running.
     * @param simName The name of the simulation on the host. If you're not closing
     *                a simulation, pass "".
     * @return true on success; false otherwise.
     */
    public boolean CloseComputeEngine(String hostName, String simName)
    {
        GetViewerState().GetViewerRPC().SetRPCType(ViewerRPC.VIEWERRPCTYPE_CLOSECOMPUTEENGINERPC);
        GetViewerState().GetViewerRPC().SetProgramHost(hostName);
        GetViewerState().GetViewerRPC().SetProgramSim(simName);
        GetViewerState().GetViewerRPC().Notify();
        return Synchronize();
    }

    /**
     * Interrupt the compute engine that is processing data for a plot. This stops
     * the compute engine from completing its work and the plot is put into the 
     * error state. DrawPlots() must be called for the plot to be processed again.
     *
     * @return true on success; false otherwise.
     */
    public void InterruptComputeEngine()
    {
        proxy.SendInterruption();
    }

    /**
     * Tells VisIt's viewer to launch a metadata server on the specified host using 
     * the designated command line arguments that affect how the metadata server
     * is launched. A metadata server is used to browse file systems and return
     * file metadata.
     *
     * @param hostName The name of the host computer where the compute engine will
     *                 be launched. Note that only one compute engine may be launched
     *                 per host.
     * @param argv Command line arguments for the compute engine that affect how it
     *             will be launched.
     * @return true on success; false otherwise.
     */
    public boolean OpenMDServer(String hostName, Vector argv)
    {
        GetViewerState().GetViewerRPC().SetRPCType(ViewerRPC.VIEWERRPCTYPE_OPENMDSERVERRPC);
        GetViewerState().GetViewerRPC().SetProgramHost(hostName);
        GetViewerState().GetViewerRPC().SetProgramOptions(argv);
        GetViewerState().GetViewerRPC().Notify();
        return Synchronize();
    }

    /**
     * Sets the number of animation frames for a keyframe animation. Note that keyframing
     * must be enabled for this method to have an effect.
     *
     * @param nFrames The number of animation frames to create for the visualization.
     * @return true on success; false otherwise.
     */
    public boolean AnimationSetNFrames(int nFrames)
    {
        GetViewerState().GetViewerRPC().SetRPCType(ViewerRPC.VIEWERRPCTYPE_ANIMATIONSETNFRAMESRPC);
        GetViewerState().GetViewerRPC().SetNFrames(nFrames);
        GetViewerState().GetViewerRPC().Notify();
        return Synchronize();
    }

    /**
     * Animate through the time states for the visualization of a time-varying database. Only
     * plots that are affected by the active time slider are animated.
     *
     * @return true on success; false otherwise.
     */
    public boolean AnimationPlay()
    {
        GetViewerState().GetViewerRPC().SetRPCType(ViewerRPC.VIEWERRPCTYPE_ANIMATIONPLAYRPC);
        GetViewerState().GetViewerRPC().Notify();
        return Synchronize();
    }

    /**
     * Animate through the time states for the visualization of a time-varying database
     * in reverse order. Only plots that are affected by the active time slider 
     * are animated.
     *
     * @return true on success; false otherwise.
     */
    public boolean AnimationReversePlay()
    {
        GetViewerState().GetViewerRPC().SetRPCType(ViewerRPC.VIEWERRPCTYPE_ANIMATIONREVERSEPLAYRPC);
        GetViewerState().GetViewerRPC().Notify();
        return Synchronize();
    }

    /**
     * Stop animation.
     *
     * @return true on success; false otherwise.
     */
    public boolean AnimationStop()
    {
        GetViewerState().GetViewerRPC().SetRPCType(ViewerRPC.VIEWERRPCTYPE_ANIMATIONSTOPRPC);
        GetViewerState().GetViewerRPC().Notify();
        return Synchronize();
    }

    /**
     * Go to the next time state for the active time slider. This causes plots that
     * are affected by the active time slider to go to the next time state as
     * determined by their database correlation.
     *
     * @return true on success; false otherwise.
     */
    public boolean TimeSliderNextState()
    {
        GetViewerState().GetViewerRPC().SetRPCType(ViewerRPC.VIEWERRPCTYPE_TIMESLIDERNEXTSTATERPC);
        GetViewerState().GetViewerRPC().Notify();
        return Synchronize();
    }

    /**
     * Go to the previous time state for the active time slider. This causes plots that
     * are affected by the active time slider to go to the previous time state as
     * determined by their database correlation.
     *
     * @return true on success; false otherwise.
     */
    public boolean TimeSliderPreviousState()
    {
        GetViewerState().GetViewerRPC().SetRPCType(ViewerRPC.VIEWERRPCTYPE_TIMESLIDERPREVIOUSSTATERPC);
        GetViewerState().GetViewerRPC().Notify();
        return Synchronize();
    }

    /**
     * Go to the specified time state for the active time slider. This will cause
     * plots that use the active time slider to be updated to a time state
     * determined by their database correlation.
     *
     * @return true on success; false otherwise.
     */
    public boolean SetTimeSliderState(int state)
    {
        GetViewerState().GetViewerRPC().SetRPCType(ViewerRPC.VIEWERRPCTYPE_SETTIMESLIDERSTATERPC);
        GetViewerState().GetViewerRPC().SetStateNumber(state);
        GetViewerState().GetViewerRPC().Notify();
        return Synchronize();
    }

    /**
     * Set the name of the active time slider. This is the name of a database 
     * correlation, which is often a host&path qualified database name. The valid
     * time slider names for a visualization window are given in the timeSliders 
     * member in the WindowInformation state object in ViewerState.
     *
     * @param ts The name of the new active time slider.
     * @return true on success; false otherwise.
     */
    public boolean SetActiveTimeSlider(String ts)
    {
        GetViewerState().GetViewerRPC().SetRPCType(ViewerRPC.VIEWERRPCTYPE_SETACTIVETIMESLIDERRPC);
        GetViewerState().GetViewerRPC().SetDatabase(ts);
        GetViewerState().GetViewerRPC().Notify();
        return Synchronize();
    }

    /**
     * Save the active visualization window as an image file or in some cases as 
     * a geometry file. You can set window-saving options by calling SetSaveWindowAttributes()
     * before calling SaveWindow().
     *
     * @return true on success; false otherwise.
     */
    public boolean SaveWindow()
    {
        GetViewerState().GetViewerRPC().SetRPCType(ViewerRPC.VIEWERRPCTYPE_SAVEWINDOWRPC);
        GetViewerState().GetViewerRPC().Notify();
        return Synchronize();
    }

    /**
     * Prints the active visualization window. You can set printer options beforehand 
     * like this:
     *
     * PrinterAttributes atts = proxy.GetViewerState().GetPrinterAttributes();
     * // Set printer options here ... (not shown)
     * atts.Notify();
     * PrintWindow();
     *
     * @return true on success; false otherwise.
     */
    public boolean PrintWindow()
    {
        GetViewerState().GetViewerRPC().SetRPCType(ViewerRPC.VIEWERRPCTYPE_PRINTWINDOWRPC);
        GetViewerState().GetViewerRPC().Notify();
        return Synchronize();
    }

    /**
     * Disables the viewer's ability to redraw its plots when you make changes. This can
     * be a useful option when you plan to make several updates that each would have caused
     * the viewer to redraw its plots. Disabling the intermediate updates causes the
     * visualization to be completed more quickly.
     *
     * @return true on success; false otherwise.
     */
    public boolean DisableRedraw()
    {
        GetViewerState().GetViewerRPC().SetRPCType(ViewerRPC.VIEWERRPCTYPE_DISABLEREDRAWRPC);
        GetViewerState().GetViewerRPC().Notify();
        return Synchronize();
    }

    /**
     * Force the viewer to redraw the plots in the active visualization window.
     *
     * @return true on success; false otherwise.
     */
    public boolean RedrawWindow()
    {
        GetViewerState().GetViewerRPC().SetRPCType(ViewerRPC.VIEWERRPCTYPE_REDRAWRPC);
        GetViewerState().GetViewerRPC().Notify();
        return Synchronize();
    }

    /**
     * Resize the specified visualization window. The provided width and height are
     * in pixels and usually correspond to the dimensions of the window's exterior
     * bounds, which includes the window decorations.
     *
     * @param win The window id of the window to resize (>=1).
     * @param w The new window width in pixels.
     * @param h The new window height in pixels.
     * @return true on success; false otherwise.
     */
    public boolean ResizeWindow(int win, int w, int h)
    {
        GetViewerState().GetViewerRPC().SetRPCType(ViewerRPC.VIEWERRPCTYPE_RESIZEWINDOWRPC);
        GetViewerState().GetViewerRPC().SetWindowId(win);
        GetViewerState().GetViewerRPC().SetIntArg1(w);
        GetViewerState().GetViewerRPC().SetIntArg2(h);
        GetViewerState().GetViewerRPC().Notify();
        return Synchronize();
    }

    /**
     * Move the specified visualization window to a location on the screen.
     *
     * @param win The window id of the window to resize (>=1).
     * @param x The new x location on the screen.
     * @param y The new y location on the screen.
     * @return true on success; false otherwise.
     */
    public boolean MoveWindow(int win, int x, int y)
    {
        GetViewerState().GetViewerRPC().SetRPCType(ViewerRPC.VIEWERRPCTYPE_MOVEWINDOWRPC);
        GetViewerState().GetViewerRPC().SetWindowId(win);
        GetViewerState().GetViewerRPC().SetIntArg1(x);
        GetViewerState().GetViewerRPC().SetIntArg2(y);
        GetViewerState().GetViewerRPC().Notify();
        return Synchronize();
    }

    /**
     * Move the specified visualization window to a location on the screen and also
     * resize it.
     *
     * @param win The window id of the window to resize (>=1).
     * @param x The new x location on the screen.
     * @param y The new y location on the screen.
     * @param w The new window width in pixels.
     * @param h The new window height in pixels.
     * @return true on success; false otherwise.
     */
    public boolean MoveAndResizeWindow(int win, int x, int y, int w, int h)
    {
        GetViewerState().GetViewerRPC().SetRPCType(ViewerRPC.VIEWERRPCTYPE_MOVEWINDOWRPC);
        GetViewerState().GetViewerRPC().SetWindowId(win);
        GetViewerState().GetViewerRPC().SetIntArg1(x);
        GetViewerState().GetViewerRPC().SetIntArg2(y);
        GetViewerState().GetViewerRPC().SetIntArg3(w);
        GetViewerState().GetViewerRPC().SetWindowLayout(h);
        GetViewerState().GetViewerRPC().Notify();
        return Synchronize();
    }

    /**
     * Hide the toolbars in a visualization window.
     *
     * @param forAllWindows True to hide the toolbars in all visualization windows.
     *                      false to hide the toolbars only in the active 
     *                      visualization window.
     * @return true on success; false otherwise.
     */
    public boolean HideToolbars(boolean forAllWindows)
    {
        if(forAllWindows)
            GetViewerState().GetViewerRPC().SetRPCType(ViewerRPC.VIEWERRPCTYPE_HIDETOOLBARSFORALLWINDOWSRPC);
        else
            GetViewerState().GetViewerRPC().SetRPCType(ViewerRPC.VIEWERRPCTYPE_HIDETOOLBARSRPC);
        GetViewerState().GetViewerRPC().Notify();
        return Synchronize();
    }

    /**
     * Show the toolbars in a visualization window.
     *
     * @param forAllWindows True to show the toolbars in all visualization windows.
     *                      false to show the toolbars only in the active 
     *                      visualization window.
     * @return true on success; false otherwise.
     */
    public boolean ShowToolbars(boolean forAllWindows)
    {
        if(forAllWindows)
            GetViewerState().GetViewerRPC().SetRPCType(ViewerRPC.VIEWERRPCTYPE_SHOWTOOLBARSFORALLWINDOWSRPC);
        else
            GetViewerState().GetViewerRPC().SetRPCType(ViewerRPC.VIEWERRPCTYPE_SHOWTOOLBARSRPC);
        GetViewerState().GetViewerRPC().Notify();
        return Synchronize();
    }

    /**
     * Adds a new plot.
     *
     * @param type The index of the plot type to be created.
     * @param var The variable to be plotted. The variable must exist in the active
     *            database and it must be a type that is compatible with the plot
     *            type (i.e. Pseudocolor plots get scalar variables -- not mesh 
     *            variables).
     * @return true on success; false otherwise.
     */
    public boolean AddPlot(int type, String var)
    {
        GetViewerState().GetViewerRPC().SetRPCType(ViewerRPC.VIEWERRPCTYPE_ADDPLOTRPC);
        GetViewerState().GetViewerRPC().SetPlotType(type);
        GetViewerState().GetViewerRPC().SetVariable(var);
        GetViewerState().GetViewerRPC().Notify();
        return Synchronize();
    }

    /**
     * Adds a new plot.
     *
     * @param plotName The name of the plot type to be created.
     * @param var The variable to be plotted. The variable must exist in the active
     *            database and it must be a type that is compatible with the plot
     *            type (i.e. Pseudocolor plots get scalar variables -- not mesh 
     *            variables).
     * @return true on success; false otherwise.
     */
    public boolean AddPlot(String plotName, String var)
    {
        boolean retval = false;
        int type = proxy.GetPlotIndex(plotName);
        if(type > -1)
        {
            GetViewerState().GetViewerRPC().SetRPCType(ViewerRPC.VIEWERRPCTYPE_ADDPLOTRPC);
            GetViewerState().GetViewerRPC().SetPlotType(type);
            GetViewerState().GetViewerRPC().SetVariable(var);
            GetViewerState().GetViewerRPC().Notify();
            retval = Synchronize();
        }

        return retval;
    }

    /**
     * Creates identical copies of the active plots and puts them into the new
     * state so their plot attributes can be changed.
     *
     * @return true on success; false otherwise.
     */
    public boolean CopyActivePlots()
    {
        GetViewerState().GetViewerRPC().SetRPCType(ViewerRPC.VIEWERRPCTYPE_COPYACTIVEPLOTSRPC);
        GetViewerState().GetViewerRPC().Notify();
        return Synchronize();
    }

    public boolean SetPlotFrameRange(int plotID, int frame0, int frame1)
    {
        GetViewerState().GetViewerRPC().SetRPCType(ViewerRPC.VIEWERRPCTYPE_SETPLOTFRAMERANGERPC);
        GetViewerState().GetViewerRPC().SetIntArg1(plotID);
        GetViewerState().GetViewerRPC().SetIntArg2(frame0);
        GetViewerState().GetViewerRPC().SetIntArg3(frame1);
        GetViewerState().GetViewerRPC().Notify();
        return Synchronize();
    }

    public boolean DeletePlotKeyframe(int plotId, int frame)
    {
        GetViewerState().GetViewerRPC().SetRPCType(ViewerRPC.VIEWERRPCTYPE_DELETEPLOTKEYFRAMERPC);
        GetViewerState().GetViewerRPC().SetIntArg1(plotId);
        GetViewerState().GetViewerRPC().SetIntArg2(frame);
        GetViewerState().GetViewerRPC().Notify();
        return Synchronize();
    }

    public boolean MovePlotKeyframe(int plotId, int oldFrame, int newFrame)
    {
        GetViewerState().GetViewerRPC().SetRPCType(ViewerRPC.VIEWERRPCTYPE_MOVEPLOTKEYFRAMERPC);
        GetViewerState().GetViewerRPC().SetIntArg1(plotId);
        GetViewerState().GetViewerRPC().SetIntArg2(oldFrame);
        GetViewerState().GetViewerRPC().SetIntArg3(newFrame);
        GetViewerState().GetViewerRPC().Notify();
        return Synchronize();
    }

    public boolean SetPlotDatabaseState(int plotId, int frame, int ts)
    {
        GetViewerState().GetViewerRPC().SetRPCType(ViewerRPC.VIEWERRPCTYPE_MOVEPLOTKEYFRAMERPC);
        GetViewerState().GetViewerRPC().SetIntArg1(plotId);
        GetViewerState().GetViewerRPC().SetIntArg2(frame);
        GetViewerState().GetViewerRPC().SetIntArg3(ts);
        GetViewerState().GetViewerRPC().Notify();
        return Synchronize();
    }

    public boolean DeletePlotDatabaseKeyframe(int plotId, int frame)
    {
        GetViewerState().GetViewerRPC().SetRPCType(ViewerRPC.VIEWERRPCTYPE_DELETEPLOTDATABASEKEYFRAMERPC);
        GetViewerState().GetViewerRPC().SetIntArg1(plotId);
        GetViewerState().GetViewerRPC().SetIntArg2(frame);
        GetViewerState().GetViewerRPC().Notify();
        return Synchronize();
    }

    public boolean MovePlotDatabaseKeyframe(int plotId, int oldFrame, int newFrame)
    {
        GetViewerState().GetViewerRPC().SetRPCType(ViewerRPC.VIEWERRPCTYPE_MOVEPLOTDATABASEKEYFRAMERPC);
        GetViewerState().GetViewerRPC().SetIntArg1(plotId);
        GetViewerState().GetViewerRPC().SetIntArg2(oldFrame);
        GetViewerState().GetViewerRPC().SetIntArg3(newFrame);
        GetViewerState().GetViewerRPC().Notify();
        return Synchronize();
    }

    /**
     * Deletes the active plots, freeing all of their data, and removing them from
     * the plot list.
     *
     * @return true on success; false otherwise.
     */
    public boolean DeleteActivePlots()
    {
        GetViewerState().GetViewerRPC().SetRPCType(ViewerRPC.VIEWERRPCTYPE_DELETEACTIVEPLOTSRPC);
        GetViewerState().GetViewerRPC().Notify();
        return Synchronize();
    }

    /**
     * Toggles the hidden flag on the visible plots. Hiding a hidden plot shows it.
     *
     * @return true on success; false otherwise.
     */
    public boolean HideActivePlots()
    {
        GetViewerState().GetViewerRPC().SetRPCType(ViewerRPC.VIEWERRPCTYPE_HIDEACTIVEPLOTSRPC);
        GetViewerState().GetViewerRPC().Notify();
        return Synchronize();
    }

    /**
     * Sets a flag on the active plots indicating whether they follow time or not.
     * If a plot does not follow time then it is disconnected from its time slider
     * and updating the time slider will not have any effect on the plot.
     *
     * @return true on success; false otherwise.
     */
    public boolean SetPlotFollowsTime()
    {
        GetViewerState().GetViewerRPC().SetRPCType(ViewerRPC.VIEWERRPCTYPE_SETPLOTFOLLOWSTIMERPC);
        GetViewerState().GetViewerRPC().Notify();
        return Synchronize();    
    }

    /**
     * Causes VisIt to process any plots that are new or in the error state so
     * they get computed in the compute engine and shown in the visualization window.
     *
     * @return true on success; false otherwise.
     */
    public boolean DrawPlots()
    {
        GetViewerState().GetViewerRPC().SetRPCType(ViewerRPC.VIEWERRPCTYPE_DRAWPLOTSRPC);
        GetViewerState().GetViewerRPC().SetBoolFlag(true);
        GetViewerState().GetViewerRPC().Notify();
        return Synchronize();
    }

    /**
     * Causes VisIt to process any plots that are new or in the error state so
     * they get computed in the compute engine and shown in the visualization window.
     *
     * @param drawAllPlots True is the default behavior and draws all plots. If you
     *                     only want to draw the active plots then pass false.
     * @return true on success; false otherwise.
     */
    public boolean DrawPlots(boolean drawAllPlots)
    {
        GetViewerState().GetViewerRPC().SetRPCType(ViewerRPC.VIEWERRPCTYPE_DRAWPLOTSRPC);
        GetViewerState().GetViewerRPC().SetBoolFlag(drawAllPlots);
        GetViewerState().GetViewerRPC().Notify();
        return Synchronize();
    }

    /**
     * Makes a list of plots in the plot list active while also setting the
     * active operators for each plot and also setting which plots are expanded.
     *
     * @param activePlots Vector of Integer objects containing the ids of the 
     *                    new active plots in the plot list. Plot list indices
     *                    begin at 0.
     * @param activeOperators Vector of Integer objects containing the ids of the
     *                        active operators for all plots in the plot list.
     * @param expandedPlots Vector of Bool objects (1 for each plot in the plot
     *                      list) each containing a flag indicating whether a 
     *                      plot is expanded.
     * @return true on success; false otherwise.
     */
    public boolean SetActivePlots(Vector activePlots, Vector activeOperators,
                                  Vector expandedPlots)
    {
        GetViewerState().GetViewerRPC().SetRPCType(ViewerRPC.VIEWERRPCTYPE_SETACTIVEPLOTSRPC);
        GetViewerState().GetViewerRPC().SetActivePlotIds(activePlots);
        GetViewerState().GetViewerRPC().SetActiveOperatorIds(activeOperators);
        GetViewerState().GetViewerRPC().SetExpandedPlotIds(expandedPlots);
        GetViewerState().GetViewerRPC().SetBoolFlag(true);
        GetViewerState().GetViewerRPC().Notify();
        return Synchronize();
    }

    /**
     * Makes a list of plots in the plot list active.
     *
     * @param ids Vector of Integer objects containing the ids of the new active 
     *            plots in the plot list. Plot list indices begin at 0.
     * @return true on success; false otherwise.
     */
    public boolean SetActivePlots(Vector ids)
    {
        GetViewerState().GetViewerRPC().SetRPCType(ViewerRPC.VIEWERRPCTYPE_SETACTIVEPLOTSRPC);
        GetViewerState().GetViewerRPC().SetActivePlotIds(ids);
        GetViewerState().GetViewerRPC().SetBoolFlag(false);
        GetViewerState().GetViewerRPC().Notify();
        return Synchronize();
    }

    /**
     * Makes a list of plots in the plot list active.
     *
     * @param ids Array of new active plots in the plot list. Plot list indices begin at 0.
     * @return true on success; false otherwise.
     */
    public boolean SetActivePlots(int[] ids)
    {
        GetViewerState().GetViewerRPC().SetRPCType(ViewerRPC.VIEWERRPCTYPE_SETACTIVEPLOTSRPC);
        Vector iv = new Vector();
        for(int i = 0; i < ids.length; ++i)
            iv.addElement(new Integer(ids[i]));
        GetViewerState().GetViewerRPC().SetActivePlotIds(iv);
        GetViewerState().GetViewerRPC().SetBoolFlag(false);
        GetViewerState().GetViewerRPC().Notify();
        return Synchronize();
    }

    /**
     * Makes a single plot in the plot list active.
     *
     * @param index Index of the new active plot in the plot list. Plot list indices begin at 0.
     * @return true on success; false otherwise.
     */
    public boolean SetActivePlot(int index)
    {
        int[] ids = new int[1];
        ids[0] = index;
        return SetActivePlots(ids);
    }

    /**
     * Changes the plotted variable for the active plots.
     *
     * @param var The name of the new variable to use in the active plots. The variable
     *            must exist in the active database.
     * @return true on success; false otherwise.
     */
    public boolean ChangeActivePlotsVar(String var)
    {
        GetViewerState().GetViewerRPC().SetRPCType(ViewerRPC.VIEWERRPCTYPE_CHANGEACTIVEPLOTSVARRPC);
        GetViewerState().GetViewerRPC().SetVariable(var);
        GetViewerState().GetViewerRPC().Notify();
        return Synchronize();
    }

    /**
     * Adds an operator to the active plots.
     *
     * @param oper Type of the operator to be added, which is an index in the 
     *             operator plugin manager.
     * @return true on success; false otherwise.
     */
    public boolean AddOperator(int oper)
    {
        GetViewerState().GetViewerRPC().SetRPCType(ViewerRPC.VIEWERRPCTYPE_ADDOPERATORRPC);
        GetViewerState().GetViewerRPC().SetOperatorType(oper);
        GetViewerState().GetViewerRPC().SetBoolFlag(true);
        GetViewerState().GetViewerRPC().Notify();
        return Synchronize();
    }

    /**
     * Adds an operator to the active plots.
     *
     * @param oper Type of the operator to be added, which is an index in the 
     *             operator plugin manager.
     * @param fromDefault True to initilize the operator from the default operator
     *                    attributes.
     * @return true on success; false otherwise.
     */
    public boolean AddOperator(int oper, boolean fromDefault)
    {
        GetViewerState().GetViewerRPC().SetRPCType(ViewerRPC.VIEWERRPCTYPE_ADDOPERATORRPC);
        GetViewerState().GetViewerRPC().SetOperatorType(oper);
        GetViewerState().GetViewerRPC().SetBoolFlag(fromDefault);
        GetViewerState().GetViewerRPC().Notify();
        return Synchronize();
    }

    /**
     * Adds an operator to the active plots.
     *
     * @param type Name of the operator to be added.
     * @return true on success; false otherwise.
     */
    public boolean AddOperator(String type)
    {
        int oper = proxy.GetOperatorIndex(type);
        GetViewerState().GetViewerRPC().SetRPCType(ViewerRPC.VIEWERRPCTYPE_ADDOPERATORRPC);
        GetViewerState().GetViewerRPC().SetOperatorType(oper);
        GetViewerState().GetViewerRPC().SetBoolFlag(true);
        GetViewerState().GetViewerRPC().Notify();
        return Synchronize();
    }

    /**
     * Adds an operator to the active plots.
     *
     * @param oper Type of the operator to be added, which is an index in the 
     *             operator plugin manager.
     * @return true on success; false otherwise.
     */
    public boolean AddInitializedOperator(int oper)
    {
        GetViewerState().GetViewerRPC().SetRPCType(ViewerRPC.VIEWERRPCTYPE_ADDINITIALIZEDOPERATORRPC);
        GetViewerState().GetViewerRPC().SetOperatorType(oper);
        GetViewerState().GetViewerRPC().Notify();
        return Synchronize();
    }

    /**
     * Promotes an operator within the list of operators applied to the active plots.
     *
     * @param operatorId The index of the operator to promote in the plot.
     * @return true on success; false otherwise.
     */
    public boolean PromoteOperator(int operatorId)
    {
        GetViewerState().GetViewerRPC().SetRPCType(ViewerRPC.VIEWERRPCTYPE_PROMOTEOPERATORRPC);
        GetViewerState().GetViewerRPC().SetOperatorType(operatorId);
        GetViewerState().GetViewerRPC().Notify();
        return Synchronize();
    }

    /**
     * Demotes an operator within the list of operators applied to the active plots.
     *
     * @param operatorId The index of the operator to demote in the plot.
     * @return true on success; false otherwise.
     */
    public boolean DemoteOperator(int operatorId)
    {
        GetViewerState().GetViewerRPC().SetRPCType(ViewerRPC.VIEWERRPCTYPE_DEMOTEOPERATORRPC);
        GetViewerState().GetViewerRPC().SetOperatorType(operatorId);
        GetViewerState().GetViewerRPC().Notify();
        return Synchronize();
    }

    /**
     * Removes the i'th operator from the operators applied to the active plots.
     *
     * @param operatorId The index of the operator to remove from the plot.
     * @return true on success; false otherwise.
     */
    public boolean RemoveOperator(int operatorId)
    {
        GetViewerState().GetViewerRPC().SetRPCType(ViewerRPC.VIEWERRPCTYPE_REMOVEOPERATORRPC);
        GetViewerState().GetViewerRPC().SetOperatorType(operatorId);
        GetViewerState().GetViewerRPC().Notify();
        return Synchronize();
    }

    /**
     * Removes last operator from the operators applied to the active plots.
     *
     * @return true on success; false otherwise.
     */
    public boolean RemoveLastOperator()
    {
        GetViewerState().GetViewerRPC().SetRPCType(ViewerRPC.VIEWERRPCTYPE_REMOVELASTOPERATORRPC);
        GetViewerState().GetViewerRPC().Notify();
        return Synchronize();
    }

    /**
     * Removes all operators from the operators applied to the active plots.
     *
     * @return true on success; false otherwise.
     */
    public boolean RemoveAllOperators()
    {
        GetViewerState().GetViewerRPC().SetRPCType(ViewerRPC.VIEWERRPCTYPE_REMOVEALLOPERATORSRPC);
        GetViewerState().GetViewerRPC().Notify();
        return Synchronize();
    }

    /**
     * Sets the default plot options for the specified plot type. Future instances
     * of that plot type will be initialized with the attributes that have been
     * set for it. The plot attributes state object must have been set before calling
     * this method.
     * 
     * Example:<br>
     *   <p class="example">
     *   int type = proxy.GetPlotIndex("Pseudocolor");<br>
     *   PseudocolorAttributes atts = (PseudocolorAttributes)proxy.GetPlotAttributes(type);<br>
     *   // Modify atts ... (not shown)<br>
     *   atts.Notify();<br>
     *   proxy.GetViewerMethods().SetDefaultPlotOptions(type);</p>
     *
     * @param type Type of the plot for which we're setting default options.
     * @return true on success; false otherwise.
     */
    public boolean SetDefaultPlotOptions(int type)
    {
        GetViewerState().GetViewerRPC().SetRPCType(ViewerRPC.VIEWERRPCTYPE_SETDEFAULTPLOTOPTIONSRPC);
        GetViewerState().GetViewerRPC().SetPlotType(type);
        GetViewerState().GetViewerRPC().Notify();
        return Synchronize();
    }

    /**
     * Sets the plot options for the specified plot type. The plot attributes state 
     * object must have been set before calling this method.
     * 
     * Example:<br>
     *   <p class="example">
     *   int type = proxy.GetPlotIndex("Pseudocolor");<br>
     *   PseudocolorAttributes atts = (PseudocolorAttributes)proxy.GetPlotAttributes(type);<br>
     *   // Modify atts ... (not shown)<br>
     *   atts.Notify();<br>
     *   proxy.GetViewerMethods().SetPlotOptions(type);</p>
     *
     * @param type Type of the plot for which we're setting default options (an index)
     * @return true on success; false otherwise.
     */
    public boolean SetPlotOptions(int type)
    {
        boolean retval = false;
        if(type >= 0)
        {
            GetViewerState().GetViewerRPC().SetRPCType(ViewerRPC.VIEWERRPCTYPE_SETPLOTOPTIONSRPC);
            GetViewerState().GetViewerRPC().SetPlotType(type);
            GetViewerState().GetViewerRPC().Notify();
            retval = Synchronize();
        }
        else
        {
            proxy.PrintMessage("SetPlotOptions: " + type + 
                               " is an invalid plot index.");
        }

        return retval;
    }

    /**
     * Sets the plot options for the specified plot type. The plot attributes state 
     * object must have been set before calling this method.
     * 
     * Example:<br>
     *   <p class="example">
     *   int type = proxy.GetPlotIndex("Pseudocolor");<br>
     *   PseudocolorAttributes atts = (PseudocolorAttributes)proxy.GetPlotAttributes(type);<br>
     *   // Modify atts ... (not shown)<br>
     *   atts.Notify();<br>
     *   proxy.GetViewerMethods().SetPlotOptions("Pseudocolor");</p>
     *
     * @param type Type of the plot for which we're setting default options.
     * @return true on success; false otherwise.
     */
    public boolean SetPlotOptions(String type)
    {
        return SetPlotOptions(proxy.GetPlotIndex(type));
    }

    /**
     * Resets the active plot's attributes to the default settings for the
     * plot's type.
     * 
     * @param type Type of the plot for which we're resetting plot options (an index)
     * @return true on success; false otherwise.
     */
    public boolean ResetPlotOptions(int type)
    {
        GetViewerState().GetViewerRPC().SetRPCType(ViewerRPC.VIEWERRPCTYPE_RESETPLOTOPTIONSRPC);
        GetViewerState().GetViewerRPC().SetPlotType(type);
        GetViewerState().GetViewerRPC().Notify();
        return Synchronize();
    }

    /**
     * Sets the default operator options for the specified operator type. Future instances
     * of that operator type will be initialized with the attributes that have been
     * set for it. The operator attributes state object must have been set before calling
     * this method.
     * 
     * Example:<br>
     *   <p class="example">
     *   int type = proxy.GetOperatorIndex("Slice");<br>
     *   SliceAttributes atts = (SliceAttributes)proxy.GetOperatorAttributes(type);<br>
     *   // Modify atts ... (not shown)<br>
     *   atts.Notify();<br>
     *   proxy.GetViewerMethods().SetDefaultOperatorOptions(type);</p>
     *
     * @param type Type of the operator for which we're setting default options.
     * @return true on success; false otherwise.
     */
    public boolean SetDefaultOperatorOptions(int type)
    {
        GetViewerState().GetViewerRPC().SetRPCType(ViewerRPC.VIEWERRPCTYPE_SETDEFAULTOPERATOROPTIONSRPC);
        GetViewerState().GetViewerRPC().SetOperatorType(type);
        GetViewerState().GetViewerRPC().Notify();
        return Synchronize();
    }

    /**
     * Sets the operator options for the specified operator type. The operator 
     * attributes state object must have been set before calling this method.
     * 
     * Example:<br>
     *   <p class="example">
     *   int type = proxy.GetOperatorIndex("Slice");<br>
     *   SliceAttributes atts = (SliceAttributes)proxy.GetOperatorAttributes(type);<br>
     *   // Modify atts ... (not shown)<br>
     *   atts.Notify();<br>
     *   proxy.GetViewerMethods().SetOperatorOptions(type);</p>
     *
     * @param type Type of the operator for which we're setting default options (an index)
     * @return true on success; false otherwise.
     */
    public boolean SetOperatorOptions(int type)
    {
        boolean retval = false;
        if(type >= 0)
        {
            GetViewerState().GetViewerRPC().SetRPCType(ViewerRPC.VIEWERRPCTYPE_SETOPERATOROPTIONSRPC);
            GetViewerState().GetViewerRPC().SetOperatorType(type);
            GetViewerState().GetViewerRPC().Notify();
            retval = Synchronize();
        }
        else
        {
            proxy.PrintMessage("SetOperatorOptions: " + type + 
                               " is an invalid operator index.");
        }

        return retval;
    }

    /**
     * Sets the operator options for the specified operator type. The operator 
     * attributes state object must have been set before calling this method.
     * 
     * Example:<br>
     *   <p class="example">
     *   int type = proxy.GetOperatorIndex("Slice");<br>
     *   SliceAttributes atts = (SliceAttributes)proxy.GetOperatorAttributes(type);<br>
     *   // Modify atts ... (not shown)<br>
     *   atts.Notify();<br>
     *   proxy.GetViewerMethods().SetOperatorOptions("Slice");</p>
     *
     * @param type Type of the operator for which we're setting default options.
     * @return true on success; false otherwise.
     */
    public boolean SetOperatorOptions(String type)
    {
        return SetOperatorOptions(proxy.GetOperatorIndex(type));
    }

    /**
     * Resets the operator attributes for the active plot to the default settings
     * for the operator's type.
     * 
     * @param type Type of the operator for which we're resetting operator options
     *             (an index)
     * @return true on success; false otherwise.
     */
    public boolean ResetOperatorOptions(int type)
    {
        GetViewerState().GetViewerRPC().SetRPCType(ViewerRPC.VIEWERRPCTYPE_RESETOPERATOROPTIONSRPC);
        GetViewerState().GetViewerRPC().SetOperatorType(type);
        GetViewerState().GetViewerRPC().Notify();
        return Synchronize();
    }

    /**
     * Tell VisIt to apply the animation attributes that were previously sent.
     *
     * Example:<br>
     *   <p class="example">
     *    AnimationAttributes atts = proxy.GetViewerState().GetAnimationAttributes();<br>
     *    // Modify the atts here... (not shown)<br>
     *    atts.Notify();<br>
     *    proxy.GetViewerMethods().SetAnimationAttributes();</p>
     *
     * @return true on success; false otherwise.
     */
    public boolean SetAnimationAttributes()
    {
        GetViewerState().GetViewerRPC().SetRPCType(ViewerRPC.VIEWERRPCTYPE_SETANIMATIONATTRIBUTESRPC);
        GetViewerState().GetViewerRPC().Notify();
        return Synchronize();
    }

    /**
     * Apply the annotation attributes that were previously sent. Annotation
     * attributes control properties for stock annotations such as the 2D/3D axes.
     *
     * Example:<br>
     *   <p class="example">
     *    AnnotationAttributes atts = proxy.GetViewerState().GetAnnotationAttributes();<br>
     *    // Modify the atts here... (not shown)<br>
     *    atts.Notify();<br>
     *    proxy.GetViewerMethods().SetAnnotationAttributes();</p>
     *
     * @return true on success; false otherwise.
     */
    public boolean SetAnnotationAttributes()
    {
        GetViewerState().GetViewerRPC().SetRPCType(ViewerRPC.VIEWERRPCTYPE_SETANNOTATIONATTRIBUTESRPC);
        GetViewerState().GetViewerRPC().Notify();
        return Synchronize();
    }

    /**
     * Tell VisIt to use the current annotation attributes as the default annotation
     * attributes, which will be the defaults for new visualization windows or the 
     * saved defaults when settings are saved.
     *
     * Example:<br>
     *   <p class="example">
     *    AnnotationAttributes atts = proxy.GetViewerState().GetAnnotationAttributes();<br>
     *    // Modify the atts here... (not shown)<br>
     *    atts.Notify();<br>
     *    proxy.GetViewerMethods().SetDefaultAnnotationAttributes();</p>
     *
     * @return true on success; false otherwise.
     */
    public boolean SetDefaultAnnotationAttributes()
    {
        GetViewerState().GetViewerRPC().SetRPCType(ViewerRPC.VIEWERRPCTYPE_SETDEFAULTANNOTATIONATTRIBUTESRPC);
        GetViewerState().GetViewerRPC().Notify();
        return Synchronize();
    }

    /**
     * Restore the annotation attributes to the default settings.
     *
     * @return true on success; false otherwise.
     */
    public boolean ResetAnnotationAttributes()
    {
        GetViewerState().GetViewerRPC().SetRPCType(ViewerRPC.VIEWERRPCTYPE_RESETANNOTATIONATTRIBUTESRPC);
        GetViewerState().GetViewerRPC().Notify();
        return Synchronize();
    }

    /**
     * Create a new annotation object and add it to the visualization.
     *
     * Example:<br>
     *   <p class="example">
     *    proxy.GetViewerMethods().AddAnnotationObject(AnnotationObject.ANNOTATIONTYPE_TIMESLIDER, "mySlider");<br>
     *   </p>
     *
     * @param annotType One of the annotation types in AnnotationObject.java
     * @param annotName  The name of the new annotation object instance.
     * @return true on success; false otherwise.
     */
    public boolean AddAnnotationObject(int annotType, String annotName)
    {
        GetViewerState().GetViewerRPC().SetRPCType(ViewerRPC.VIEWERRPCTYPE_ADDANNOTATIONOBJECTRPC);
        GetViewerState().GetViewerRPC().SetIntArg1(annotType);
        GetViewerState().GetViewerRPC().SetStringArg1(annotName);
        GetViewerState().GetViewerRPC().Notify();
        return Synchronize();
    }

    /**
     * Hides the selected annotation objects.
     *
     * @return true on success; false otherwise.
     */
    public boolean HideActiveAnnotationObjects()
    {
        GetViewerState().GetViewerRPC().SetRPCType(ViewerRPC.VIEWERRPCTYPE_HIDEACTIVEANNOTATIONOBJECTSRPC);
        GetViewerState().GetViewerRPC().Notify();
        return Synchronize();
    }

    /**
     * Deletes the selected annotation objects.
     *
     * @return true on success; false otherwise.
     */
    public boolean DeleteActiveAnnotationObjects()
    {
        GetViewerState().GetViewerRPC().SetRPCType(ViewerRPC.VIEWERRPCTYPE_DELETEACTIVEANNOTATIONOBJECTSRPC);
        GetViewerState().GetViewerRPC().Notify();
        return Synchronize();
    }

    /**
     * Raises the selected annotation objects above the others so they are drawn on top.
     *
     * @return true on success; false otherwise.
     */
    public boolean RaiseActiveAnnotationObjects()
    {
        GetViewerState().GetViewerRPC().SetRPCType(ViewerRPC.VIEWERRPCTYPE_RAISEACTIVEANNOTATIONOBJECTSRPC);
        GetViewerState().GetViewerRPC().Notify();
        return Synchronize();
    }

    /**
     * Lowers the selected annotation objects above the others so they are drawn below others.
     *
     * @return true on success; false otherwise.
     */
    public boolean LowerActiveAnnotationObjects()
    {
        GetViewerState().GetViewerRPC().SetRPCType(ViewerRPC.VIEWERRPCTYPE_LOWERACTIVEANNOTATIONOBJECTSRPC);
        GetViewerState().GetViewerRPC().Notify();
        return Synchronize();
    }

    /**
     * Tell VisIt to apply the annotation object attributes that were previously sent.
     * Annotation object options set properties for user-created annotations such as 
     * time sliders, text, etc.
     *
     * Example:<br>
     *   <p class="example">
     *    AnnotationObjectList atts = proxy.GetViewerState().GetAnnotationObjectList();<br>
     *    AnnotationObject obj = atts.GetAnnotation(0);
     *    // Modify obj here... (not shown)<br>
     *    atts.Notify();<br>
     *    proxy.GetViewerMethods().SetAnnotationObjectOptions();</p>
     *
     * @return true on success; false otherwise.
     */
    public boolean SetAnnotationObjectOptions()
    {
        GetViewerState().GetViewerRPC().SetRPCType(ViewerRPC.VIEWERRPCTYPE_SETANNOTATIONOBJECTOPTIONSRPC);
        GetViewerState().GetViewerRPC().Notify();
        return Synchronize();
    }

    /**
     * Tell VisIt to apply the annotation object attributes that were previously sent as 
     * the default annotation object list, which will be used for future visualization windows
     * and for saving settings.
     *
     * @return true on success; false otherwise.
     */
    public boolean SetDefaultAnnotationObjectList()
    {
        GetViewerState().GetViewerRPC().SetRPCType(ViewerRPC.VIEWERRPCTYPE_SETDEFAULTANNOTATIONOBJECTLISTRPC);
        GetViewerState().GetViewerRPC().Notify();
        return Synchronize();
    }

    /**
     * Restore the annotation object list to the default settings.
     *
     * @return true on success; false otherwise.
     */
    public boolean ResetAnnotationObjectList()
    {
        GetViewerState().GetViewerRPC().SetRPCType(ViewerRPC.VIEWERRPCTYPE_RESETANNOTATIONOBJECTLISTRPC);
        GetViewerState().GetViewerRPC().Notify();
        return Synchronize();
    }

    /**
     * Tell VisIt to apply the material attributes that were previously sent.
     * Material attributes control material interface reconstruction, which is 
     * the process of recreating material boundaries from material mass fractions
     * that some databases provide.
     *
     * Example:<br>
     *   <p class="example">
     *    MaterialAttributes atts = proxy.GetViewerState().GetMaterialAttributes();<br>
     *    // Modify the atts here... (not shown)<br>
     *    atts.Notify();<br>
     *    proxy.GetViewerMethods().SetMaterialAttributes();</p>
     *
     * @return true on success; false otherwise.
     */
    public boolean SetMaterialAttributes()
    {
        GetViewerState().GetViewerRPC().SetRPCType(ViewerRPC.VIEWERRPCTYPE_SETMATERIALATTRIBUTESRPC);
        GetViewerState().GetViewerRPC().Notify();
        return Synchronize();
    }

    /**
     * Tell VisIt to apply the material attributes that were previously sent as the
     * default material options, which will be used when saving settings.
     *
     * @return true on success; false otherwise.
     */
    public boolean SetDefaultMaterialAttributes()
    {
        GetViewerState().GetViewerRPC().SetRPCType(ViewerRPC.VIEWERRPCTYPE_SETDEFAULTMATERIALATTRIBUTESRPC);
        GetViewerState().GetViewerRPC().Notify();
        return Synchronize();
    }

    /**
     * Reset the material attributes to the default material attributes.
     *
     * @return true on success; false otherwise.
     */
    public boolean ResetMaterialAttributes()
    {
        GetViewerState().GetViewerRPC().SetRPCType(ViewerRPC.VIEWERRPCTYPE_RESETMATERIALATTRIBUTESRPC);
        GetViewerState().GetViewerRPC().Notify();
        return Synchronize();
    }

    /**
     * Tell VisIt to apply the mesh management attributes that were previously sent. 
     * Mesh management attributes control algorithms such as CSG discretization.
     *
     * Example:<br>
     *   <p class="example">
     *    MeshManagementAttributes atts = proxy.GetViewerState().GetMeshManagementAttributes();<br>
     *    // Modify the atts here... (not shown)<br>
     *    atts.Notify();<br>
     *    proxy.GetViewerMethods().SetMeshManagementAttributes();</p>
     *
     * @return true on success; false otherwise.
     */
    public boolean SetMeshManagementAttributes()
    {
        GetViewerState().GetViewerRPC().SetRPCType(ViewerRPC.VIEWERRPCTYPE_SETMESHMANAGEMENTATTRIBUTESRPC);
        GetViewerState().GetViewerRPC().Notify();
        return Synchronize();
    }

    /**
     * Tell VisIt to apply the mesh management attributes that were previously 
     * sent as the default mesh management options, which will be used when 
     * saving settings.
     *
     * @return true on success; false otherwise.
     */
    public boolean SetDefaultMeshManagementAttributes()
    {
        GetViewerState().GetViewerRPC().SetRPCType(ViewerRPC.VIEWERRPCTYPE_SETDEFAULTMESHMANAGEMENTATTRIBUTESRPC);
        GetViewerState().GetViewerRPC().Notify();
        return Synchronize();
    }

    /**
     * Reset the mesh management attributes to the default mesh management attributes.
     *
     * @return true on success; false otherwise.
     */
    public boolean ResetMeshManagementAttributes()
    {
        GetViewerState().GetViewerRPC().SetRPCType(ViewerRPC.VIEWERRPCTYPE_RESETMESHMANAGEMENTATTRIBUTESRPC);
        GetViewerState().GetViewerRPC().Notify();
        return Synchronize();
    }

    /**
     * Apply the keyframe attributes that were previously sent.
     *
     * Example:<br>
     *   <p class="example">
     *    KeyframeAttributes atts = proxy.GetViewerState().GetKeyframeAttributes();<br>
     *    // Modify the atts here... (not shown)<br>
     *    atts.Notify();<br>
     *    proxy.GetViewerMethods().SetKeyframeAttributes();</p>
     *
     * @return true on success; false otherwise.
     */
    public boolean SetKeyframeAttributes()
    {
        GetViewerState().GetViewerRPC().SetRPCType(ViewerRPC.VIEWERRPCTYPE_SETKEYFRAMEATTRIBUTESRPC);
        GetViewerState().GetViewerRPC().Notify();
        return Synchronize();
    }

    /**
     * Apply the light list that were previously sent. The light list controls
     * lights in VisIt. There can be up to 8 lights and each can have different
     * colors, types, and orientations. Multiple lights are often useful when
     * making a presentation-quality visualization since you can brighten up
     * areas of a visualization that are too dark and you can add more dramatic
     * lighting.
     *
     * Example:<br>
     *   <p class="example">
     *    LightList atts = proxy.GetViewerState().GetLightList();<br>
     *    // Modify the atts here... (not shown)<br>
     *    atts.Notify();<br>
     *    proxy.GetViewerMethods().SetLightList();</p>
     *
     * @return true on success; false otherwise.
     */
    public boolean SetLightList()
    {
        GetViewerState().GetViewerRPC().SetRPCType(ViewerRPC.VIEWERRPCTYPE_SETLIGHTLISTRPC);
        GetViewerState().GetViewerRPC().Notify();
        return Synchronize();
    }

    /**
     * Apply the light list that was previously sent as the default light list,
     * which will be used for new visualization windows and for when saving settings.
     *
     * @return true on success; false otherwise.
     */
    public boolean SetDefaultLightList()
    {
        GetViewerState().GetViewerRPC().SetRPCType(ViewerRPC.VIEWERRPCTYPE_SETDEFAULTLIGHTLISTRPC);
        GetViewerState().GetViewerRPC().Notify();
        return Synchronize();
    }

    /**
     * Reset the light list to the default light list.
     *
     * @return true on success; false otherwise.
     */
    public boolean ResetLightList()
    {
        GetViewerState().GetViewerRPC().SetRPCType(ViewerRPC.VIEWERRPCTYPE_RESETLIGHTLISTRPC);
        GetViewerState().GetViewerRPC().Notify();
        return Synchronize();
    }

    /**
     * Reset the pick letter so the next pick letter will be 'A'.
     *
     * @return true on success; false otherwise.
     */
    public boolean ResetPickLetter()
    {
        GetViewerState().GetViewerRPC().SetRPCType(ViewerRPC.VIEWERRPCTYPE_RESETPICKLETTERRPC);
        GetViewerState().GetViewerRPC().Notify();
        return Synchronize();
    }

    /**
     * Reset the pick attributes to the defaults.
     *
     * @return true on success; false otherwise.
     */
    public boolean ResetPickAttributes()
    {
        GetViewerState().GetViewerRPC().SetRPCType(ViewerRPC.VIEWERRPCTYPE_RESETPICKATTRIBUTESRPC);
        GetViewerState().GetViewerRPC().Notify();
        return Synchronize();
    }

    /**
     * Set the active continuous color table, which is the color table to be used
     * in plots such as Pseudocolor and Surface when their plot attributes are set
     * to use the "default" color table. A continuous color table is one that 
     * interpolates between the small set of colors that make up its definition to
     * create smooth transitions between colors.
     *
     * @param colorTableName The name of the color table to use as the default continuous
     *                       color table. The name must be present in the names stored
     *                       in the ColorTableAttributes state object.
     * @return true on success; false otherwise.
     */
    public boolean SetActiveContinuousColorTable(String colorTableName)
    {
        if(proxy.GetViewerState().GetColorTableAttributes().GetColorTableIndex(colorTableName) != -1)
        {
            proxy.GetViewerState().GetColorTableAttributes().SetActiveContinuous(colorTableName);
            proxy.GetViewerState().GetColorTableAttributes().Notify();

            // Update the color table. This has the effect of making all plots
            // use the default color table update to use the new active color
            // table.
            boolean sync = proxy.GetSynchronous();
            proxy.SetSynchronous(false);
            UpdateColorTable(colorTableName);
            proxy.SetSynchronous(sync);
        }

        return Synchronize();
    }

    /**
     * Set the active discrete color table, which is the color table to be used
     * in plots such as Subset and FilledBoundary when their plot attributes are set
     * to use the "default" color table. A discrete color table is a set of colors that
     * repeat mod the number of colors in the color table.
     *
     * @param colorTableName The name of the color table to use as the default discrete
     *                       color table. The name must be present in the names stored
     *                       in the ColorTableAttributes state object.
     * @return true on success; false otherwise.
     */
    public boolean SetActiveDiscreteColorTable(String colorTableName)
    {
        if(proxy.GetViewerState().GetColorTableAttributes().GetColorTableIndex(colorTableName) != -1)
        {
            proxy.GetViewerState().GetColorTableAttributes().SetActiveDiscrete(colorTableName);
            proxy.GetViewerState().GetColorTableAttributes().Notify();
        }

        return Synchronize();
    }

    /**
     * Delete the named color table.
     *
     * @param colorTableName The name of the color table to delete. The name must 
     *                       be present in the names stored in the ColorTableAttributes 
     *                       state object.
     * @return true on success; false otherwise.
     */
    public boolean DeleteColorTable(String colorTableName)
    {
        // If it's a valid color table name, make it active.
        int index = proxy.GetViewerState().GetColorTableAttributes().GetColorTableIndex(colorTableName);
        if(index != -1)
        {
            // Remove the color table from the list and update.
            proxy.GetViewerState().GetColorTableAttributes().RemoveColorTable(index);
            proxy.GetViewerState().GetColorTableAttributes().Notify();

            // Update the color table. The specified color table will no
            // longer exist in the list of color tables so all plots that used
            // that color table will have their color tables changed to something
            // else.
            boolean sync = proxy.GetSynchronous();
            proxy.SetSynchronous(false);
            UpdateColorTable(colorTableName);
            proxy.SetSynchronous(sync);
        }

        return Synchronize();
    }

    /**
     * Applies the ColorTableAttributes that were previously sent and updates any
     * plots that use the color table that was modified.
     *
     * Example:<br>
     *   <p class="example">
     *    ColorTableAttributes atts = proxy.GetViewerState().GetColorTableAttributes();<br>
     *    ColorControlPointList ct = atts.GetColorControlPoints("hot");
     *    // Modify the ct here... (not shown)<br>
     *    atts.SelectAll();
     *    atts.Notify();<br>
     *    proxy.GetViewerMethods().UpdateColorTable("hot");</p>
     *
     * @param colorTableName The name of the color table to update. The name must 
     *                       be present in the names stored in the ColorTableAttributes 
     *                       state object.
     * @return true on success; false otherwise.
     */
    public boolean UpdateColorTable(String colorTableName)
    {
        GetViewerState().GetViewerRPC().SetRPCType(ViewerRPC.VIEWERRPCTYPE_UPDATECOLORTABLERPC);
        GetViewerState().GetViewerRPC().SetColorTableName(colorTableName);
        GetViewerState().GetViewerRPC().Notify();
        return Synchronize();
    }

    /**
     * Exports the specified color table to a file in the user's .visit directory.
     * This function exists to make it easy for users to exchange color tables.
     * When color table (*.ct) files are located in the user's .visit directory,
     * the color tables are loaded when VisIt starts up.
     *
     * @param colorTableName The name of the color table to update. The name must 
     *                       be present in the names stored in the ColorTableAttributes 
     *                       state object.
     * @return true on success; false otherwise.
     */
    public boolean ExportColorTable(String colorTableName)
    {
        GetViewerState().GetViewerRPC().SetRPCType(ViewerRPC.VIEWERRPCTYPE_EXPORTCOLORTABLERPC);
        GetViewerState().GetViewerRPC().SetColorTableName(colorTableName);
        GetViewerState().GetViewerRPC().Notify();
        return Synchronize();
    }

    /**
     * Reverses the visualization window's foreground and background colors.
     *
     * @return true on success; false otherwise.
     */
    public boolean InvertBackgroundColor()
    {
        GetViewerState().GetViewerRPC().SetRPCType(ViewerRPC.VIEWERRPCTYPE_INVERTBACKGROUNDRPC);
        GetViewerState().GetViewerRPC().Notify();
        return Synchronize();
    }

    /**
     * Applies the default file opening options which were previously sent. The 
     * file opening options allow the user to provide options that are used
     * by database plugins when opening files.
     *
     * Example:<br>
     *   <p class="example">
     *    FileOpenOptions atts = proxy.GetViewerState().GetFileOpenOptions();<br>
     *    // Modify the atts here... (not shown)<br>
     *    atts.Notify();<br>
     *    proxy.GetViewerMethods().SetDefaultFileOptions();</p>
     *
     * @return true on success; false otherwise.
     */
    public boolean SetDefaultFileOptions()
    {
        GetViewerState().GetViewerRPC().SetRPCType(ViewerRPC.VIEWERRPCTYPE_SETDEFAULTFILEOPENOPTIONSRPC);
        GetViewerState().GetViewerRPC().Notify();
        return Synchronize();
    }

    /**
     * Saves the viewer's settings file, which is called ~/.visit/config and contains
     * default values for state objects.
     *
     * @return true on success; false otherwise.
     */
    public boolean WriteConfigFile()
    {
        GetViewerState().GetViewerRPC().SetRPCType(ViewerRPC.VIEWERRPCTYPE_WRITECONFIGFILERPC);
        GetViewerState().GetViewerRPC().Notify();
        return Synchronize();
    }

    /**
     * Saves a session file containing all of the default settings for the viewer as well
     * as all information needed to recreate the plots in all visualization windows in
     * future VisIt sessions.
     *
     * @param filename The name of the local file to which the session information will be saved.
     * @return true on success; false otherwise.
     */
    public boolean ExportEntireState(String filename)
    {
        GetViewerState().GetViewerRPC().SetRPCType(ViewerRPC.VIEWERRPCTYPE_EXPORTENTIRESTATERPC);
        GetViewerState().GetViewerRPC().SetVariable(filename);
        GetViewerState().GetViewerRPC().Notify();
        return Synchronize();
    }

    /**
     * Restores a session file, which causes VisIt to delete all plots and recreate the
     * plots described in the session file.
     *
     * @param filename The name of the local file from which the session information will be loaded.
     * @param inVisItDir true if the session file exists in ~/.visit/; otherwise false and the
     *                   filename must contain the entire path to the session file.
     * @return true on success; false otherwise.
     */
    public boolean ImportEntireState(String filename, boolean inVisItDir)
    {
        GetViewerState().GetViewerRPC().SetRPCType(ViewerRPC.VIEWERRPCTYPE_IMPORTENTIRESTATERPC);
        GetViewerState().GetViewerRPC().SetVariable(filename);
        GetViewerState().GetViewerRPC().SetBoolFlag(inVisItDir);
        GetViewerState().GetViewerRPC().Notify();
        return Synchronize();
    }

    /**
     * Restores a session file, which causes VisIt to delete all plots and recreate the
     * plots described in the session file but different databases will be used instead
     * of the databases saved in the session file.
     *
     * @param filename The name of the local file from which the session information will be loaded.
     * @param inVisItDir true if the session file exists in ~/.visit/; otherwise false and the
     *                   filename must contain the entire path to the session file.
     * @param sources A vector of String objects containing the names of the databases that will
     *                be used to restore the session. The vector must contain the same number of
     *                sources as the session file for the session to be restored properly.
     * @return true on success; false otherwise.
     */
    public boolean ImportEntireStateWithDifferentSources(String filename, boolean inVisItDir, Vector sources)
    {
        GetViewerState().GetViewerRPC().SetRPCType(ViewerRPC.VIEWERRPCTYPE_IMPORTENTIRESTATEWITHDIFFERENTSOURCESRPC);
        GetViewerState().GetViewerRPC().SetVariable(filename);
        GetViewerState().GetViewerRPC().SetBoolFlag(inVisItDir);
        GetViewerState().GetViewerRPC().SetProgramOptions(sources);
        GetViewerState().GetViewerRPC().Notify();
        return Synchronize();
    }

    /**
     * Sets the center of rotation, which is the point about which VisIt rotates 
     * plots when you interactively rotate using the mouse. This option is especially
     * useful when you have zoomed into a small area and want to rotate the plots.
     *
     * @param x The x location of the new center of rotation.
     * @param y The y location of the new center of rotation.
     * @param z The z location of the new center of rotation.
     * @return true on success; false otherwise.
     */
    public boolean SetCenterOfRotation(double x, double y, double z)
    {
        GetViewerState().GetViewerRPC().SetRPCType(ViewerRPC.VIEWERRPCTYPE_SETCENTEROFROTATIONRPC);
        double[] pt = new double[3];
        pt[0] = x;
        pt[1] = y;
        pt[2] = z;
        GetViewerState().GetViewerRPC().SetQueryPoint1(pt);
        GetViewerState().GetViewerRPC().Notify();
        return Synchronize();
    }

    /**
     * Lets the user interactively choose a center of rotation using pick. The center 
     * of rotation is the point about which VisIt rotates plots when you interactively 
     * rotate using the mouse
     *
     * @return true on success; false otherwise.
     */
    public boolean ChooseCenterOfRotation()
    {
        GetViewerState().GetViewerRPC().SetRPCType(ViewerRPC.VIEWERRPCTYPE_CHOOSECENTEROFROTATIONRPC);
        GetViewerState().GetViewerRPC().SetBoolFlag(false);
        GetViewerState().GetViewerRPC().Notify();
        return Synchronize();
    }

    /**
     * Sets the center of rotation as the 3D point that is under the 2D screen 
     * coordinate that specifies a location within the visualization window.
     *
     * @param sx The x location in the visualization window
     * @param sy The y location in the visualization window
     * @return true on success; false otherwise.
     */
    public boolean ChooseCenterOfRotation(double sx, double sy)
    {
        GetViewerState().GetViewerRPC().SetRPCType(ViewerRPC.VIEWERRPCTYPE_CHOOSECENTEROFROTATIONRPC);
        GetViewerState().GetViewerRPC().SetBoolFlag(true);
        double[] pt = new double[3];
        pt[0] = sx;
        pt[1] = sy;
        pt[2] = 0.;
        GetViewerState().GetViewerRPC().SetQueryPoint1(pt);
        GetViewerState().GetViewerRPC().Notify();
        return Synchronize();
    }

    /**
     * Applies the view axis array view object which was previously sent. This
     * type of view is strictly for controlling the view in the 
     * ParallelCoordinates plot.
     *
     * Example:<br>
     *   <p class="example">
     *    ViewAxisAttributes atts = proxy.GetViewerState().GetViewAxisAttributes();<br>
     *    // Modify the atts here... (not shown)<br>
     *    atts.Notify();<br>
     *    proxy.GetViewerMethods().SetViewAxisArray();</p>
     *
     * @return true on success; false otherwise.
     */
    public boolean SetViewAxisArray()
    {
        GetViewerState().GetViewerRPC().SetRPCType(ViewerRPC.VIEWERRPCTYPE_SETVIEWAXISARRAYRPC);
        GetViewerState().GetViewerRPC().Notify();
        return Synchronize();
    }

    /**
     * Applies the curve view object which was previously sent. The ViewCurveAttributes
     * object controls the view for Curve plots.
     *
     * Example:<br>
     *   <p class="example">
     *    ViewCurveAttributes atts = proxy.GetViewerState().GetViewCurveAttributes();<br>
     *    // Modify the atts here... (not shown)<br>
     *    atts.Notify();<br>
     *    proxy.GetViewerMethods().SetViewCurve();</p>
     *
     * @return true on success; false otherwise.
     */
    public boolean SetViewCurve()
    {
        GetViewerState().GetViewerRPC().SetRPCType(ViewerRPC.VIEWERRPCTYPE_SETVIEWCURVERPC);
        GetViewerState().GetViewerRPC().Notify();
        return Synchronize();
    }

    /**
     * Applies the 2D view object which was previously sent. The View2DAttributes
     * object controls the view for all 2D plots in a visualization window.
     *
     * Example:<br>
     *   <p class="example">
     *    View2DAttributes atts = proxy.GetViewerState().GetView2DAttributes();<br>
     *    // Modify the atts here... (not shown)<br>
     *    atts.Notify();<br>
     *    proxy.GetViewerMethods().SetView2D();</p>
     *
     * @return true on success; false otherwise.
     */
    public boolean SetView2D()
    {
        GetViewerState().GetViewerRPC().SetRPCType(ViewerRPC.VIEWERRPCTYPE_SETVIEW2DRPC);
        GetViewerState().GetViewerRPC().Notify();
        return Synchronize();
    }

    /**
     * Applies the 3D view object which was previously sent. The View2DAttributes
     * object controls the view for all 3D plots in a visualization window.
     *
     * Example:<br>
     *   <p class="example">
     *    View3DAttributes atts = proxy.GetViewerState().GetView3DAttributes();<br>
     *    // Modify the atts here... (not shown)<br>
     *    atts.Notify();<br>
     *    proxy.GetViewerMethods().SetView3D();</p>
     *
     * @return true on success; false otherwise.
     */
    public boolean SetView3D()
    {
        GetViewerState().GetViewerRPC().SetRPCType(ViewerRPC.VIEWERRPCTYPE_SETVIEW3DRPC);
        GetViewerState().GetViewerRPC().Notify();
        return Synchronize();
    }

    public boolean ClearViewKeyframes()
    {
        GetViewerState().GetViewerRPC().SetRPCType(ViewerRPC.VIEWERRPCTYPE_CLEARVIEWKEYFRAMESRPC);
        GetViewerState().GetViewerRPC().Notify();
        return Synchronize();
    }

    public boolean DeleteViewKeyframe(int frame)
    {
        GetViewerState().GetViewerRPC().SetRPCType(ViewerRPC.VIEWERRPCTYPE_DELETEVIEWKEYFRAMERPC);
        GetViewerState().GetViewerRPC().SetFrame(frame);
        GetViewerState().GetViewerRPC().Notify();
        return Synchronize();
    }

    public boolean MoveViewKeyframe(int oldFrame, int newFrame)
    {
        GetViewerState().GetViewerRPC().SetRPCType(ViewerRPC.VIEWERRPCTYPE_MOVEVIEWKEYFRAMERPC);
        GetViewerState().GetViewerRPC().SetIntArg1(oldFrame);
        GetViewerState().GetViewerRPC().SetIntArg2(newFrame);
        GetViewerState().GetViewerRPC().Notify();
        return Synchronize();
    }

    public boolean SetViewKeyframe()
    {
        GetViewerState().GetViewerRPC().SetRPCType(ViewerRPC.VIEWERRPCTYPE_SETVIEWKEYFRAMERPC);
        GetViewerState().GetViewerRPC().Notify();
        return Synchronize();
    }

    /**
     * Recalculates a suitable default view for the visualization window and 
     * applies that view.
     *
     * @return true on success; false otherwise.
     */
    public boolean ResetView()
    {
        GetViewerState().GetViewerRPC().SetRPCType(ViewerRPC.VIEWERRPCTYPE_RESETVIEWRPC);
        GetViewerState().GetViewerRPC().Notify();
        return Synchronize();
    }

    /**
     * Recenters the objects so they fit in the visualization window.
     *
     * @return true on success; false otherwise.
     */
    public boolean RecenterView()
    {
        GetViewerState().GetViewerRPC().SetRPCType(ViewerRPC.VIEWERRPCTYPE_RECENTERVIEWRPC);
        GetViewerState().GetViewerRPC().Notify();
        return Synchronize();
    }

    /**
     * Sets whether the view is calculated using actual or original spatial extents.
     *
     * @param t 0=original extents, 1=actual extents
     * @return true on success; false otherwise.
     */
    public boolean SetViewExtentsType(int t)
    {
        GetViewerState().GetViewerRPC().SetRPCType(ViewerRPC.VIEWERRPCTYPE_SETVIEWEXTENTSTYPERPC);
        GetViewerState().GetViewerRPC().SetWindowLayout(t);
        GetViewerState().GetViewerRPC().Notify();
        return Synchronize();
    }

    /**
     * Toggles the maintain view mode, which ensures that the view remains constant (when turned on).
     *
     * @return true on success; false otherwise.
     */
    public boolean ToggleMaintainViewMode()
    {
        GetViewerState().GetViewerRPC().SetRPCType(ViewerRPC.VIEWERRPCTYPE_TOGGLEMAINTAINVIEWMODERPC);
        GetViewerState().GetViewerRPC().Notify();
        return Synchronize();
    }

    /**
     * Toggles the maintain data mode, which ensures that data limits such as those 
     * used in the Pseudocolor plot remain set to the values that were in force when
     * maintain data mode was engaged.
     *
     * @return true on success; false otherwise.
     */
    public boolean ToggleMaintainDataMode()
    {
        GetViewerState().GetViewerRPC().SetRPCType(ViewerRPC.VIEWERRPCTYPE_TOGGLEMAINTAINDATAMODERPC);
        GetViewerState().GetViewerRPC().Notify();
        return Synchronize();
    }

    /**
     * Toggles full frame mode, which is a 1D/2D window mode that exagerrates scale 
     * in one or more dimensions to ensure that plots take up most of the visualization
     * window viewport.
     *
     * @return true on success; false otherwise.
     */
    public boolean ToggleFullFrameMode()
    {
        GetViewerState().GetViewerRPC().SetRPCType(ViewerRPC.VIEWERRPCTYPE_TOGGLEFULLFRAMERPC);
        GetViewerState().GetViewerRPC().Notify();
        return Synchronize();
    }

    /**
     * Undo the last view change, returning to the previous view.
     *
     * @return true on success; false otherwise.
     */
    public boolean UndoView()
    {
        GetViewerState().GetViewerRPC().SetRPCType(ViewerRPC.VIEWERRPCTYPE_UNDOVIEWRPC);
        GetViewerState().GetViewerRPC().Notify();
        return Synchronize();
    }

    /**
     * Redo the last view undo.
     *
     * @return true on success; false otherwise.
     */
    public boolean RedoView()
    {
        GetViewerState().GetViewerRPC().SetRPCType(ViewerRPC.VIEWERRPCTYPE_REDOVIEWRPC);
        GetViewerState().GetViewerRPC().Notify();
        return Synchronize();
    }

    /**
     * Toggles the lock view mode. When visualization windows have their views locked,
     * changes to the view in one window are also made in other visualization windows
     * whose plot dimensions are equal to the window that originated the view change.
     *
     * @return true on success; false otherwise.
     */
    public boolean ToggleLockViewMode()
    {
        GetViewerState().GetViewerRPC().SetRPCType(ViewerRPC.VIEWERRPCTYPE_TOGGLELOCKVIEWMODERPC);
        GetViewerState().GetViewerRPC().Notify();
        return Synchronize();
    }

    /**
     * Toggles the lock time mode. When visualization windows have their time sliders
     * locked, changing the time slider in one window will cause other time-locked 
     * windows that have compatible database correlations to also update in time. 
     *
     * @return true on success; false otherwise.
     */
    public boolean ToggleLockTime()
    {
        GetViewerState().GetViewerRPC().SetRPCType(ViewerRPC.VIEWERRPCTYPE_TOGGLELOCKTIMERPC);
        GetViewerState().GetViewerRPC().Notify();
        return Synchronize();
    }

    /**
     * Turns off all forms of locking in all windows.
     *
     * @return true on success; false otherwise.
     */
    public boolean TurnOffAllLocks()
    {
        GetViewerState().GetViewerRPC().SetRPCType(ViewerRPC.VIEWERRPCTYPE_TURNOFFALLLOCKSRPC);
        GetViewerState().GetViewerRPC().Notify();
        return Synchronize();    
    }

    /**
     * Toggles the lock tools mode. When visualization windows have their tools
     * locked, interacting with a tool in one window will cause the tool attributes
     * to be applied to other windows whose tools are locked. This setting is used
     * for operations such as moving a 3D plane tool in one window to set the Slice
     * operator attributes in another visualization window.
     *
     * @return true on success; false otherwise.
     */
    public boolean ToggleLockTools()
    {
        GetViewerState().GetViewerRPC().SetRPCType(ViewerRPC.VIEWERRPCTYPE_TOGGLELOCKTOOLSRPC);
        GetViewerState().GetViewerRPC().Notify();
        return Synchronize();
    }

    /**
     * Toggles spin mode in the active visualization window. Spin mode causes plots to
     * keep spinning when you rotate them interactively.
     *
     * @return true on success; false otherwise.
     */
    public boolean ToggleSpinMode()
    {
        GetViewerState().GetViewerRPC().SetRPCType(ViewerRPC.VIEWERRPCTYPE_TOGGLESPINMODERPC);
        GetViewerState().GetViewerRPC().Notify();
        return Synchronize();
    }

    /**
     * Toggles camera view mode, which indicates that the camera should be copied 
     * into the view during keyframe animations.
     *
     * @return true on success; false otherwise.
     */
    public boolean ToggleCameraViewMode()
    {
        GetViewerState().GetViewerRPC().SetRPCType(ViewerRPC.VIEWERRPCTYPE_TOGGLECAMERAVIEWMODERPC);
        GetViewerState().GetViewerRPC().Notify();
        return Synchronize();
    }

    /**
     * Sets the window mode for the active visualization window.
     *
     * @param mode 0=navigate, 1=zone pick, 2=node pick, 3=zoom, 4=lineout
     * @return true on success; false otherwise.
     */
    public boolean SetWindowMode(int mode)
    {
        GetViewerState().GetViewerRPC().SetRPCType(ViewerRPC.VIEWERRPCTYPE_SETWINDOWMODERPC);
        GetViewerState().GetViewerRPC().SetWindowMode(mode);
        GetViewerState().GetViewerRPC().Notify();
        return Synchronize();
    }

    /**
     * Toggles bounding box mode for the active visualization window. When a window is in
     * bounding box mode, its plots are replaced with a bounding box during rotation
     * so rotations are much faster. This option is helpful with scalable rendering.
     *
     * @return true on success; false otherwise.
     */
    public boolean ToggleBoundingBoxMode()
    {
        GetViewerState().GetViewerRPC().SetRPCType(ViewerRPC.VIEWERRPCTYPE_TOGGLEBOUNDINGBOXMODERPC);
        GetViewerState().GetViewerRPC().Notify();
        return Synchronize();
    }

    /**
     * Enables or disables an interactive tool in the active visualization window.
     *
     * @param tool 0=Box, 1=Line, 2=Plane, 3=Sphere, 4=Point, 5=Extents, 6=Axis restriction
     * @param enabled true to enable the tool; false to disable.
     * @return true on success; false otherwise.
     */
    public boolean EnableTool(int tool, boolean enabled)
    {
        GetViewerState().GetViewerRPC().SetRPCType(ViewerRPC.VIEWERRPCTYPE_ENABLETOOLRPC);
        GetViewerState().GetViewerRPC().SetToolId(tool);
        GetViewerState().GetViewerRPC().SetBoolFlag(enabled);
        GetViewerState().GetViewerRPC().Notify();
        return Synchronize();
    }

    /**
     * Copies the view from one visualization window to another.
     *
     * @param from Source visualization window (>=1).
     * @param to   Destination visualization window (>=1), (from != to)
     * @return true on success; false otherwise.
     */
    public boolean CopyViewToWindow(int from, int to)
    {
        GetViewerState().GetViewerRPC().SetRPCType(ViewerRPC.VIEWERRPCTYPE_COPYVIEWTOWINDOWRPC);
        GetViewerState().GetViewerRPC().SetWindowLayout(from);
        GetViewerState().GetViewerRPC().SetWindowId(to);
        GetViewerState().GetViewerRPC().Notify();
        return Synchronize();
    }

    /**
     * Copies the lighting from one visualization window to another.
     *
     * @param from Source visualization window (>=1).
     * @param to   Destination visualization window (>=1), (from != to)
     * @return true on success; false otherwise.
     */
    public boolean CopyLightingToWindow(int from, int to)
    {
        GetViewerState().GetViewerRPC().SetRPCType(ViewerRPC.VIEWERRPCTYPE_COPYLIGHTINGTOWINDOWRPC);
        GetViewerState().GetViewerRPC().SetWindowLayout(from);
        GetViewerState().GetViewerRPC().SetWindowId(to);
        GetViewerState().GetViewerRPC().Notify();
        return Synchronize();
    }

    /**
     * Copies the annotations from one visualization window to another.
     *
     * @param from Source visualization window (>=1).
     * @param to   Destination visualization window (>=1), (from != to)
     * @return true on success; false otherwise.
     */
    public boolean CopyAnnotationsToWindow(int from, int to)
    {
        GetViewerState().GetViewerRPC().SetRPCType(ViewerRPC.VIEWERRPCTYPE_COPYANNOTATIONSTOWINDOWRPC);
        GetViewerState().GetViewerRPC().SetWindowLayout(from);
        GetViewerState().GetViewerRPC().SetWindowId(to);
        GetViewerState().GetViewerRPC().Notify();
        return Synchronize();
    }

    /**
     * Copies the plots from one visualization window to another.
     *
     * @param from Source visualization window (>=1).
     * @param to   Destination visualization window (>=1), (from != to)
     * @return true on success; false otherwise.
     */
    public boolean CopyPlotsToWindow(int from, int to)
    {
        GetViewerState().GetViewerRPC().SetRPCType(ViewerRPC.VIEWERRPCTYPE_COPYPLOTSTOWINDOWRPC);
        GetViewerState().GetViewerRPC().SetWindowLayout(from);
        GetViewerState().GetViewerRPC().SetWindowId(to);
        GetViewerState().GetViewerRPC().Notify();
        return Synchronize();
    }

    /**
     * Sets the SIL restriction that was previously sent. A SIL is a data structure 
     * that describes how a dataset is put together, with sets as leaves, A SIL restriction
     * is a SIL that is augmented with a list of boolean values indicating which sets 
     * are on and which are off. VisIt uses the SIL restriction to turn off domains, 
     * materials, etc. Note -- The SIL restriction would have to be sent to the viewer
     * in its compacted form since no avtSILRestriction object yet exists in the Java API.
     *
     * @return true on success; false otherwise.
     */
    public boolean SetPlotSILRestriction()
    {
        GetViewerState().GetViewerRPC().SetRPCType(ViewerRPC.VIEWERRPCTYPE_SETPLOTSILRESTRICTIONRPC);
        GetViewerState().GetViewerRPC().Notify();
        return Synchronize();
    }

    /**
     * Applies the expression list that was previously sent. This method is called
     * after you make changes to the expression list, which is a list of expression
     * definitions that VisIt can use for plotting.
     *
     * <br>Examples:<br>
     *   <p class="example">
     *    ExpressionList atts = proxy.GetViewerState().GetExpressionList();<br>
     *    // Modify the atts here... (not shown)<br>
     *    atts.Notify();<br>
     *    proxy.GetViewerMethods().ProcessExpressions();</p>
     * @return true on success; false otherwise.
     */
    public boolean ProcessExpressions()
    {
        GetViewerState().GetViewerRPC().SetRPCType(ViewerRPC.VIEWERRPCTYPE_PROCESSEXPRESSIONSRPC);
        GetViewerState().GetViewerRPC().Notify();
        return Synchronize();
    }

    /**
     * Applies the appearance attributes that were previously sent. This method 
     * is called after you make changes to the appearance attributes, an object
     * that allows you to set GUI properties such as color and font.
     *
     * <br>Examples:<br>
     *   <p class="example">
     *    AppearanceAttributes atts = proxy.GetViewerState().GetAppearanceAttributes();<br>
     *    // Modify the atts here... (not shown)<br>
     *    atts.Notify();<br>
     *    proxy.GetViewerMethods().SetAppearanceAttributes();</p>
     * @return true on success; false otherwise.
     */
    public boolean SetAppearanceAttributes()
    {
        GetViewerState().GetViewerRPC().SetRPCType(ViewerRPC.VIEWERRPCTYPE_SETAPPEARANCERPC);
        GetViewerState().GetViewerRPC().Notify();
        return Synchronize();
    }

    /**
     * Clear all pick points from the visualization window.
     *
     * @return true on success; false otherwise.
     */
    public boolean ClearPickPoints()
    {
        GetViewerState().GetViewerRPC().SetRPCType(ViewerRPC.VIEWERRPCTYPE_CLEARPICKPOINTSRPC);
        GetViewerState().GetViewerRPC().Notify();
        return Synchronize();
    }

    /**
     * Clear all reference lines from the visualization window.
     *
     * @return true on success; false otherwise.
     */
    public boolean ClearReferenceLines()
    {
        GetViewerState().GetViewerRPC().SetRPCType(ViewerRPC.VIEWERRPCTYPE_CLEARREFLINESRPC);
        GetViewerState().GetViewerRPC().Notify();
        return Synchronize();
    }

    /**
     * Applies the rendering attributes that were previously sent.
     *
     * <br>Examples:<br>
     *   <p class="example">
     *    RenderingAttributes atts = proxy.GetViewerState().GetRenderingAttributes();<br>
     *    // Modify the atts here... (not shown)<br>
     *    atts.Notify();<br>
     *    proxy.GetViewerMethods().SetRenderingAttributes();</p>
     * @return true on success; false otherwise.
     */
    public boolean SetRenderingAttributes()
    {
        GetViewerState().GetViewerRPC().SetRPCType(ViewerRPC.VIEWERRPCTYPE_SETRENDERINGATTRIBUTESRPC);
        GetViewerState().GetViewerRPC().Notify();
        return Synchronize();
    }

    /**
     * Set the area of the screen that will be used for visualization windows.
     * Note - the size of visualization windows in the window area is determined
     * by VisIt and the window layout. Use ResizeWindow if you want to resize the
     * visualization windows yourself.
     *
     * @param x The x location of the upper left corner of the window area.
     * @param y The y location of the upper left corner of the window area.
     * @param w The width of the window area.
     * @param h The height of the window area.
     * @return true on success; false otherwise.
     */
    public void SetWindowArea(int x, int y, int w, int h)
    {
        String tmp = new String(x + "x" + y + "+" + x + "+" + y);
        GetViewerState().GetViewerRPC().SetRPCType(ViewerRPC.VIEWERRPCTYPE_SETWINDOWAREARPC);
        GetViewerState().GetViewerRPC().SetWindowArea(tmp);
        GetViewerState().GetViewerRPC().Notify();
    }

    /**
     * Performs a database query
     *
     * @param queryName The name of the query to perform, which must be present
     *                  in the QueryList state object.
     * @param vars The variables to involve in the query.
     * @return true on success; false otherwise.
     */
    public boolean DatabaseQuery(String queryName, Vector vars, boolean bflag, 
        int arg1, int arg2, boolean globalFlag, Vector darg1, Vector darg2)
    {
        GetViewerState().GetViewerRPC().SetRPCType(ViewerRPC.VIEWERRPCTYPE_DATABASEQUERYRPC);
        GetViewerState().GetViewerRPC().SetQueryName(queryName);
        GetViewerState().GetViewerRPC().SetQueryVariables(vars);
        GetViewerState().GetViewerRPC().SetIntArg1(arg1);
        GetViewerState().GetViewerRPC().SetIntArg2(arg2);
        GetViewerState().GetViewerRPC().SetBoolFlag(bflag);
        GetViewerState().GetViewerRPC().SetIntArg3(globalFlag?1:0);
        GetViewerState().GetViewerRPC().SetDoubleArg1(darg1);
        GetViewerState().GetViewerRPC().SetDoubleArg2(darg2);
        GetViewerState().GetViewerRPC().Notify();
        return Synchronize();
    }

    /**
     * Performs a database query
     *
     * @param queryName The name of the query to perform, which must be present
     *                  in the QueryList state object.
     * @param vars The variables to involve in the query.
     * @return true on success; false otherwise.
     */
    public boolean DatabaseQuery(String queryName, Vector vars)
    {
        return DatabaseQuery(queryName, vars, false, 0, 0, false, new Vector(), new Vector());
    }

    /**
     * Performs a point-based query such as pick.
     *
     * @return true on success; false otherwise.
     */
    public boolean PointQuery(String queryName, double[] pt, Vector vars, boolean time, 
        int arg1, int arg2, boolean globalFlag)
    {
        GetViewerState().GetViewerRPC().SetRPCType(ViewerRPC.VIEWERRPCTYPE_POINTQUERYRPC);
        GetViewerState().GetViewerRPC().SetQueryName(queryName);
        GetViewerState().GetViewerRPC().SetQueryPoint1(pt);
        GetViewerState().GetViewerRPC().SetQueryVariables(vars);
        GetViewerState().GetViewerRPC().SetBoolFlag(time);
        GetViewerState().GetViewerRPC().SetIntArg1(arg1);
        GetViewerState().GetViewerRPC().SetIntArg2(arg2);
        GetViewerState().GetViewerRPC().SetIntArg3(globalFlag?1:0);
        GetViewerState().GetViewerRPC().Notify();
        return Synchronize();
    }

    /**
     * Performs a point-based query such as pick.
     *
     * @return true on success; false otherwise.
     */
    public boolean PointQuery(String queryName, double[] pt, Vector vars)
    {
        return PointQuery(queryName, pt, vars, false, -1, -1, false);
    }

    /**
     * Performs a line-based query such as lineout.
     *
     * @return true on success; false otherwise.
     */
    public boolean LineQuery(String queryName, double[] pt1, double[] pt2,
        Vector vars, int samples, boolean forceSampling)
    {
        GetViewerState().GetViewerRPC().SetRPCType(ViewerRPC.VIEWERRPCTYPE_LINEQUERYRPC);
        GetViewerState().GetViewerRPC().SetQueryName(queryName);
        GetViewerState().GetViewerRPC().SetQueryPoint1(pt1);
        GetViewerState().GetViewerRPC().SetQueryPoint2(pt2);
        GetViewerState().GetViewerRPC().SetQueryVariables(vars);
        GetViewerState().GetViewerRPC().SetIntArg1(samples);
        GetViewerState().GetViewerRPC().SetBoolFlag(forceSampling);
        GetViewerState().GetViewerRPC().Notify();
        return Synchronize();
    }

    /**
     * Performs a line-based query such as lineout.
     *
     * @return true on success; false otherwise.
     */
    public boolean LineQuery(String queryName, double[] pt1, double[] pt2,
        Vector vars, int samples)
    {
        return LineQuery(queryName, pt1, pt2, vars, samples, false);
    }

    /**
     * Performs a zone pick of the active plot based on window screen coordinates.
     *
     * @param x The x screen coordinate of the pick point.
     * @param y The y screen coordinate of the pick point.
     * @param vars The variables for which to return pick information.
     * @return true on success; false otherwise.
     */
    public boolean Pick(int x, int y, Vector vars)
    {
        double[] pt = new double[3];
        pt[0] = (double)x;
        pt[1] = (double)y;
        pt[2] = 0.;
        return PointQuery("ScreenZonePick", pt, vars);
    }

    /**
     * Performs a zone pick of the active plot based on a 3D coordinate.
     *
     * @param xyz The 3D location of the pick point
     * @param vars The variables for which to return pick information.
     * @return true on success; false otherwise.
     */
    public boolean Pick(double[] xyz, Vector vars)
    {
        return PointQuery("Pick", xyz, vars);
    }

    /**
     * Performs a node pick of the active plot based on window screen coordinates.
     *
     * @param x The x screen coordinate of the pick point.
     * @param y The y screen coordinate of the pick point.
     * @param vars The variables for which to return pick information.
     * @return true on success; false otherwise.
     */
    public boolean NodePick(int x, int y, Vector vars)
    {
        double[] pt = new double[3];
        pt[0] = (double)x;
        pt[1] = (double)y;
        pt[2] = 0.;
        return PointQuery("ScreenNodePick", pt, vars);
    }

    /**
     * Performs a node pick of the active plot based on a 3D coordinate.
     *
     * @param xyz The 3D location of the pick point
     * @param vars The variables for which to return pick information.
     * @return true on success; false otherwise.
     */
    public boolean NodePick(double[] xyz, Vector vars)
    {
        return PointQuery("NodePick", xyz, vars);
    }

    /**
     * Performs a lineout between the specified endpoints, creating Curve plots
     * of the resulting data.
     *
     * @param p0 The 3D location of the start of the lineout.
     * @param p1 The 3D location of the end of the lineout.
     * @param vars The variables for which to create lineouts.
     * @param samples The number of samples to extract along the line.
     * @param forceSampling true to use sampling; false to do piecewise intersections.
     * @return true on success; false otherwise.
     */
    public boolean Lineout(double[] p0, double[] p1, Vector vars, 
        int samples, boolean forceSampling)
    {
        return LineQuery("Lineout", p0, p1, vars, samples, forceSampling);
    }

    /**
     * Performs a lineout between the specified endpoints, creating Curve plots
     * of the resulting data.
     *
     * @param p0 The 3D location of the start of the lineout.
     * @param p1 The 3D location of the end of the lineout.
     * @param vars The variables for which to create lineouts.
     * @param samples The number of samples to extract along the line.
     * @return true on success; false otherwise.
     */
    public boolean Lineout(double[] p0, double[] p1, Vector vars, int samples)
    {
        return LineQuery("Lineout", p0, p1, vars, samples, false);
    }

    /**
     * Performs a lineout between the specified endpoints, creating Curve plots
     * of the resulting data.
     *
     * @param x0 The starting x location.
     * @param y0 The starting y location.
     * @param x1 The ending x location.
     * @param y1 The ending y location.
     * @param vars The variables for which to create lineouts.
     * @return true on success; false otherwise.
     */
    public boolean Lineout(double x0, double y0, double x1, double y1, Vector vars)
    {
        double[] pt1 = new double[3];
        pt1[0] = x0;
        pt1[1] = y0;
        pt1[2] = 0.;
        double[] pt2 = new double[3];
        pt2[0] = x1;
        pt2[1] = y1;
        pt2[2] = 0.;
        return LineQuery("Lineout", pt1, pt2, vars, 100, false);
    }

    /**
     * Applies the global lineout attributes that were previously sent.
     *
     * <br>Examples:<br>
     *   <p class="example">
     *    GlobalLineoutAttributes atts = proxy.GetViewerState().GetGlobalLineoutAttributes();<br>
     *    // Modify the atts here... (not shown)<br>
     *    atts.Notify();<br>
     *    proxy.GetViewerMethods().SetGlobalLineoutAttributes();</p>
     * @return true on success; false otherwise.
     */
    public boolean SetGlobalLineoutAttributes()
    {
        GetViewerState().GetViewerRPC().SetRPCType(ViewerRPC.VIEWERRPCTYPE_SETGLOBALLINEOUTATTRIBUTESRPC);
        GetViewerState().GetViewerRPC().Notify();
        return Synchronize();
    }

    /**
     * Makes the pick attributes be the default pick attributes to be used for
     * new visualization windows and in the saved settings.
     *
     * @return true on success; false otherwise.
     */
    public boolean SetDefaultPickAttributes()
    {
        GetViewerState().GetViewerRPC().SetRPCType(ViewerRPC.VIEWERRPCTYPE_SETDEFAULTPICKATTRIBUTESRPC);
        GetViewerState().GetViewerRPC().Notify();
        return Synchronize();
    }

    /**
     * Applies the pick attributes that were previously sent. The pick attributes
     * mainly control the formatting options for VisIt's pick capability.
     *
     * <br>Examples:<br>
     *   <p class="example">
     *    PickAttributes atts = proxy.GetViewerState().GetPickAttributes();<br>
     *    // Modify the atts here... (not shown)<br>
     *    atts.Notify();<br>
     *    proxy.GetViewerMethods().SetPickAttributes();</p>
     * @return true on success; false otherwise.
     */
    public boolean SetPickAttributes()
    {
        GetViewerState().GetViewerRPC().SetRPCType(ViewerRPC.VIEWERRPCTYPE_SETPICKATTRIBUTESRPC);
        GetViewerState().GetViewerRPC().Notify();
        return Synchronize();
    }

    /**
     * Resets the query over time attributes to their default values.
     *
     * @return true on success; false otherwise.
     */
    public boolean ResetQueryOverTimeAttributes()
    {
        GetViewerState().GetViewerRPC().SetRPCType(ViewerRPC.VIEWERRPCTYPE_RESETQUERYOVERTIMEATTRIBUTESRPC);
        GetViewerState().GetViewerRPC().Notify();
        return Synchronize();
    }

    /**
     * Makes the query over time attributes be the default query over time attributes
     * to be used for saved settings.
     *
     * @return true on success; false otherwise.
     */
    public boolean SetDefaultQueryOverTimeAttributes()
    {
        GetViewerState().GetViewerRPC().SetRPCType(ViewerRPC.VIEWERRPCTYPE_SETDEFAULTQUERYOVERTIMEATTRIBUTESRPC);
        GetViewerState().GetViewerRPC().Notify();
        return Synchronize();
    }

    /**
     * Applies the query over time attributes that were previously sent. The 
     * query over time attributes affect VisIt's behavior when performing
     * queries that can be made over time to yield curves, etc.
     *
     * <br>Examples:<br>
     *   <p class="example">
     *    QueryOverTimeAttributes atts = proxy.GetViewerState().GetQueryOverTimeAttributes();<br>
     *    // Modify the atts here... (not shown)<br>
     *    atts.Notify();<br>
     *    proxy.GetViewerMethods().SetQueryOverTimeAttributes();</p>
     * @return true on success; false otherwise.
     */
    public boolean SetQueryOverTimeAttributes()
    {
        GetViewerState().GetViewerRPC().SetRPCType(ViewerRPC.VIEWERRPCTYPE_SETQUERYOVERTIMEATTRIBUTESRPC);
        GetViewerState().GetViewerRPC().Notify();
        return Synchronize();
    }

    /**
     * Sets whether VisIt should invest in the extra effort needed to return
     * valid cycles and times from virtual databases composed of many single
     * time step files.
     *
     * @param flag true if VisIt should take the extra effort, which can be significant; false otherwise.
     * @return true on success; false otherwise.
     */
    public boolean SetTryHarderCyclesTimes(int flag)
    {
        GetViewerState().GetViewerRPC().SetRPCType(ViewerRPC.VIEWERRPCTYPE_SETTRYHARDERCYCLESTIMESRPC);
        GetViewerState().GetViewerRPC().SetIntArg1(flag);
        GetViewerState().GetViewerRPC().Notify();
        return Synchronize();
    }

    /**
     * Sets whether VisIt should treat all time-varying databases as having 
     * dynamic metadata. This option forces the variable lists to be refreshed
     * each time the user changes the time state for the active database. This 
     * option can often have a negative performance impact but it allows you
     * to acquire variables that appear later in the file sequence that makes
     * up the database.
     *
     * @param flag true if VisIt should treat all databases as having dynamic metadata.
     * @return true on success; false otherwise.
     */
    public boolean SetTreatAllDBsAsTimeVarying(int flag)
    {
        GetViewerState().GetViewerRPC().SetRPCType(ViewerRPC.VIEWERRPCTYPE_SETTREATALLDBSASTIMEVARYINGRPC);
        GetViewerState().GetViewerRPC().SetIntArg1(flag);
        GetViewerState().GetViewerRPC().Notify();
        return Synchronize();
    }

    /**
     * Resets the lineout color the initial lineout color.
     *
     * @return true on success; false otherwise.
     */
    public boolean ResetLineoutColor()
    {
        GetViewerState().GetViewerRPC().SetRPCType(ViewerRPC.VIEWERRPCTYPE_RESETLINEOUTCOLORRPC);
        GetViewerState().GetViewerRPC().Notify();
        return Synchronize();
    }

    /**
     * Resets the interactor attributes to their defaults.
     *
     * @return true on success; false otherwise.
     */
    public boolean ResetInteractorAttributes()
    {
        GetViewerState().GetViewerRPC().SetRPCType(ViewerRPC.VIEWERRPCTYPE_RESETINTERACTORATTRIBUTESRPC);
        GetViewerState().GetViewerRPC().Notify();
        return Synchronize();
    }

    /**
     * Set the default interactor attributes.
     *
     * @return true on success; false otherwise.
     */
    public boolean SetDefaultInteractorAttributes()
    {
        GetViewerState().GetViewerRPC().SetRPCType(ViewerRPC.VIEWERRPCTYPE_SETDEFAULTINTERACTORATTRIBUTESRPC);
        GetViewerState().GetViewerRPC().Notify();
        return Synchronize();
    }

    /**
     * Applies the interactor attributes that were previously sent. The 
     * interactor attributes govern the behavior of VisIt's navigation 
     * and zoom window modes.
     *
     * <br>Examples:<br>
     *   <p class="example">
     *    InteractorAttributes atts = proxy.GetViewerState().GetInteractorAttributes();<br>
     *    // Modify the atts here... (not shown)<br>
     *    atts.Notify();<br>
     *    proxy.GetViewerMethods().SetInteractorAttributes();</p>
     * @return true on success; false otherwise.
     */
    public boolean SetInteractorAttributes()
    {
        GetViewerState().GetViewerRPC().SetRPCType(ViewerRPC.VIEWERRPCTYPE_SETINTERACTORATTRIBUTESRPC);
        GetViewerState().GetViewerRPC().Notify();
        return Synchronize();
    }

    /**
     * Returns process information related to a particular VisIt component
     *
     * @param id ???
     * @param engineHostName The name of the computer running the compute engine.
     * @param engineDBName The name of the engine's active database.
     * @return true on success; false otherwise.
     */
    public boolean QueryProcessAttributes(int id, String engineHostName, String engineDBName)
    {
        GetViewerState().GetViewerRPC().SetRPCType(ViewerRPC.VIEWERRPCTYPE_GETPROCINFORPC);
        GetViewerState().GetViewerRPC().SetIntArg1(id);
        GetViewerState().GetViewerRPC().SetProgramHost(engineHostName);
        GetViewerState().GetViewerRPC().SetProgramSim(engineDBName);
        GetViewerState().GetViewerRPC().Notify();
        return Synchronize();
    }

    /**
     * Sends a command to a connected simulation.
     *
     * @param hostName The name of the computer where the simulation is running.
     * @param simName  The name of the simulation.
     * @param command The name of the command to send to the simulation.
     * @param argument The arguments to the command encoded as a string.
     * @return true on success; false otherwise.
     */
    public boolean SendSimulationCommand(String hostName, String simName, String command, String argument)
    {
        GetViewerState().GetViewerRPC().SetRPCType(ViewerRPC.VIEWERRPCTYPE_SENDSIMULATIONCOMMANDRPC);
        GetViewerState().GetViewerRPC().SetProgramHost(hostName);
        GetViewerState().GetViewerRPC().SetProgramSim(simName);
        GetViewerState().GetViewerRPC().SetStringArg1(command);
        GetViewerState().GetViewerRPC().SetStringArg2(argument);
        GetViewerState().GetViewerRPC().Notify();
        return Synchronize();
    }

    /**
     * Sends a command to a connected simulation.
     *
     * @param hostName The name of the computer where the simulation is running.
     * @param simName  The name of the simulation.
     * @param command The name of the command to send to the simulation.
     * @return true on success; false otherwise.
     */
    public boolean SendSimulationCommand(String hostName, String simName, String command)
    {
        return SendSimulationCommand(hostName, simName, command, "");
    }

    /**
     * Opens another VisIt client such as the GUI or CLI. This method allows
     * one VisIt client to open another by telling the viewer to reverse
     * launch the client.
     *
     * @param clientName The name to use to identify the new client.
     * @param program  The program to execute
     * @param args Additional command line arguments to be passed to the client.
     * @return true on success; false otherwise.
     */
    public boolean OpenClient(String clientName, String program, Vector args)
    {
        GetViewerState().GetViewerRPC().SetRPCType(ViewerRPC.VIEWERRPCTYPE_OPENCLIENTRPC);
        GetViewerState().GetViewerRPC().SetDatabase(clientName);
        GetViewerState().GetViewerRPC().SetProgramHost(program);
        GetViewerState().GetViewerRPC().SetProgramOptions(args);
        GetViewerState().GetViewerRPC().Notify();
        return Synchronize();
    }

    /**
     * Turns off query output so it does not get printed.
     *
     * @param onOff true turns off query output; false turns it on.
     * @return true on success; false otherwise.
     */
    public boolean SuppressQueryOutput(boolean onOff)
    {
        GetViewerState().GetViewerRPC().SetRPCType(ViewerRPC.VIEWERRPCTYPE_SUPPRESSQUERYOUTPUTRPC);
        GetViewerState().GetViewerRPC().SetBoolFlag(onOff);
        GetViewerState().GetViewerRPC().Notify();
        return Synchronize();
    }

    /**
     * Sets the format string used for floating point numbers when creating 
     * query output strings. This method allows you to control the precision.
     *
     * @param format A valid C-style format string for floating point numbers.
     * @return true on success; false otherwise.
     */
    public boolean SetQueryFloatFormat(String format)
    {
        GetViewerState().GetViewerRPC().SetRPCType(ViewerRPC.VIEWERRPCTYPE_SETQUERYFLOATFORMATRPC);
        GetViewerState().GetViewerRPC().SetStringArg1(format);
        GetViewerState().GetViewerRPC().Notify();
        return Synchronize();
    }

    /**
     * Tells the viewer to send back plot information that was created by the specified
     * plot.
     *
     * @param plotID The index of the plot in the plot list (>=0).
     * @param winID The index of the visualization window that contains the plot (>=1).
     * @return true on success; false otherwise.
     */
    public boolean UpdatePlotInfoAtts(int plotID, int winID)
    {
        GetViewerState().GetViewerRPC().SetRPCType(ViewerRPC.VIEWERRPCTYPE_UPDATEPLOTINFOATTSRPC);
        GetViewerState().GetViewerRPC().SetWindowId(winID);
        GetViewerState().GetViewerRPC().SetIntArg1(plotID);
        GetViewerState().GetViewerRPC().Notify();
        return Synchronize();
    }

    /**
     * Sets whether VisIt will automatically create mesh quality expressions.
     *
     * @param flag true means that the expressions will be created; false otherwise.
     * @return true on success; false otherwise.
     */
    public boolean SetCreateMeshQualityExpressions(boolean flag)
    {
        GetViewerState().GetViewerRPC().SetRPCType(ViewerRPC.VIEWERRPCTYPE_SETCREATEMESHQUALITYEXPRESSIONSRPC);
        GetViewerState().GetViewerRPC().SetIntArg1(flag?1:0);
        GetViewerState().GetViewerRPC().Notify();
        return Synchronize();
    }

    /**
     * Sets whether VisIt will automatically create time-derivative expressions.
     *
     * @param flag true means that the expressions will be created; false otherwise.
     * @return true on success; false otherwise.
     */
    public boolean SetCreateTimeDerivativeExpressions(boolean flag)
    {
        GetViewerState().GetViewerRPC().SetRPCType(ViewerRPC.VIEWERRPCTYPE_SETCREATETIMEDERIVATIVEEXPRESSIONSRPC);
        GetViewerState().GetViewerRPC().SetIntArg1(flag?1:0);
        GetViewerState().GetViewerRPC().Notify();
        return Synchronize();
    }

    /**
     * Sets whether VisIt will automatically create vector magnitude expressions.
     *
     * @param flag true means that the expressions will be created; false otherwise.
     * @return true on success; false otherwise.
     */
    public boolean SetCreateVectorMagnitudeExpressions(boolean flag)
    {
        GetViewerState().GetViewerRPC().SetRPCType(ViewerRPC.VIEWERRPCTYPE_SETCREATEVECTORMAGNITUDEEXPRESSIONSRPC);
        GetViewerState().GetViewerRPC().SetIntArg1(flag?1:0);
        GetViewerState().GetViewerRPC().Notify();
        return Synchronize();
    }

    /**
     * Sets whether VisIt will will suppress messages that would ordinarily 
     * be sent to clients.
     *
     * @param flag true means that the messages will be suppressed and remain suppressed
     *             until this method is called again with a value of false.
     * @return true on success; false otherwise.
     */
    public boolean SetSuppressMessages(boolean flag)
    {
        GetViewerState().GetViewerRPC().SetRPCType(ViewerRPC.VIEWERRPCTYPE_SETSUPPRESSMESSAGESRPC);
        GetViewerState().GetViewerRPC().SetIntArg1(flag?1:0);
        GetViewerState().GetViewerRPC().Notify();
        return Synchronize();
    }

    //
    // Convenience methods
    //

    private ViewerState GetViewerState()
    {
        return proxy.GetViewerState();
    }

    private boolean Synchronize()
    {
        return proxy.GetSynchronous() ? proxy.Synchronize() : true;
    }

    private ViewerProxy proxy;
}
