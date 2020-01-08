// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

package llnl.visit.operators;

import llnl.visit.AttributeSubject;
import llnl.visit.CommunicationBuffer;
import llnl.visit.Plugin;

// ****************************************************************************
// Class: SubdivideQuadsAttributes
//
// Purpose:
//    Attributes for SubdivideQuads operator
//
// Notes:      Autogenerated by xml2java.
//
// Programmer: xml2java
// Creation:   omitted
//
// Modifications:
//
// ****************************************************************************

public class SubdivideQuadsAttributes extends AttributeSubject implements Plugin
{
    private static int SubdivideQuadsAttributes_numAdditionalAtts = 5;

    public SubdivideQuadsAttributes()
    {
        super(SubdivideQuadsAttributes_numAdditionalAtts);

        threshold = 0.500002;
        maxSubdivs = 4;
        fanOutPoints = true;
        doTriangles = false;
        variable = new String("default");
    }

    public SubdivideQuadsAttributes(int nMoreFields)
    {
        super(SubdivideQuadsAttributes_numAdditionalAtts + nMoreFields);

        threshold = 0.500002;
        maxSubdivs = 4;
        fanOutPoints = true;
        doTriangles = false;
        variable = new String("default");
    }

    public SubdivideQuadsAttributes(SubdivideQuadsAttributes obj)
    {
        super(obj);

        threshold = obj.threshold;
        maxSubdivs = obj.maxSubdivs;
        fanOutPoints = obj.fanOutPoints;
        doTriangles = obj.doTriangles;
        variable = new String(obj.variable);

        SelectAll();
    }

    public int Offset()
    {
        return super.Offset() + super.GetNumAdditionalAttributes();
    }

    public int GetNumAdditionalAttributes()
    {
        return SubdivideQuadsAttributes_numAdditionalAtts;
    }

    public boolean equals(SubdivideQuadsAttributes obj)
    {
        // Create the return value
        return ((threshold == obj.threshold) &&
                (maxSubdivs == obj.maxSubdivs) &&
                (fanOutPoints == obj.fanOutPoints) &&
                (doTriangles == obj.doTriangles) &&
                (variable.equals(obj.variable)));
    }

    public String GetName() { return "SubdivideQuads"; }
    public String GetVersion() { return "1.0"; }

    // Property setting methods
    public void SetThreshold(double threshold_)
    {
        threshold = threshold_;
        Select(0);
    }

    public void SetMaxSubdivs(int maxSubdivs_)
    {
        maxSubdivs = maxSubdivs_;
        Select(1);
    }

    public void SetFanOutPoints(boolean fanOutPoints_)
    {
        fanOutPoints = fanOutPoints_;
        Select(2);
    }

    public void SetDoTriangles(boolean doTriangles_)
    {
        doTriangles = doTriangles_;
        Select(3);
    }

    public void SetVariable(String variable_)
    {
        variable = variable_;
        Select(4);
    }

    // Property getting methods
    public double  GetThreshold() { return threshold; }
    public int     GetMaxSubdivs() { return maxSubdivs; }
    public boolean GetFanOutPoints() { return fanOutPoints; }
    public boolean GetDoTriangles() { return doTriangles; }
    public String  GetVariable() { return variable; }

    // Write and read methods.
    public void WriteAtts(CommunicationBuffer buf)
    {
        if(WriteSelect(0, buf))
            buf.WriteDouble(threshold);
        if(WriteSelect(1, buf))
            buf.WriteInt(maxSubdivs);
        if(WriteSelect(2, buf))
            buf.WriteBool(fanOutPoints);
        if(WriteSelect(3, buf))
            buf.WriteBool(doTriangles);
        if(WriteSelect(4, buf))
            buf.WriteString(variable);
    }

    public void ReadAtts(int index, CommunicationBuffer buf)
    {
        switch(index)
        {
        case 0:
            SetThreshold(buf.ReadDouble());
            break;
        case 1:
            SetMaxSubdivs(buf.ReadInt());
            break;
        case 2:
            SetFanOutPoints(buf.ReadBool());
            break;
        case 3:
            SetDoTriangles(buf.ReadBool());
            break;
        case 4:
            SetVariable(buf.ReadString());
            break;
        }
    }

    public String toString(String indent)
    {
        String str = new String();
        str = str + doubleToString("threshold", threshold, indent) + "\n";
        str = str + intToString("maxSubdivs", maxSubdivs, indent) + "\n";
        str = str + boolToString("fanOutPoints", fanOutPoints, indent) + "\n";
        str = str + boolToString("doTriangles", doTriangles, indent) + "\n";
        str = str + stringToString("variable", variable, indent) + "\n";
        return str;
    }


    // Attributes
    private double  threshold;
    private int     maxSubdivs;
    private boolean fanOutPoints;
    private boolean doTriangles;
    private String  variable;
}

