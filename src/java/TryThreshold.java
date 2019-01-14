// ****************************************************************************
//
// Copyright (c) 2000 - 2019, Lawrence Livermore National Security, LLC
// Produced at the Lawrence Livermore National Laboratory
// LLNL-CODE-442911
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

import java.util.Vector;
import llnl.visit.AttributeSubject;
import llnl.visit.View3DAttributes;

import llnl.visit.plots.PseudocolorAttributes;
import llnl.visit.operators.ThresholdAttributes;

// ****************************************************************************
// Class: TryThreshold
//
// Purpose:
//   This example program sets up a Pseudocolor plot with threshold operator.
//
// Notes:      Based on threshold.py of test-suite.
//
// Programmer: Kathleen Biagas 
// Creation:   March 31, 2017 
//
// Modifications:
//
// ****************************************************************************

public class TryThreshold extends RunViewer
{
    public TryThreshold()
    {
        super();
    }

    protected void work(String[] args)
    {
        // Try and open a database
        if(viewer.GetViewerMethods().OpenDatabase(viewer.GetDataPath() + "globe.silo"))
        {
            viewer.GetViewerMethods().AddPlot("Pseudocolor", "u");
            viewer.GetViewerMethods().AddOperator("Threshold");

            // Set the pseudocolor attributes
            ThresholdAttributes t = (ThresholdAttributes)viewer.GetOperatorAttributes("Threshold");
            t.SetOutputMeshType(0);
            Vector zp = new Vector();
            zp.add(1);
            t.SetZonePortions(zp);
            Vector lb = new Vector();
            lb.add(-4.0);
            t.SetLowerBounds(lb);
            Vector ub = new Vector();
            ub.add(4.0);
            t.SetUpperBounds(ub);
            t.Notify();
            viewer.GetViewerMethods().SetOperatorOptions("Threshold");
            viewer.GetViewerMethods().DrawPlots();

            // Set the view
            View3DAttributes v = viewer.GetViewerState().GetView3DAttributes();
            v.SetViewNormal(-0.528889, 0.367702, 0.7649);
            v.SetViewUp(0.176641, 0.929226, -0.324558);
            v.SetParallelScale(17.3205);
            v.SetPerspective(true);
            v.Notify();
            viewer.GetViewerMethods().SetView3D();

            viewer.GetViewerMethods().SaveWindow();

            // Change zone inclusion criteria
            zp.set(0, 0);
            t.SetZonePortions(zp);
            t.Notify();
            viewer.GetViewerMethods().SetOperatorOptions("Threshold");
            viewer.GetViewerMethods().SaveWindow();

            //  Threshold by a variable different than the PC coloring variable.

            zp.set(0, 1);
            t.SetZonePortions(zp);
            lb.set(0, 140.0);
            t.SetLowerBounds(lb);
            ub.set(0, 340.0);
            t.SetUpperBounds(ub);
            Vector vn = new Vector();
            vn.add("t");
            t.SetListedVarNames(vn);
            t.Notify();
            viewer.GetViewerMethods().SetOperatorOptions("Threshold");
            viewer.GetViewerMethods().SaveWindow();

        }
        else
            System.out.println("Could not open the database!");
    }


    public static void main(String args[])
    {
        TryThreshold r = new TryThreshold();
        r.run(args);
    }
}
