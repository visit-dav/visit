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

import java.lang.ArrayIndexOutOfBoundsException;
import java.util.Vector;
import java.util.prefs.Preferences;
import java.util.prefs.BackingStoreException;

// ****************************************************************************
// Class: ViewerProxy
//
// Purpose:
//   This is the main class that users of the Java VisIt Interface need to
//   use in order to control VisIt from Java. This class provides methods to
//   launch VisIt's viewer tell it to do things.
//
// Notes:      
//
// Programmer: Brad Whitlock
// Creation:   Thu Aug 8 12:43:40 PDT 2002
//
// Modifications:
//   Brad Whitlock, Mon Sep 16 12:46:02 PDT 2002
//   I added a method to clear reflines and a method to set the view extents.
//   I also added the windowInfo and renderAtts members.
//
//   Brad Whitlock, Tue Sep 24 07:22:27 PDT 2002
//   I fixed the Create method so it blocks until the plugins are loaded.
//   This prevents an error where an exception is thrown when looking up the
//   name of an exception before the plugins are loaded. Observers can still
//   attach to the subjects in the ViewerProxy but in order to get initial
//   state from the viewer, they should be connected before the call to the
//   Create method.
//
//   Brad Whitlock, Tue Oct 15 16:22:42 PST 2002
//   I added methods to clone windows and copy plots from one window to
//   another window.
//
//   Jeremy Meredith, Thu Oct 24 16:15:11 PDT 2002
//   Added material options.
//
//   Brad Whitlock, Mon Nov 11 11:43:28 PDT 2002
//   I added methods to lock tools and lock time.
//
//   Brad Whitlock, Thu Dec 12 10:22:22 PDT 2002
//   I fixed some color table issues.
//
//   Brad Whitlock, Fri Dec 13 17:03:00 PST 2002
//   I added keyframing routines.
//
//   Brad Whitlock, Fri Dec 27 15:45:38 PST 2002
//   I made it use a new automatically generated version of ViewerRPC so it
//   is trivial to keep the code up to date with the C++ version. I also
//   added new methods that were added while I was making this update.
//
//   Brad Whitlock, Thu Mar 20 10:30:13 PDT 2003
//   I added GlobalLineoutAttributes.
//
//   Brad Whitlock, Thu Apr 10 09:27:16 PDT 2003
//   I added PromoteOperator, DemoteOperator, RemoveOperator methods. I also
//   added an override of SetActivePlots.
//
//   Brad Whitlock, Mon May 5 14:13:33 PST 2003
//   I made it use the regenerated ViewerRPC.java class.
//
//   Brad Whitlock, Thu May 15 13:05:09 PST 2003
//   I added a new OpenDatabase method that lets us open databases at late
//   time states.
//
//   Kathleen Bonnell, Tue Jul  1 10:11:37 PDT 2003
//   I added SetPickAttributes, SetGlobalLineoutAttributes. 
//
//   Brad Whitlock, Tue Jul 1 16:49:48 PST 2003
//   I added a method to tell the viewer to export a colortable.
//
//   Brad Whitlock, Wed Jul 9 12:30:58 PDT 2003
//   I added methods to tell the viewer to export and import its entire state.
//
//   Eric Brugger, Wed Aug 27 09:10:35 PDT 2003
//   I added viewCurveAttributes.  I split the view attributes into 2d
//   and 3d parts.
//
//   Brad Whitlock, Fri Aug 29 11:24:11 PDT 2003
//   I added HideToolbars and ShowToolbars.
//
//   Brad Whitlock, Wed Oct 15 15:39:02 PST 2003
//   I added a timeState argument to the ReplaceDatabase method.
//
//   Brad Whitlock, Wed Oct 22 12:25:43 PDT 2003
//   I added another overloaded OpenDatabase method.
//
//   Brad Whitlock, Wed Oct 29 10:41:21 PDT 2003
//   I added new RPCs to deal with setting up advanced annotations.
//
//   Kathleen Bonnell, Wed Nov 26 14:16:53 PST 2003 
//   Added ResetPickAttributes method. Added overloaded PointQuery and
//   DatabaseQuery methods to handle optional arguments.
//
//   Kathleen Bonnell, Wed Dec 17 15:19:46 PST 2003 
//   Added SetDefaultPickAttributes, ResetPickLetter. 
//
//   Brad Whitlock, Mon Dec 29 09:24:20 PDT 2003
//   Added methods for setting and choosing the center of rotation.
//
//   Brad Whitlock, Thu Jan 8 14:19:43 PST 2004
//   I fixed a typo that prevented it from building.
//
//   Brad Whitlock, Fri Jan 23 14:15:49 PST 2004
//   I added DatabaseCorrelationList and the CloseDatabase rpc.
//
//   Brad Whitlock, Thu Feb 26 13:38:26 PST 2004
//   I added ClearCacheForAllEngines.
//
//   Jeremy Meredith, Tue Mar 30 17:36:24 PST 2004
//   Added support for a simulation name in addition to simply a host name
//   when specifying engines (i.e. ClearCache and CloseComputeEngine).
//
//   Kathleen Bonnell, Wed Mar 31 07:35:25 PST 2004 
//   Added support for QueryOverTimeAttributes.
//
//   Brad Whitlock, Mon Jul 26 15:57:32 PST 2004
//   Added CheckForNewStates.
//
//   Kathleen Bonnell, Wed Aug 18 09:19:25 PDT 2004 
//   Added support for InteractorAttributes.
//
//   Brad Whitlock, Fri Jan 7 17:00:00 PST 2005
//   I hooked up the InteractorAttributes to xfer and added a silAtts
//   member to get the sil attributes for simulations.
//
//   Brad Whitlock, Fri Apr 8 11:57:46 PDT 2005
//   Overloaded AddOperator.
//
//   Brad Whitlock, Fri Apr 15 10:32:35 PDT 2005
//   Added PostponedAction.
//
//   Brad Whitlock, Thu May 12 13:46:37 PST 2005
//   Added ProcessAttributes.
//
//   Brad Whitlock, Wed May 4 09:53:34 PDT 2005
//   Changed the order of some state objects in xfer and added support for
//   reverse launching. I also added OpenClient, AssistOpenClient, and the
//   new clientMethod, clientInformation, clientInformationList, and
//   movieAttributes members.
//
//   Brad Whitlock, Thu Jun 2 11:15:40 PDT 2005
//   Added SetTryHarderCyclesTimes.
//
//   Brad Whitlock, Mon Jun 6 09:39:33 PDT 2005
//   Added some code to yield to other threads so we don't tie up the CPU
//   in some of our blocking loops. I also added code to try and determine
//   VisIt's installation location using java.util.prefs. The Windows
//   installer for VisIt pokes the right values into the registry so Java
//   will know where to look for VisIt. On UNIX, we use the default binary
//   path unless the user overrides it. I also added GetDataPath so it is
//   easier to write test programs that know where to find default data files.
//
//   Brad Whitlock, Thu Jun 16 17:24:58 PST 2005
//   I made the code that waits for the plugins to get loaded use the Yielder
//   class, which makes the thread sleep if yield is not sufficient to make
//   the thread reduce its CPU usage. I also changed things so the Create
//   method never turns off xfer's reading thread. This means that the class
//   will now always read from the viewer unless you call StopProcessing.
//
//   Brad Whitlock, Thu Jul 14 12:25:11 PDT 2005
//   I made SetPlotOptions and SetOperatorOptions complain if they are given
//   invalid indices or names.
//
//   Brad Whitlock, Thu Oct 27 10:45:59 PDT 2005
//   I made it possible to skip plugins that did not load while still allowing
//   plugins that did load to get hooked up to xfer.
//
//   Brad Whitlock, Thu Nov 17 16:36:17 PST 2005
//   I added methods to move and resize windows.
//
//   Brad Whitlock, Tue Nov 29 12:13:41 PDT 2005
//   I added mesh management attributes.
//
//   Brad Whitlock, Tue Jan 10 16:29:28 PST 2006
//   Added another ViewerRPC object for logging.
//
//   Brad Whitlock, Tue Mar 7 16:43:46 PST 2006
//   Added RedoView.
//
//   Brad Whitlock, Thu Mar 16 16:23:50 PST 2006
//   Added DDF attributes.
//
//   Brad Whitlock, Fri Sep 22 14:17:25 PST 2006
//   Added PlotInfoAttributes.
//
//   Brad Whitlock, Fri Nov 10 09:36:57 PDT 2006
//   Added ImportEntireStateWithDifferentSources RPC.
//
//   Brad Whitlock, Wed Mar 14 18:16:12 PST 2007
//   Updated some state object interfaces and added metadata functions.
//
//   Brad Whitlock, Wed Jan 23 10:26:28 PST 2008
//   Added TurnOFfAllLocks, SetPlotFollowsTime.
//
//   Jeremy Meredith, Wed Jan 23 16:39:05 EST 2008
//   Added fileOpenOptions to match viewer state.
//
//   Jeremy Meredith, Mon Feb  4 13:43:57 EST 2008
//   Added axis array view support.
//
//   Brad Whitlock, Fri Feb 22 16:50:05 PST 2008
//   Refactored some code into ViewerState and ViewerMethods. State objects
//   are now located in ViewerState. Viewer rpc's are now located in
//   ViewerMethods. Added JavaDoc comments.
//
// ****************************************************************************
/**
 * ViewerProxy is the main class that users of the Java VisIt Interface need to
 * use in order to control VisIt from Java. This class provides methods to
 * launch VisIt's viewer and control it.
 *
 * <br>Example usage:<br>
 *   <p class="example">
 *   ViewerProxy proxy = new ViewerProxy();<br>
 *   <i>// Set the path to the visit launch script</i><br>
 *   proxy.SetBinPath("/usr/local/apps/visit/bin");<br>
 *   <i>// Add arguments before we launch the viewer.</i><br>
 *   proxy.AddArgument("-debug");<br>
 *   proxy.AddArgument("5"); <br>
 *   <i>// Launch the viewer, listening on port 5600</i><br>
 *   if(proxy.Create(5600))<br>
 *   {<br>
 *   &nbsp;&nbsp;&nbsp;&nbsp;<i>// Now that the viewer is running, do stuff.</i><br>
 *   &nbsp;&nbsp;&nbsp;&nbsp;proxy.GetViewerMethods().OpenDatabase(proxy.GetDataPath()+"globe.silo");<br>
 *   &nbsp;&nbsp;&nbsp;&nbsp;proxy.AddPlots("Pseudocolor", "speed");<br>
 *   &nbsp;&nbsp;&nbsp;&nbsp;proxy.DrawPlots();<br>
 *   &nbsp;&nbsp;&nbsp;&nbsp;proxy.SaveWindow();<br>
 *   &nbsp;&nbsp;&nbsp;&nbsp;<i>// We're done so close the viewer.</i><br>
 *   &nbsp;&nbsp;&nbsp;&nbsp;proxy.Close();<br>
 *   }<br>
 *   </p>
 * @author Brad Whitlock
 */
