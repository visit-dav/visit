// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

import java.lang.ArrayIndexOutOfBoundsException;
import java.lang.String;

import java.util.Vector;

import llnl.visit.AttributeSubject;
import llnl.visit.ClientMethod;
import llnl.visit.ClientInformation;
import llnl.visit.ClientInformationList;
import llnl.visit.PlotList;
import llnl.visit.SimpleObserver;


// ****************************************************************************
// Class: DualClients
//
// Purpose:
//   This example program shows how to launch the Python client from Java
//   and send commands to it.
//
// Notes:
//
// Programmer: Brad Whitlock
// Creation:   Tue Jan 11 09:30:41 PST 2011
//
// Modifications:
//
// ****************************************************************************

public class DualClients extends RunViewer implements SimpleObserver
{
    public DualClients()
    {
        super();
        doUpdate = true;

        // Make this object observe the plot list
        viewer.GetViewerState().GetPlotList().Attach(this);
    }

    //
    // Main work method for the program
    //
    protected void work(String[] args)
    {
        // Try and open a database
        if(viewer.GetViewerMethods().OpenDatabase(viewer.GetDataPath() + "noise.silo"))
        {
            // Interpret some Python using the VisIt CLI
            InterpretPython("AddPlot('Pseudocolor', 'hardyglobal')");
            InterpretPython("AddPlot('Mesh', 'Mesh')");
            InterpretPython("DrawPlots()");
            InterpretPython("SaveWindow()");
        }
        else
            System.out.println("Could not open the database!");
    }

    //
    // Check all of the client information until we find a client that
    // supports the Interpret method with a string argument.
    //
    protected boolean NoInterpretingClient()
    {
        // Make a copy because the reader thread could be messing with it.
        // Need to synchronize access.
        ClientInformationList cL = new ClientInformationList(
            viewer.GetViewerState().GetClientInformationList());

        for(int i = 0; i < cL.GetNumClients(); ++i)
        {
            ClientInformation client = cL.GetClients(i);
            for(int j = 0; j < client.GetMethodNames().size(); ++j)
            {
                String name = (String)client.GetMethodNames().elementAt(j);
                if(name.equals("Interpret"))
                {
                    String proto = (String)client.GetMethodPrototypes().elementAt(j);
                    if(proto.equals("s"))
                    {
                        // We have an interpreting client
                        return false;
                    }
                }
            }
        }
        return true;
    }

    //
    // If we don't have a client that can "Interpret" then tell the viewer
    // to launch a VisIt CLI.
    //
    protected boolean Initialize()
    {
        boolean launched = false;
        if(NoInterpretingClient())
        {
            System.out.println("Tell the viewer to create a CLI so we can execute code.");
            Vector args = new Vector();
            args.addElement(new String("-cli"));
            args.addElement(new String("-newconsole"));
            viewer.GetViewerMethods().OpenClient("CLI",
                 "visit",
                 args);
            launched = true;

            viewer.Synchronize();

            // HACK: Wait until we have an interpreting client.
            while(NoInterpretingClient())
                viewer.Synchronize();
        }
        return launched;
    }

    //
    // Interpret a Python command string.
    //
    protected void InterpretPython(String cmd)
    {
        Initialize();

        // Send the command to interpret as a client method.
        ClientMethod method = viewer.GetViewerState().GetClientMethod();
        method.SetIntArgs(new Vector());
        method.SetDoubleArgs(new Vector());
        Vector args = new Vector();
        args.addElement(new String(cmd + "\n"));
        method.SetStringArgs(args);
        method.SetMethodName("Interpret");
        method.Notify();
        System.out.println("Interpret: " + cmd);

        viewer.Synchronize();
    }

    //
    // SimpleObserver interface methods
    //
    public void Update(AttributeSubject s)
    {
        // Do something with the plot list.
        System.out.println(s.toString());
    }
    public void SetUpdate(boolean val) { doUpdate = val; }
    public boolean GetUpdate() { return doUpdate; }


    public static void main(String args[])
    {
        DualClients r = new DualClients();
        r.run(args);
    }

    private boolean doUpdate;
}
