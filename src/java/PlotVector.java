// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

import llnl.visit.ViewerProxy;
import llnl.visit.Expression;
import llnl.visit.ExpressionList;

public class PlotVector extends RunViewer
{
    public PlotVector()
    {
        super();
    }
 
    protected void work(String[] args)
    {
        if(viewer.GetViewerMethods().OpenDatabase(viewer.GetDataPath() + "globe.silo"))
        {
            ExpressionList explist = viewer.GetViewerState().GetExpressionList();
            Expression e = new Expression();
            e.SetName("disp");
            e.SetType(Expression.EXPRTYPE_VECTORMESHVAR);
            e.SetDefinition("{speed,u,v} - coord(mesh1)");
            explist.AddExpressions(e);
            explist.Notify();
            viewer.GetViewerMethods().ProcessExpressions();

            // Add a plot of the vector
            viewer.GetViewerMethods().AddPlot("Vector", "disp");
            viewer.GetViewerMethods().AddOperator("Displace");
            viewer.GetViewerMethods().DrawPlots();
        }
        else
        {
            System.out.println("Could not open the database!");
        }
    }
 
    public static void main(String args[])
    {
        PlotVector r = new PlotVector();
        r.run(args);
    }
}
