// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

package llnl.visit;

import java.util.Vector;

// ****************************************************************************
// Class: PlotList
//
// Purpose:
//    This class contains a list of plots.
//
// Notes:      Autogenerated by xml2java.
//
// Programmer: xml2java
// Creation:   omitted
//
// Modifications:
//
// ****************************************************************************

public class PlotList extends AttributeSubject
{
    private static int PlotList_numAdditionalAtts = 1;

    public PlotList()
    {
        super(PlotList_numAdditionalAtts);

        plots = new Vector();
    }

    public PlotList(int nMoreFields)
    {
        super(PlotList_numAdditionalAtts + nMoreFields);

        plots = new Vector();
    }

    public PlotList(PlotList obj)
    {
        super(obj);

        int i;

        // *** Copy the plots field ***
        plots = new Vector(obj.plots.size());
        for(i = 0; i < obj.plots.size(); ++i)
        {
            Plot oldObj = (Plot)obj.plots.elementAt(i);
            plots.addElement(new Plot(oldObj));
        }


        SelectAll();
    }

    public int Offset()
    {
        return super.Offset() + super.GetNumAdditionalAttributes();
    }

    public int GetNumAdditionalAttributes()
    {
        return PlotList_numAdditionalAtts;
    }

    public boolean equals(PlotList obj)
    {
        int i;

        // Compare the elements in the plots vector.
        boolean plots_equal = (obj.plots.size() == plots.size());
        for(i = 0; (i < plots.size()) && plots_equal; ++i)
        {
            // Make references to Plot from Object.
            Plot plots1 = (Plot)plots.elementAt(i);
            Plot plots2 = (Plot)obj.plots.elementAt(i);
            plots_equal = plots1.equals(plots2);
        }
        // Create the return value
        return (plots_equal);
    }

    // Property setting methods
    // Property getting methods
    public Vector GetPlots() { return plots; }

    // Write and read methods.
    public void WriteAtts(CommunicationBuffer buf)
    {
        if(WriteSelect(0, buf))
        {
            buf.WriteInt(plots.size());
            for(int i = 0; i < plots.size(); ++i)
            {
                Plot tmp = (Plot)plots.elementAt(i);
                tmp.Write(buf);
            }
        }
    }

    public void ReadAtts(int index, CommunicationBuffer buf)
    {
        {
            int len = buf.ReadInt();
            plots.clear();
            for(int j = 0; j < len; ++j)
            {
                Plot tmp = new Plot();
                tmp.Read(buf);
                plots.addElement(tmp);
            }
        }
        Select(0);
    }

    public String toString(String indent)
    {
        String str = new String();
        str = str + indent + "plots = {\n";
        for(int i = 0; i < plots.size(); ++i)
        {
            AttributeSubject s = (AttributeSubject)plots.elementAt(i);
            str = str + s.toString(indent + "    ");
            if(i < plots.size()-1)
                str = str + ", ";
            str = str + "\n";
        }
        str = str + "}\n";
        return str;
    }

    // Attributegroup convenience methods
    public void AddPlots(Plot obj)
    {
        plots.addElement(new Plot(obj));
        Select(0);
    }

    public void ClearPlots()
    {
        plots.clear();
        Select(0);
    }

    public void RemovePlots(int index)
    {
        if(index >= 0 && index < plots.size())
        {
            plots.remove(index);
            Select(0);
        }
    }

    public int GetNumPlots()
    {
        return plots.size();
    }

    public Plot GetPlots(int i)
    {
        Plot tmp = (Plot)plots.elementAt(i);
        return tmp;
    }


    // Attributes
    private Vector plots; // vector of Plot objects
}