public class ViewerProxy implements SimpleObserver
{
    /**
     * Contructor for the ViewerProxy class. Basic initialization is performed.
     */
    public ViewerProxy()
    {
        viewer = new RemoteProcess("visit");
        viewer.AddArgument("-viewer");

        // Make viewer RPC's synchronous by default.
        syncCount = 0;
        synchronous = true;
        pluginsLoaded = false;
        doUpdate = true;
        verbose = false;

        xfer = new Xfer();
        messageObserver = new MessageObserver();
        syncNotifier = new SyncNotifier();

        state = new ViewerState();
        methods = new ViewerMethods(this);

        // Create the plugin managers.
        plotPlugins = new PluginManager("plot");
        operatorPlugins = new PluginManager("operator");

        // The VISITHOME Java preference will not have been set on
        // UNIX, set the default paths to where we install in /usr/gapps.
        SetBinPath("/usr/gapps/visit/bin");
        dataPath = new String("/usr/gapps/visit/data/");

        // Now that the default paths have been set, try and override them
        // with information from the Java preferences.
        try
        {
            String visitHomeKey = new String("/llnl/visit");           
            if(Preferences.systemRoot().nodeExists(visitHomeKey))
            {
                // Get the node
                Preferences visitNode = Preferences.systemRoot().node(visitHomeKey);
                String defaultValue = new String("");
                String visitPath = visitNode.get("VISITHOME", defaultValue);
                if(!visitPath.equals(defaultValue))
                {
                    PrintMessage("Setting visit path to: " + visitPath);
                    SetBinPath(visitPath);
                    dataPath = new String(visitPath + "\\data\\");
                }
            }
        }
        catch(BackingStoreException e)
        {
            PrintMessage("Cannot access preferences");
        }
    }

