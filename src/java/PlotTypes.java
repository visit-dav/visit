// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

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

    // The Plugin interface should have a GetVariableTypes method.
    // For now, this will suffice.
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
        namestovar.put("MultiCurve", new Integer(CURVE));
        namestovar.put("ParallelCoordinates", new Integer(0)); //SCALAR | ARRAY));
        namestovar.put("Poincare", new Integer(VECTOR));
        namestovar.put("Pseudocolor", new Integer(SCALAR | SPECIES));
        namestovar.put("Scatter", new Integer(SCALAR));
        namestovar.put("Spreadsheet", new Integer(SCALAR));
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
