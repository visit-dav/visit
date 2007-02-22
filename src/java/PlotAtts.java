// ****************************************************************************
//
// Copyright (c) 2000 - 2007, The Regents of the University of California
// Produced at the Lawrence Livermore National Laboratory
// All rights reserved.
//
// This file is part of VisIt. For details, see http://www.llnl.gov/visit/. The
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
//    documentation and/or materials provided with the distribution.
//  - Neither the name of the UC/LLNL nor  the names of its contributors may be
//    used to  endorse or  promote products derived from  this software without
//    specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT  HOLDERS AND CONTRIBUTORS "AS IS"
// AND ANY EXPRESS OR  IMPLIED WARRANTIES, INCLUDING,  BUT NOT  LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND  FITNESS FOR A PARTICULAR  PURPOSE
// ARE  DISCLAIMED.  IN  NO  EVENT  SHALL  THE  REGENTS  OF  THE  UNIVERSITY OF
// CALIFORNIA, THE U.S.  DEPARTMENT  OF  ENERGY OR CONTRIBUTORS BE  LIABLE  FOR
// ANY  DIRECT,  INDIRECT,  INCIDENTAL,  SPECIAL,  EXEMPLARY,  OR CONSEQUENTIAL
// DAMAGES (INCLUDING, BUT NOT  LIMITED TO, PROCUREMENT OF  SUBSTITUTE GOODS OR
// SERVICES; LOSS OF  USE, DATA, OR PROFITS; OR  BUSINESS INTERRUPTION) HOWEVER
// CAUSED  AND  ON  ANY  THEORY  OF  LIABILITY,  WHETHER  IN  CONTRACT,  STRICT
// LIABILITY, OR TORT  (INCLUDING NEGLIGENCE OR OTHERWISE)  ARISING IN ANY  WAY
// OUT OF THE  USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH
// DAMAGE.
//
// ****************************************************************************


import llnl.visit.View3DAttributes;
import llnl.visit.plots.PseudocolorAttributes;

// ****************************************************************************
// Class: PlotAtts
//
// Purpose:
//   This is an example program that shows how to set plot attributes.
//
// Notes:      
//
// Programmer: Brad Whitlock
// Creation:   Thu Aug 15 16:09:03 PST 2002
//
// Modifications:
//   Brad Whitlock, Tue Sep 24 08:05:51 PDT 2002
//   I changed it so the view is set after the plot is drawn.
//
//   Eric Brugger, Wed Aug 27 09:04:55 PDT 2003
//   I modified it to use the new view interface.
//
//   Brad Whitlock, Mon Jun 6 17:25:34 PST 2005
//   I made it use GetDataPath to locate the data.
//
//   Brad Whitlock, Thu Jul 14 12:15:42 PDT 2005
//   Updated.
//
// ****************************************************************************

public class PlotAtts extends RunViewer
{
    public PlotAtts()
    {
        super();
    }

    protected void work(String[] args)
    {
        if(viewer.OpenDatabase(viewer.GetDataPath() + "globe.silo"))
        {
            // Create a plot.
            viewer.AddPlot("Pseudocolor", "u");

            // Set the pseudocolor attributes
            PseudocolorAttributes p = (PseudocolorAttributes)viewer.GetPlotAttributes("Pseudocolor");
            p.SetOpacity(0.3);
            p.Notify();
            viewer.SetPlotOptions("Pseudocolor");

            // Draw the plot
            viewer.DrawPlots();

            // Set the view
            View3DAttributes v = viewer.GetView3D();
            v.SetViewNormal(0.456808, 0.335583, 0.823839);
            v.SetFocus(-0.927295, -1.22113, 1.01159);
            v.SetViewUp(-0.184554, 0.941716, -0.281266);
            v.SetParallelScale(15.7041);
            v.SetNearPlane(-34.641);
            v.SetFarPlane(34.641);
            v.Notify();
            viewer.SetView3D();
        }
        else
            System.out.println("Could not open the database!");
    }

    public static void main(String args[])
    {
        PlotAtts r = new PlotAtts();
        r.run(args);
    }
}