    /**
     * Closes VisIt's viewer when the object is garbage collected.
     */
    protected void finalize() throws Throwable
    {
        Close();
        super.finalize();
    }

    /**
     * Sets the location of the visit binary that Java will launch. The
     * version of VisIt must be the same version as the Java client.
     *
     * @param path An absolute path to the "bin" directory containing
     *             the "visit" script.
     */
    public void SetBinPath(String path)
    {
        viewer.SetBinPath(path);
    }

    /**
     * Gets the data path directory, which is often a "data" directory 
     * peer to the "bin" directory provided via SetBinPath.
     *
     * @return The data path directory that you can use to access VisIt's
     *         built-in data files.
     */
    public String GetDataPath()
    {
        return dataPath;
    }

    /**
     * Adds extra arguments to the viewer's command line so you can
     * launch VisIt with arguments that you provide.
     *
     * @param arg A command line option to be added to the viewer command
     *            line. Note that once the viewer is launched, this method
     *            has no effect.
     */
    public void AddArgument(String arg)
    {
        viewer.AddArgument(arg);
    }

    /**
     * Prints a message to the console if the ViewerProxy object is 
     * in verbose mode.
     *
     * @param msg The message to print.
     */
    public synchronized void PrintMessage(String msg)
    {
        if(verbose)
            System.out.println(msg);
    }

