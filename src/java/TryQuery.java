// ****************************************************************************
//
// Copyright (c) 2000 - 2009, Lawrence Livermore National Security, LLC
// Produced at the Lawrence Livermore National Laboratory
// LLNL-CODE-400124
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

import java.lang.ArrayIndexOutOfBoundsException;
import java.util.Vector;
import llnl.visit.AttributeSubject;
import llnl.visit.SimpleObserver;
import llnl.visit.QueryAttributes;

// ****************************************************************************
// Class: TryQuery
//
// Purpose:
//   This example program does a plot and queries some values in it.
//
// Notes:      
//
// Programmer: Brad Whitlock
// Creation:   Tue Oct 1 12:51:29 PDT 2002
//
// Modifications:
//   Brad Whitlock, Thu Dec 12 10:44:31 PDT 2002
//   Updated because of changse to color table methods.
//
//   Brad Whitlock, Thu Jan 2 16:05:48 PST 2003
//   Changed because of Lineout method interface change.
//
//   Brad Whitlock, Mon Jun 6 17:25:34 PST 2005
//   I made it use GetDataPath to locate the data.
//
//   Brad Whitlock, Mon Feb 25 11:07:24 PDT 2008
//   Changed to new ViewerProxy interface.
//
// ****************************************************************************

public class TryQuery extends RunViewer implements SimpleObserver
{
    public TryQuery()
    {
        super();
        doUpdate = true;

        // Make this object observe the light attributes.
        viewer.GetViewerState().GetQueryAttributes().Attach(this);
    }

    protected void work(String[] args)
    {
        // Try and open a database
        if(viewer.GetViewerMethods().OpenDatabase(viewer.GetDataPath() + "curv2d.silo"))
        {
            viewer.GetViewerMethods().AddPlot("Mesh", "curvmesh2d");
            viewer.GetViewerMethods().AddPlot("Pseudocolor", "d");
            viewer.GetViewerMethods().DrawPlots();

            // Set the colortable to one that has white at the bottom values.
            viewer.GetViewerMethods().SetActiveContinuousColorTable("calewhite");

            // Create the variable list.
            Vector vars = new Vector();
            vars.addElement(new String("default"));

            // Do some picks.
            viewer.GetViewerMethods().Pick(300, 300, vars);
            viewer.GetViewerMethods().Pick(450, 350, vars);
            viewer.GetViewerMethods().Pick(600, 400, vars);

            // Do some lineouts.
            viewer.GetViewerMethods().Lineout(-4.01261, 1.91818, 2.52975, 3.78323, vars);
            viewer.GetViewerMethods().SetActiveWindow(1);
            viewer.GetViewerMethods().Lineout(-3.89903, 1.79309, 2.91593, 3.40794, vars);

            // Change the window layout.
            viewer.GetViewerMethods().SetWindowLayout(2);
        }
        else
            System.out.println("Could not open the database!");
    }

    public void Update(AttributeSubject s)
    {
        QueryAttributes q = (QueryAttributes)s;
        printResults(q);
    }

    public void SetUpdate(boolean val) { doUpdate = val; }
    public boolean GetUpdate() { return doUpdate; }

    protected void printResults(QueryAttributes q)
    {
        System.out.println("Query name="+q.GetName()+
            " resultString="+q.GetResultsMessage());
    }

    public static void main(String args[])
    {
        TryQuery r = new TryQuery();
        r.run(args);
    }

    private boolean doUpdate;
}
