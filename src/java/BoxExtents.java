// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

package llnl.visit;


// ****************************************************************************
// Class: BoxExtents
//
// Purpose:
//    Attributes for an axis-aligned box
//
// Notes:      Autogenerated by xml2java.
//
// Programmer: xml2java
// Creation:   omitted
//
// Modifications:
//   
// ****************************************************************************

public class BoxExtents extends AttributeSubject
{
    private static int BoxExtents_numAdditionalAtts = 1;

    public BoxExtents()
    {
        super(BoxExtents_numAdditionalAtts);

        extents = new double[6];
        extents[0] = 0;
        extents[1] = 0;
        extents[2] = 0;
        extents[3] = 0;
        extents[4] = 0;
        extents[5] = 0;
    }

    public BoxExtents(int nMoreFields)
    {
        super(BoxExtents_numAdditionalAtts + nMoreFields);

        extents = new double[6];
        extents[0] = 0;
        extents[1] = 0;
        extents[2] = 0;
        extents[3] = 0;
        extents[4] = 0;
        extents[5] = 0;
    }

    public BoxExtents(BoxExtents obj)
    {
        super(obj);

        int i;

        extents = new double[6];
        for(i = 0; i < obj.extents.length; ++i)
            extents[i] = obj.extents[i];


        SelectAll();
    }

    public int Offset()
    {
        return super.Offset() + super.GetNumAdditionalAttributes();
    }

    public int GetNumAdditionalAttributes()
    {
        return BoxExtents_numAdditionalAtts;
    }

    public boolean equals(BoxExtents obj)
    {
        int i;

        // Compare the extents arrays.
        boolean extents_equal = true;
        for(i = 0; i < 6 && extents_equal; ++i)
            extents_equal = (extents[i] == obj.extents[i]);

        // Create the return value
        return (extents_equal);
    }

    // Property setting methods
    public void SetExtents(double[] extents_)
    {
        for(int i = 0; i < 6; ++i)
             extents[i] = extents_[i];
        Select(0);
    }

    // Property getting methods
    public double[] GetExtents() { return extents; }

    // Write and read methods.
    public void WriteAtts(CommunicationBuffer buf)
    {
        if(WriteSelect(0, buf))
            buf.WriteDoubleArray(extents);
    }

    public void ReadAtts(int index, CommunicationBuffer buf)
    {
        SetExtents(buf.ReadDoubleArray());
    }

    public String toString(String indent)
    {
        String str = new String();
        str = str + doubleArrayToString("extents", extents, indent) + "\n";
        return str;
    }


    // Attributes
    private double[] extents;
}