    /**
     * Launches VisIt's viewer and performs the rest of the initialization
     * of the ViewerProxy class, including loading plugins, starting a listener
     * thread to read from the viewer.
     *
     * @param port The default port that will be used to listen for the
     *             viewer when it wants to connect back to the ViewerProxy.
     *             VisIt usually defaults to port 5600.
     * @return true on success; false on failure.
     */
    // Launches the viewer and sets up state objects.
    public boolean Create(int port)
    {
        boolean retval = false;

        if(viewer.Open(port))
        {
            retval = true;

            PrintMessage("Adding state objects.");

            // Set up xfer and the RPC's. The state objects herein must appear
            // in the same order as in ViewerState.h.

            xfer.SetRemoteProcess(viewer);
            for(int i = 0; i < state.GetNumStateObjects(); ++i)
                xfer.Add(state.GetStateObject(i));

            // hook up the message observer.
            messageObserver.Attach(state.GetMessageAttributes());

            // Hook up the syncAtts notifier.
            state.GetSyncAttributes().Attach(syncNotifier);

            // Hook up this object to the plugin atts.
            state.GetPluginManagerAttributes().Attach(this);

            // Start reading input from the viewer so we can load the
            // plugins that the viewer has loaded.
            StartProcessing();
            Yielder y = new Yielder(1000);
            boolean errFlag = false;
            while(!errFlag && !pluginsLoaded)
            {
                // Yield to other threads so we don't swamp the CPU with
                // zero work.
                try
                {
                    y.yield();
                }
                catch(java.lang.InterruptedException e)
                {
                    errFlag = true;
                }
            }
        }

        return retval;
    }

    /**
     * Starts automatic processing of information from the viewer. If automatic
     * processing is enabled then the ViewerProxy will listen for new state
     * from the viewer on a socket. For most applications, this is desireable.
     *
     */
    public void StartProcessing()
    {
        PrintMessage("Starting to read from input from viewer.");
        xfer.StartProcessing();
    }

