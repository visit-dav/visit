// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

package llnl.visit;

import java.util.Vector;

// ****************************************************************************
// Class: avtDefaultPlotMetaData
//
// Purpose:
//    Contains default plot metadata attributes
//
// Notes:      Autogenerated by xml2java.
//
// Programmer: xml2java
// Creation:   omitted
//
// Modifications:
//   
// ****************************************************************************

public class avtDefaultPlotMetaData extends AttributeSubject
{
    private static int avtDefaultPlotMetaData_numAdditionalAtts = 3;

    public avtDefaultPlotMetaData()
    {
        super(avtDefaultPlotMetaData_numAdditionalAtts);

        pluginID = new String("");
        plotVar = new String("var");
        plotAttributes = new Vector();
    }

    public avtDefaultPlotMetaData(int nMoreFields)
    {
        super(avtDefaultPlotMetaData_numAdditionalAtts + nMoreFields);

        pluginID = new String("");
        plotVar = new String("var");
        plotAttributes = new Vector();
    }

    public avtDefaultPlotMetaData(avtDefaultPlotMetaData obj)
    {
        super(obj);

        int i;

        pluginID = new String(obj.pluginID);
        plotVar = new String(obj.plotVar);
        plotAttributes = new Vector(obj.plotAttributes.size());
        for(i = 0; i < obj.plotAttributes.size(); ++i)
            plotAttributes.addElement(new String((String)obj.plotAttributes.elementAt(i)));


        SelectAll();
    }

    public int Offset()
    {
        return super.Offset() + super.GetNumAdditionalAttributes();
    }

    public int GetNumAdditionalAttributes()
    {
        return avtDefaultPlotMetaData_numAdditionalAtts;
    }

    public boolean equals(avtDefaultPlotMetaData obj)
    {
        int i;

        // Compare the elements in the plotAttributes vector.
        boolean plotAttributes_equal = (obj.plotAttributes.size() == plotAttributes.size());
        for(i = 0; (i < plotAttributes.size()) && plotAttributes_equal; ++i)
        {
            // Make references to String from Object.
            String plotAttributes1 = (String)plotAttributes.elementAt(i);
            String plotAttributes2 = (String)obj.plotAttributes.elementAt(i);
            plotAttributes_equal = plotAttributes1.equals(plotAttributes2);
        }
        // Create the return value
        return ((pluginID.equals(obj.pluginID)) &&
                (plotVar.equals(obj.plotVar)) &&
                plotAttributes_equal);
    }

    // Property setting methods
    public void SetPluginID(String pluginID_)
    {
        pluginID = pluginID_;
        Select(0);
    }

    public void SetPlotVar(String plotVar_)
    {
        plotVar = plotVar_;
        Select(1);
    }

    public void SetPlotAttributes(Vector plotAttributes_)
    {
        plotAttributes = plotAttributes_;
        Select(2);
    }

    // Property getting methods
    public String GetPluginID() { return pluginID; }
    public String GetPlotVar() { return plotVar; }
    public Vector GetPlotAttributes() { return plotAttributes; }

    // Write and read methods.
    public void WriteAtts(CommunicationBuffer buf)
    {
        if(WriteSelect(0, buf))
            buf.WriteString(pluginID);
        if(WriteSelect(1, buf))
            buf.WriteString(plotVar);
        if(WriteSelect(2, buf))
            buf.WriteStringVector(plotAttributes);
    }

    public void ReadAtts(int index, CommunicationBuffer buf)
    {
        switch(index)
        {
        case 0:
            SetPluginID(buf.ReadString());
            break;
        case 1:
            SetPlotVar(buf.ReadString());
            break;
        case 2:
            SetPlotAttributes(buf.ReadStringVector());
            break;
        }
    }

    public String toString(String indent)
    {
        String str = new String();
        str = str + stringToString("pluginID", pluginID, indent) + "\n";
        str = str + stringToString("plotVar", plotVar, indent) + "\n";
        str = str + stringVectorToString("plotAttributes", plotAttributes, indent) + "\n";
        return str;
    }


    // Attributes
    private String pluginID;
    private String plotVar;
    private Vector plotAttributes; // vector of String objects
}

