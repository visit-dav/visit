// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

package llnl.visit;


// ****************************************************************************
// Class: AxisLabels
//
// Purpose:
//    Contains the label properties for one axis.
//
// Notes:      Autogenerated by xml2java.
//
// Programmer: xml2java
// Creation:   omitted
//
// Modifications:
//   
// ****************************************************************************

public class AxisLabels extends AttributeSubject
{
    private static int AxisLabels_numAdditionalAtts = 3;

    public AxisLabels()
    {
        super(AxisLabels_numAdditionalAtts);

        visible = true;
        font = new FontAttributes();
        scaling = 0;
    }

    public AxisLabels(int nMoreFields)
    {
        super(AxisLabels_numAdditionalAtts + nMoreFields);

        visible = true;
        font = new FontAttributes();
        scaling = 0;
    }

    public AxisLabels(AxisLabels obj)
    {
        super(obj);

        visible = obj.visible;
        font = new FontAttributes(obj.font);
        scaling = obj.scaling;

        SelectAll();
    }

    public int Offset()
    {
        return super.Offset() + super.GetNumAdditionalAttributes();
    }

    public int GetNumAdditionalAttributes()
    {
        return AxisLabels_numAdditionalAtts;
    }

    public boolean equals(AxisLabels obj)
    {
        // Create the return value
        return ((visible == obj.visible) &&
                (font.equals(obj.font)) &&
                (scaling == obj.scaling));
    }

    // Property setting methods
    public void SetVisible(boolean visible_)
    {
        visible = visible_;
        Select(0);
    }

    public void SetFont(FontAttributes font_)
    {
        font = font_;
        Select(1);
    }

    public void SetScaling(int scaling_)
    {
        scaling = scaling_;
        Select(2);
    }

    // Property getting methods
    public boolean        GetVisible() { return visible; }
    public FontAttributes GetFont() { return font; }
    public int            GetScaling() { return scaling; }

    // Write and read methods.
    public void WriteAtts(CommunicationBuffer buf)
    {
        if(WriteSelect(0, buf))
            buf.WriteBool(visible);
        if(WriteSelect(1, buf))
            font.Write(buf);
        if(WriteSelect(2, buf))
            buf.WriteInt(scaling);
    }

    public void ReadAtts(int index, CommunicationBuffer buf)
    {
        switch(index)
        {
        case 0:
            SetVisible(buf.ReadBool());
            break;
        case 1:
            font.Read(buf);
            Select(1);
            break;
        case 2:
            SetScaling(buf.ReadInt());
            break;
        }
    }

    public String toString(String indent)
    {
        String str = new String();
        str = str + boolToString("visible", visible, indent) + "\n";
        str = str + indent + "font = {\n" + font.toString(indent + "    ") + indent + "}\n";
        str = str + intToString("scaling", scaling, indent) + "\n";
        return str;
    }


    // Attributes
    private boolean        visible;
    private FontAttributes font;
    private int            scaling;
}