    /**
     * Stops automatic processing of information from the viewer.
     */
    public void StopProcessing()
    {
        PrintMessage("Stopped reading input from viewer.");
        xfer.StopProcessing();
    }

    /**
     * Sets whether the ViewerProxy will force synchronization with the
     * viewer after calling methods from the ViewerMethods object. Simple 
     * applications applications should perform synchronous processing since
     * it makes the values returned from ViewerMethods functions take into
     * account any errors that may have occurred while processing a request.
     *
     * Event-based applications should disable synchronous processing so they
     * can send off commands to the viewer without blocking for return values.
     * In the event that synchronization is truely needed in that case, you
     * can always call the Synchronize() method.
     *
     * @param val true for synchronous processing; false for asynchronous.
     */
    public void SetSynchronous(boolean val)
    {
        synchronous = val;
    }

    /**
     * Return whether the proxy is running with synchronization.
     *
     * @return true for synchronous processing; false for asynchronous.
     */
    public boolean GetSynchronous()
    {
        return synchronous;
    }

    /**
     * Sets a flag that determines if messages from the viewer are printed to
     * the console.
     *
     * @param val True to make the ViewerProxy print verbose output.
     */
    public void SetVerbose(boolean val)
    {
        verbose = val;
        messageObserver.SetVerbose(val);
        plotPlugins.SetVerbose(val);
        operatorPlugins.SetVerbose(val);
        viewer.SetVerbose(val);
        xfer.SetVerbose(val);
        syncNotifier.SetVerbose(val);
    }

    /**
     * Tells the ViewerProxy to close the viewer. The ViewerProxy should not
     * be used after calling  this method since calling Create() again will
     * not currently relaunch the viewer properly.
     *
     * @return true
     */
    public boolean Close()
    {
        PrintMessage("Closing the viewer.");
        xfer.StopProcessing();
        methods.Close();
        viewer.Close();
        return true;
    }

    /**
     * Sends an interruption message to the viewer, which can be used to
     * interrupt processing of multiple domain datasets in the compute engine.
     *
     */
    public void SendInterruption()
    {
        xfer.SendInterruption();
    }

    /**
     * Sends a synchronization message to the viewer and blocks until the 
     * viewer's reply to that message is read on the listener thread.
     *
     * @return true if there were no errors; false otherwise.
     */
    public synchronized boolean Synchronize()
    {
        // Clear any error in the message observer.
        messageObserver.ClearError();

        // Tell the syncObserver to notify this thread when the new syncCount
        // gets returned from the viewer.
        ++syncCount;
        syncNotifier.NotifyThreadOnValue(Thread.currentThread(), syncCount);
        syncNotifier.SetUpdate(false);

        // Send a new synchronization tag
        GetViewerState().GetSyncAttributes().SetSyncTag(syncCount);
        GetViewerState().GetSyncAttributes().Notify();
        GetViewerState().GetSyncAttributes().SetSyncTag(-1);

        // Wait until the syncNotifier notifies us that it's okay to proceed.
        boolean noErrors = true;
        try
        {
            synchronized(Thread.currentThread())
            { 
                PrintMessage("Waiting for viewer sync.");
                Thread.currentThread().wait();
            }
        }
        catch(java.lang.InterruptedException e)
        {
            noErrors = false;
        }

        // Make it return true if there are no errors.
        return noErrors && !messageObserver.GetErrorFlag();
    }

    /**
     * Gets the last error message that was encountered. This method can be
     * called after a Synchronize() call to get any errors that were posted.
     *
     * @return The last error message.
     */
    public String GetLastError()
    {
        return messageObserver.GetLastError();
    }

    /**
     * Returns the ViewerState object, which is the container object that
     * holds all of the state objects that can be observed or manipulated
     * in VisIt.
     *
     * One will commonly get the viewer state and then get one of its state
     * objects and make changes to it before calling the Notify() method on
     * that state object to affect changes in the viewer.
     *
     * @return The ViewerState object.
     * @see ViewerState
     */
    public ViewerState GetViewerState()
    {
        return state;
    }

