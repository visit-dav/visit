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

import llnl.visit.ColorAttribute;
import llnl.visit.plots.MeshAttributes;

// ****************************************************************************
// Class: TryPointGlyphing
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

public class TryPointGlyphing extends RunViewer
{
    public TryPointGlyphing()
    {
        super();
    }

    protected void work(String[] args)
    {
        // Try and open a database
        if(viewer.GetViewerMethods().OpenDatabase(viewer.GetDataPath() + "noise.silo"))
        {
            viewer.GetViewerMethods().AddPlot("Mesh", "PointMesh");
            viewer.GetViewerMethods().DrawPlots();

            // Set the pseudocolor attributes
            MeshAttributes m = (MeshAttributes)viewer.GetPlotAttributes("Mesh");
            m.SetPointSize(1.0); 
            m.SetPointSizePixels(5); 

            m.SetMeshColorSource(1);
            m.SetMeshColor(new ColorAttribute(204,153,255,0));
            // Run through all the point types
            for (int i = 0; i < 8; ++i)
            {
                m.SetPointType(i);
                m.Notify();
                viewer.GetViewerMethods().SetPlotOptions("Mesh");

                viewer.GetViewerMethods().SaveWindow();
            }
        }
        else
            System.out.println("Could not open the database!");
    }


    public static void main(String args[])
    {
        TryPointGlyphing r = new TryPointGlyphing();
        r.run(args);
    }
}
