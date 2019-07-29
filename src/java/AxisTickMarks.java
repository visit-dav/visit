// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

package llnl.visit;


// ****************************************************************************
// Class: AxisTickMarks
//
// Purpose:
//    Contains the tick mark properties for one axis.
//
// Notes:      Autogenerated by xml2java.
//
// Programmer: xml2java
// Creation:   omitted
//
// Modifications:
//   
// ****************************************************************************

public class AxisTickMarks extends AttributeSubject
{
    private static int AxisTickMarks_numAdditionalAtts = 5;

    public AxisTickMarks()
    {
        super(AxisTickMarks_numAdditionalAtts);

        visible = true;
        majorMinimum = 0;
        majorMaximum = 1;
        minorSpacing = 0.02;
        majorSpacing = 0.2;
    }

    public AxisTickMarks(int nMoreFields)
    {
        super(AxisTickMarks_numAdditionalAtts + nMoreFields);

        visible = true;
        majorMinimum = 0;
        majorMaximum = 1;
        minorSpacing = 0.02;
        majorSpacing = 0.2;
    }

    public AxisTickMarks(AxisTickMarks obj)
    {
        super(obj);

        visible = obj.visible;
        majorMinimum = obj.majorMinimum;
        majorMaximum = obj.majorMaximum;
        minorSpacing = obj.minorSpacing;
        majorSpacing = obj.majorSpacing;

        SelectAll();
    }

    public int Offset()
    {
        return super.Offset() + super.GetNumAdditionalAttributes();
    }

    public int GetNumAdditionalAttributes()
    {
        return AxisTickMarks_numAdditionalAtts;
    }

    public boolean equals(AxisTickMarks obj)
    {
        // Create the return value
        return ((visible == obj.visible) &&
                (majorMinimum == obj.majorMinimum) &&
                (majorMaximum == obj.majorMaximum) &&
                (minorSpacing == obj.minorSpacing) &&
                (majorSpacing == obj.majorSpacing));
    }

    // Property setting methods
    public void SetVisible(boolean visible_)
    {
        visible = visible_;
        Select(0);
    }

    public void SetMajorMinimum(double majorMinimum_)
    {
        majorMinimum = majorMinimum_;
        Select(1);
    }

    public void SetMajorMaximum(double majorMaximum_)
    {
        majorMaximum = majorMaximum_;
        Select(2);
    }

    public void SetMinorSpacing(double minorSpacing_)
    {
        minorSpacing = minorSpacing_;
        Select(3);
    }

    public void SetMajorSpacing(double majorSpacing_)
    {
        majorSpacing = majorSpacing_;
        Select(4);
    }

    // Property getting methods
    public boolean GetVisible() { return visible; }
    public double  GetMajorMinimum() { return majorMinimum; }
    public double  GetMajorMaximum() { return majorMaximum; }
    public double  GetMinorSpacing() { return minorSpacing; }
    public double  GetMajorSpacing() { return majorSpacing; }

    // Write and read methods.
    public void WriteAtts(CommunicationBuffer buf)
    {
        if(WriteSelect(0, buf))
            buf.WriteBool(visible);
        if(WriteSelect(1, buf))
            buf.WriteDouble(majorMinimum);
        if(WriteSelect(2, buf))
            buf.WriteDouble(majorMaximum);
        if(WriteSelect(3, buf))
            buf.WriteDouble(minorSpacing);
        if(WriteSelect(4, buf))
            buf.WriteDouble(majorSpacing);
    }

    public void ReadAtts(int index, CommunicationBuffer buf)
    {
        switch(index)
        {
        case 0:
            SetVisible(buf.ReadBool());
            break;
        case 1:
            SetMajorMinimum(buf.ReadDouble());
            break;
        case 2:
            SetMajorMaximum(buf.ReadDouble());
            break;
        case 3:
            SetMinorSpacing(buf.ReadDouble());
            break;
        case 4:
            SetMajorSpacing(buf.ReadDouble());
            break;
        }
    }

    public String toString(String indent)
    {
        String str = new String();
        str = str + boolToString("visible", visible, indent) + "\n";
        str = str + doubleToString("majorMinimum", majorMinimum, indent) + "\n";
        str = str + doubleToString("majorMaximum", majorMaximum, indent) + "\n";
        str = str + doubleToString("minorSpacing", minorSpacing, indent) + "\n";
        str = str + doubleToString("majorSpacing", majorSpacing, indent) + "\n";
        return str;
    }


    // Attributes
    private boolean visible;
    private double  majorMinimum;
    private double  majorMaximum;
    private double  minorSpacing;
    private double  majorSpacing;
}