    /**
     * Returns the ViewerMethods object, which is the container object that
     * maps all method calls through it into ViewerRPC calls to the viewer.
     * There are a lot of methods that can be called and it improves maintainability
     * to have the methods separated out.
     *
     * @return The ViewerMethods object.
     */
    public ViewerMethods GetViewerMethods()
    {
        return methods;
    }

    /**
     * Convenience method that looks up the index of the named plot plugin
     * in the plot plugin manager.
     *
     * @param plotName The name of the plot to look for.
     * @return The index of the plot in the plugin manager.
     */
    public int GetPlotIndex(String plotName)
    {
        return plotPlugins.IndexFromName(plotName);
    }

    /**
     * Convenience method that looks up the name of the plot plugin
     * in the plot plugin manager, given an index.
     *
     * @param index The index of the plot.
     * @return The name of the plot in the plugin manager.
     */
    public String GetPlotName(int index) throws ArrayIndexOutOfBoundsException
    { 
        return plotPlugins.GetPluginName(index);
    }

    /**
     * Convenience method that returns the plot plugin version for the 
     * specified plugin index.
     *
     * @param index The index of the plot.
     * @return The version of the plot in the plugin manager.
     */
    public String GetPlotVersion(int index) throws ArrayIndexOutOfBoundsException
    { 
        return plotPlugins.GetPluginVersion(index);
    }

    /**
     * Convenience method that returns the state object for the plot plugin.
     * You can access the state object via ViewerState too.
     *
     * @param index The index of the plot.
     * @return The plot's state object. This is the object that you'd use to
     *         make changes in the plot's attributes, programmatically or through
     *         a GUI window, etc.
     */
    public Plugin GetPlotAttributes(int index) throws ArrayIndexOutOfBoundsException
    {
        return plotPlugins.GetPluginAttributes(index);
    }

    /**
     * Convenience method that returns the state object for the plot plugin.
     * You can access the state object via ViewerState too.
     *
     * @param plotName The name of the plot.
     * @return The plot's state object. This is the object that you'd use to
     *         make changes in the plot's attributes, programmatically or through
     *         a GUI window, etc.
     */
    public Plugin GetPlotAttributes(String plotName) throws ArrayIndexOutOfBoundsException
    {
        int index = plotPlugins.IndexFromName(plotName);
        return plotPlugins.GetPluginAttributes(index);
    }

    /**
     * Returns the number of plot plugins that were loaded.
     *
     * @return The number of plot plugins that were loaded.
     */
    public int GetNumPlotPlugins()
    {
        return plotPlugins.GetNumPlugins();
    }

    /**
     * Convenience method that looks up the index of the named operator plugin
     * in the operator plugin manager.
     *
     * @param operatorName The name of the plot to look for.
     * @return The index of the operator in the plugin manager.
     */
    public int GetOperatorIndex(String operatorName)
    {
        return operatorPlugins.IndexFromName(operatorName);
    }

    /**
     * Convenience methods that looks up the name of the operator plugin
     * in the operator plugin manager, given an index.
     *
     * @param index The index of the operator.
     * @return The name of the operator in the plugin manager.
     */
    public String GetOperatorName(int index) throws ArrayIndexOutOfBoundsException
    { 
        return operatorPlugins.GetPluginName(index);
    }

    /**
     * Convenience methods that returns the Operator plugin version for the 
     * specified plugin index.
     *
     * @param index The index of the Operator.
     * @return The version of the Operator in the plugin manager.
     */
    public String GetOperatorVersion(int index) throws ArrayIndexOutOfBoundsException
    { 
        return operatorPlugins.GetPluginVersion(index);
    }

    /**
     * Convenience method that returns the state object for the operator plugin.
     * You can access the state object via ViewerState too.
     *
     * @param index The index of the operator.
     * @return The operator's state object. This is the object that you'd use to
     *         make changes in the operator's attributes, programmatically or through
     *         a GUI window, etc.
     */
    public Plugin GetOperatorAttributes(int index) throws ArrayIndexOutOfBoundsException
    {
        return operatorPlugins.GetPluginAttributes(index);
    }

