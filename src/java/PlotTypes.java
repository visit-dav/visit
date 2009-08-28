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

import java.util.Hashtable;
import llnl.visit.avtDatabaseMetaData;

// ****************************************************************************
// Class: PlotTypes
//
// Purpose:
//   This is an example program that shows how to determine which plots can
//   accept variables from a file.
//
// Notes:      
//
// Programmer: Brad Whitlock
// Creation:   Fri Aug 28 09:21:06 PDT 2009
//
// Modifications:
//
// ****************************************************************************

public class PlotTypes extends RunViewer
{
    public PlotTypes()
    {
        super();
        savePlots = false;
    }

    public final static int MESH              = 0x0001;
    public final static int SCALAR            = 0x0002;
    public final static int MATERIAL          = 0x0004;
    public final static int VECTOR            = 0x0008;
    public final static int SUBSET            = 0x0010;
    public final static int SPECIES           = 0x0020;
    public final static int CURVE             = 0x0040;
    public final static int TENSOR            = 0x0080;
    public final static int SYMMETRICTENSOR   = 0x0100;
    public final static int LABEL             = 0x0200;
    public final static int ARRAY             = 0x0400;

    // The Plugin interface should have a GetVariableTypes method but I
    // didn't do it at the time (do for 2.0). For now, this will suffice.
    public int GetVariableTypes(String plotName)
    {
        Hashtable namestovar = new Hashtable();
        namestovar.put("Boundary", new Integer(MATERIAL));
        namestovar.put("Contour", new Integer(SCALAR | SPECIES));
        namestovar.put("Curve", new Integer(CURVE));
        namestovar.put("FilledBoundary", new Integer(MATERIAL));
        namestovar.put("Histogram", new Integer(SCALAR | ARRAY));
        namestovar.put("Kerbel", new Integer(MESH));
        namestovar.put("Label", new Integer(MESH | SCALAR | VECTOR | MATERIAL | SUBSET | TENSOR | SYMMETRICTENSOR | LABEL | ARRAY));
        namestovar.put("Mesh", new Integer(MESH));
        namestovar.put("Molecule", new Integer(SCALAR));
        namestovar.put("ParallelCoordinates", new Integer(0)); //SCALAR | ARRAY));
        namestovar.put("Poincare", new Integer(VECTOR));
        namestovar.put("Pseudocolor", new Integer(SCALAR | SPECIES));
        namestovar.put("Scatter", new Integer(SCALAR));
        namestovar.put("Spreadsheet", new Integer(SCALAR));
        namestovar.put("Streamline", new Integer(VECTOR));
        namestovar.put("Subset", new Integer(SUBSET | MESH));
        namestovar.put("Surface", new Integer(SCALAR | SPECIES));
        namestovar.put("Tensor", new Integer(TENSOR | SYMMETRICTENSOR));
        namestovar.put("Topology", new Integer(SCALAR));
        namestovar.put("Truecolor", new Integer(VECTOR));
        namestovar.put("Vector", new Integer(VECTOR));
        namestovar.put("Volume", new Integer(SCALAR | SPECIES));
        namestovar.put("WellBore", new Integer(MESH));

        return ((Integer)namestovar.get(plotName)).intValue();
    }

    protected void savePlot(String plotType, String var)
    {
        System.out.println(var);
        if(savePlots)
        {
            viewer.GetViewerMethods().AddPlot(plotType, var);
            viewer.GetViewerMethods().DrawPlots();
            viewer.GetViewerMethods().ResetView();
            viewer.GetViewerMethods().SaveWindow();
            viewer.GetViewerMethods().DeleteActivePlots();
        }
    }

    protected void work(String[] args)
    {
        System.out.println("Plots\n==================================================");
        for(int i = 0; i < viewer.GetNumPlotPlugins(); ++i)
            System.out.println("Plot "+i+": name="+viewer.GetPlotName(i)+", version="+viewer.GetPlotVersion(i));

        System.out.println("Operators\n==================================================");
        for(int i = 0; i < viewer.GetNumOperatorPlugins(); ++i)
            System.out.println("Operator "+i+": name="+viewer.GetOperatorName(i)+", version="+viewer.GetOperatorVersion(i));

        String db = new String(viewer.GetDataPath() + "noise.silo");
        if(viewer.GetViewerMethods().RequestMetaData(db, 0))
        {
            if(savePlots)
                viewer.GetViewerMethods().OpenDatabase(db);

            avtDatabaseMetaData md = viewer.GetViewerState().GetDatabaseMetaData();
            for(int i = 0; i < viewer.GetNumPlotPlugins(); ++i)
            {
                System.out.println("\n"+viewer.GetPlotName(i) + " can accept variables:\n=====================================");
                int vartypes = GetVariableTypes(viewer.GetPlotName(i));
                if((vartypes & MESH) > 0)
                {
                    for(int j = 0; j < md.GetNumMeshes(); ++j)
                        savePlot(viewer.GetPlotName(i), md.GetMeshes(j).GetName());
                }
                if((vartypes & SCALAR) > 0)
                {
                    for(int j = 0; j < md.GetNumScalars(); ++j)
                        savePlot(viewer.GetPlotName(i), md.GetScalars(j).GetName());
                }
                if((vartypes & MATERIAL) > 0)
                {
                    for(int j = 0; j < md.GetNumMaterials(); ++j)
                        savePlot(viewer.GetPlotName(i), md.GetMaterials(j).GetName());
                }
                if((vartypes & VECTOR) > 0)
                {
                    for(int j = 0; j < md.GetNumVectors(); ++j)
                        savePlot(viewer.GetPlotName(i), md.GetVectors(j).GetName());
                }
                if((vartypes & SPECIES) > 0)
                {
                    for(int j = 0; j < md.GetNumSpecies(); ++j)
                        savePlot(viewer.GetPlotName(i), md.GetSpecies(j).GetName());
                }
                if((vartypes & CURVE) > 0)
                {
                    for(int j = 0; j < md.GetNumCurves(); ++j)
                        savePlot(viewer.GetPlotName(i), md.GetCurves(j).GetName());
                }
                if((vartypes & TENSOR) > 0)
                {
                    for(int j = 0; j < md.GetNumTensors(); ++j)
                        savePlot(viewer.GetPlotName(i), md.GetTensors(j).GetName());
                }
                if((vartypes & SYMMETRICTENSOR) > 0)
                {
                    for(int j = 0; j < md.GetNumSymmTensors(); ++j)
                        savePlot(viewer.GetPlotName(i), md.GetSymmTensors(j).GetName());
                }
                if((vartypes & LABEL) > 0)
                {
                    for(int j = 0; j < md.GetNumLabels(); ++j)
                        savePlot(viewer.GetPlotName(i), md.GetLabels(j).GetName());
                }
                if((vartypes & ARRAY) > 0)
                {
                    for(int j = 0; j < md.GetNumArrays(); ++j)
                        savePlot(viewer.GetPlotName(i), md.GetArrays(j).GetName());
                }
            }
        }
        else
            System.out.println("Could not open the database!");
    }

    public static void main(String args[])
    {
        PlotTypes r = new PlotTypes();

        for(int i = 0; i < args.length; ++i)
        {
            if(args[i].equals("-plot"))
                r.savePlots = true;
        }

        r.run(args);
    }

    private boolean savePlots;
}
