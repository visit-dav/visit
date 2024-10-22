// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

import llnl.visit.ViewerProxy;
import llnl.visit.MachineProfile;
import llnl.visit.LaunchProfile;
import llnl.visit.HostProfileList;
import java.util.Vector;

// ****************************************************************************
// Class: MakeHostProfile
//
// Purpose:
//   This class implements an example program that shows how to use the
//   ViewerProxy class and control VisIt's viewer from Java.
//
// Notes:
//
// Programmer: Brad Whitlock
// Creation:   Mon Aug 10 13:40:40 PDT 2009
//
// Modifications:
//   Jeremy Meredith, Thu Feb 18 17:14:38 EST 2010
//   Split host profile into machine profile and launch profile.
//   Also, added directory argument.
//
//   Kathleen Biagas, Wed Nov  9 14:30:32 PST 2016
//   Update host name to a machine that still exists.
//
//   Kathleen Biagas, Tuesday Oct 22, 2024
//   Replace pascal with poodle.
//
// ****************************************************************************

public class MakeHostProfile extends RunViewer
{
    public MakeHostProfile()
    {
        super();
    }

    protected void work(String[] args)
    {
        // Change these for your remote system.
        String host = new String("poodle.llnl.gov");
        String user = new String("kbonnell");
        String remotevisitPath = new String("/usr/gapps/visit");

        // Basic, serial profile.
        LaunchProfile example = new LaunchProfile();
        example.SetProfileName("example");
        example.SetActive(true);

        // Create a new machine profile object and the serial launch profile.
        MachineProfile profile = new MachineProfile();
        profile.SetHost(host);
        profile.SetUserName(user);
        profile.SetClientHostDetermination(MachineProfile.CLIENTHOSTDETERMINATION_PARSEDFROMSSHCLIENT);
        profile.SetTunnelSSH(true);
        profile.SetDirectory(remotevisitPath);
        profile.AddLaunchProfiles(example);

        // Replace the list of host profiles and tell the viewer about the changes. We could
        // have added to the list instead of clearing the list.
        viewer.GetViewerState().GetHostProfileList().ClearMachines();
        viewer.GetViewerState().GetHostProfileList().AddMachines(profile);
        viewer.GetViewerState().GetHostProfileList().Notify();
        System.out.println("HostProfileList = \n" +
            viewer.GetViewerState().GetHostProfileList().toString(""));

        // Do a plot of the remote data.
        String remoteData = new String(host + ":" + remotevisitPath + "/data/globe.silo");
        if(viewer.GetViewerMethods().OpenDatabase(remoteData))
        {
            viewer.GetViewerMethods().AddPlot("Pseudocolor", "u");
            viewer.GetViewerMethods().AddPlot("Mesh", "mesh1");
            viewer.GetViewerMethods().DrawPlots();
        }
        else
        {
            System.out.println("Could not open the database!");
        }
    }

    public static void main(String args[])
    {
        MakeHostProfile r = new MakeHostProfile();
        r.run(args);
    }
}