    /**
     * Convenience method that returns the state object for the operator plugin.
     * You can access the state object via ViewerState too.
     *
     * @param operatorName The name of the plot.
     * @return The operator's state object. This is the object that you'd use to
     *         make changes in the operator's attributes, programmatically or through
     *         a GUI window, etc.
     */
    public Plugin GetOperatorAttributes(String operatorName) throws ArrayIndexOutOfBoundsException
    {
        int index = operatorPlugins.IndexFromName(operatorName);
        return operatorPlugins.GetPluginAttributes(index);
    }

    /**
     * Returns the number of operator plugins that were loaded.
     *
     * @return The number of operator plugins that were loaded.
     */
    public int GetNumOperatorPlugins()
    {
        return operatorPlugins.GetNumPlugins();
    }

    //
    // SimpleObserver interface
    //

    /**
     * This method implements the SimpleObserver interface and is called when
     * an observed state object changes, in this case, the plugin manager 
     * attributes state object. We use this method to load plugins when we get
     * the list of loaded plugins from the viewer.
     *
     * @param s The state object that was modified - in this case the plugin
     *          manager attributes.
     */
    public void Update(AttributeSubject s)
    {
        if(s == GetViewerState().GetPluginManagerAttributes())
        {
            LoadPlugins();
        }
    }

    /**
     * This method implements the SimpleObserver interface and is called when
     * we want to prevent updates from an observed state object to have an effect
     * on this observer.
     *
     * @param val True to make the update have an effect; false otherwise.
     */
    public void SetUpdate(boolean val) { doUpdate = val; }

    /**
     * This method implements the SimpleObserver interface and is called when
     * we want to determine whether updates to a state object should cause this
     * object's Update method to be called.
     *
     * @return True if updates are enabled; false otherwise.
     */
    public boolean GetUpdate() { return doUpdate; }

    /**
     * Returns whether plugins have been loaded.
     *
     * @return True if have been loaded; false otherwise.
     */
    public boolean PluginsLoaded()
    {
        return pluginsLoaded;
    }

    /**
     * Loads VisIt's plugins according to the list specified in the plugin
     * manager attributes. When the ViewerProxy calls this method, the 
     * client/viewer state object interface is completed.
     *
     * @return True if have been loaded; false otherwise.
     */
    private void LoadPlugins()
    {
        if(!pluginsLoaded)
        {
            // Try loading the plot plugins. If they can all be loaded,
            // add them to xfer.
            plotPlugins.LoadPlugins(GetViewerState().GetPluginManagerAttributes());
            for(int i = 0; i < plotPlugins.GetNumPlugins(); ++i)
            {
                 Plugin p = plotPlugins.GetPluginAttributes(i);
                 state.RegisterPlotAttributes((AttributeSubject)p);
                 if(p != null)
                     xfer.Add((AttributeSubject)p);
                 else
                     xfer.AddDummy();
            }

            // Try loading the operator plugins. If they can all be loaded,
            // add them to xfer.
            operatorPlugins.LoadPlugins(GetViewerState().GetPluginManagerAttributes());
            for(int i = 0; i < operatorPlugins.GetNumPlugins(); ++i)
            {
                 Plugin p = operatorPlugins.GetPluginAttributes(i);
                 state.RegisterOperatorAttributes((AttributeSubject)p);
                 if(p != null)
                     xfer.Add((AttributeSubject)p);
                 else
                     xfer.AddDummy();
            }

            pluginsLoaded = true;
        }
    }

    //
    // Data members
    //
    private RemoteProcess        viewer;
    private ViewerMethods        methods;
    private ViewerState          state;
    private Xfer                 xfer;
    private PluginManager        plotPlugins;
    private PluginManager        operatorPlugins;
    private int                  syncCount;
    private boolean              synchronous;
    private MessageObserver      messageObserver;
    private SyncNotifier         syncNotifier;
    private boolean              pluginsLoaded;
    private boolean              doUpdate;
    private boolean              verbose;
    private String               dataPath;
}
