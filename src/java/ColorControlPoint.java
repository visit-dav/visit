// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

package llnl.visit;


// ****************************************************************************
// Class: ColorControlPoint
//
// Purpose:
//    This class contains an RGBA color with a position value.
//
// Notes:      Autogenerated by xml2java.
//
// Programmer: xml2java
// Creation:   omitted
//
// Modifications:
//   
// ****************************************************************************

public class ColorControlPoint extends AttributeSubject
{
    private static int ColorControlPoint_numAdditionalAtts = 2;

    public ColorControlPoint()
    {
        super(ColorControlPoint_numAdditionalAtts);

        colors = new byte[4];
        colors[0] = (byte)0;
        colors[1] = (byte)0;
        colors[2] = (byte)0;
        colors[3] = (byte)255;
        position = 0f;
    }

    public ColorControlPoint(int nMoreFields)
    {
        super(ColorControlPoint_numAdditionalAtts + nMoreFields);

        colors = new byte[4];
        colors[0] = (byte)0;
        colors[1] = (byte)0;
        colors[2] = (byte)0;
        colors[3] = (byte)255;
        position = 0f;
    }

    public ColorControlPoint(ColorControlPoint obj)
    {
        super(obj);

        int i;

        colors = new byte[4];
        for(i = 0; i < obj.colors.length; ++i)
            colors[i] = obj.colors[i];

        position = obj.position;

        SelectAll();
    }

    public int Offset()
    {
        return super.Offset() + super.GetNumAdditionalAttributes();
    }

    public int GetNumAdditionalAttributes()
    {
        return ColorControlPoint_numAdditionalAtts;
    }

    public boolean equals(ColorControlPoint obj)
    {
        int i;

        // Compare the colors arrays.
        boolean colors_equal = true;
        for(i = 0; i < 4 && colors_equal; ++i)
            colors_equal = (colors[i] == obj.colors[i]);

        // Create the return value
        return (colors_equal &&
                (position == obj.position));
    }

    // Property setting methods
    public void SetColors(byte[] colors_)
    {
        colors[0] = colors_[0];
        colors[1] = colors_[1];
        colors[2] = colors_[2];
        colors[3] = colors_[3];
        Select(0);
    }

    public void SetColors(byte e0, byte e1, byte e2, byte e3)
    {
        colors[0] = e0;
        colors[1] = e1;
        colors[2] = e2;
        colors[3] = e3;
        Select(0);
    }

    public void SetPosition(float position_)
    {
        position = position_;
        Select(1);
    }

    // Property getting methods
    public byte[] GetColors() { return colors; }
    public float  GetPosition() { return position; }

    // Write and read methods.
    public void WriteAtts(CommunicationBuffer buf)
    {
        if(WriteSelect(0, buf))
            buf.WriteByteArray(colors, true);
        if(WriteSelect(1, buf))
            buf.WriteFloat(position);
    }

    public void ReadAtts(int index, CommunicationBuffer buf)
    {
        switch(index)
        {
        case 0:
            SetColors(buf.ReadByteArray());
            break;
        case 1:
            SetPosition(buf.ReadFloat());
            break;
        }
    }

    public String toString(String indent)
    {
        String str = new String();
        str = str + ucharArrayToString("colors", colors, indent) + "\n";
        str = str + floatToString("position", position, indent) + "\n";
        return str;
    }


    // Attributes
    private byte[] colors;
    private float  position;
}

