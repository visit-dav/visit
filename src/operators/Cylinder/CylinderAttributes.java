// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

package llnl.visit.operators;

import llnl.visit.AttributeSubject;
import llnl.visit.CommunicationBuffer;
import llnl.visit.Plugin;

// ****************************************************************************
// Class: CylinderAttributes
//
// Purpose:
//    Contain the attributes for a cylinder
//
// Notes:      Autogenerated by xml2java.
//
// Programmer: xml2java
// Creation:   omitted
//
// Modifications:
//   
// ****************************************************************************

public class CylinderAttributes extends AttributeSubject implements Plugin
{
    private static int CylinderAttributes_numAdditionalAtts = 4;

    public CylinderAttributes()
    {
        super(CylinderAttributes_numAdditionalAtts);

        point1 = new double[3];
        point1[0] = 0;
        point1[1] = 0;
        point1[2] = 0;
        point2 = new double[3];
        point2[0] = 1;
        point2[1] = 0;
        point2[2] = 0;
        radius = 1;
        inverse = false;
    }

    public CylinderAttributes(int nMoreFields)
    {
        super(CylinderAttributes_numAdditionalAtts + nMoreFields);

        point1 = new double[3];
        point1[0] = 0;
        point1[1] = 0;
        point1[2] = 0;
        point2 = new double[3];
        point2[0] = 1;
        point2[1] = 0;
        point2[2] = 0;
        radius = 1;
        inverse = false;
    }

    public CylinderAttributes(CylinderAttributes obj)
    {
        super(obj);

        int i;

        point1 = new double[3];
        point1[0] = obj.point1[0];
        point1[1] = obj.point1[1];
        point1[2] = obj.point1[2];

        point2 = new double[3];
        point2[0] = obj.point2[0];
        point2[1] = obj.point2[1];
        point2[2] = obj.point2[2];

        radius = obj.radius;
        inverse = obj.inverse;

        SelectAll();
    }

    public int Offset()
    {
        return super.Offset() + super.GetNumAdditionalAttributes();
    }

    public int GetNumAdditionalAttributes()
    {
        return CylinderAttributes_numAdditionalAtts;
    }

    public boolean equals(CylinderAttributes obj)
    {
        int i;

        // Compare the point1 arrays.
        boolean point1_equal = true;
        for(i = 0; i < 3 && point1_equal; ++i)
            point1_equal = (point1[i] == obj.point1[i]);

        // Compare the point2 arrays.
        boolean point2_equal = true;
        for(i = 0; i < 3 && point2_equal; ++i)
            point2_equal = (point2[i] == obj.point2[i]);

        // Create the return value
        return (point1_equal &&
                point2_equal &&
                (radius == obj.radius) &&
                (inverse == obj.inverse));
    }

    public String GetName() { return "Cylinder"; }
    public String GetVersion() { return "1.0"; }

    // Property setting methods
    public void SetPoint1(double[] point1_)
    {
        point1[0] = point1_[0];
        point1[1] = point1_[1];
        point1[2] = point1_[2];
        Select(0);
    }

    public void SetPoint1(double e0, double e1, double e2)
    {
        point1[0] = e0;
        point1[1] = e1;
        point1[2] = e2;
        Select(0);
    }

    public void SetPoint2(double[] point2_)
    {
        point2[0] = point2_[0];
        point2[1] = point2_[1];
        point2[2] = point2_[2];
        Select(1);
    }

    public void SetPoint2(double e0, double e1, double e2)
    {
        point2[0] = e0;
        point2[1] = e1;
        point2[2] = e2;
        Select(1);
    }

    public void SetRadius(double radius_)
    {
        radius = radius_;
        Select(2);
    }

    public void SetInverse(boolean inverse_)
    {
        inverse = inverse_;
        Select(3);
    }

    // Property getting methods
    public double[] GetPoint1() { return point1; }
    public double[] GetPoint2() { return point2; }
    public double   GetRadius() { return radius; }
    public boolean  GetInverse() { return inverse; }

    // Write and read methods.
    public void WriteAtts(CommunicationBuffer buf)
    {
        if(WriteSelect(0, buf))
            buf.WriteDoubleArray(point1);
        if(WriteSelect(1, buf))
            buf.WriteDoubleArray(point2);
        if(WriteSelect(2, buf))
            buf.WriteDouble(radius);
        if(WriteSelect(3, buf))
            buf.WriteBool(inverse);
    }

    public void ReadAtts(int index, CommunicationBuffer buf)
    {
        switch(index)
        {
        case 0:
            SetPoint1(buf.ReadDoubleArray());
            break;
        case 1:
            SetPoint2(buf.ReadDoubleArray());
            break;
        case 2:
            SetRadius(buf.ReadDouble());
            break;
        case 3:
            SetInverse(buf.ReadBool());
            break;
        }
    }

    public String toString(String indent)
    {
        String str = new String();
        str = str + doubleArrayToString("point1", point1, indent) + "\n";
        str = str + doubleArrayToString("point2", point2, indent) + "\n";
        str = str + doubleToString("radius", radius, indent) + "\n";
        str = str + boolToString("inverse", inverse, indent) + "\n";
        return str;
    }


    // Attributes
    private double[] point1;
    private double[] point2;
    private double   radius;
    private boolean  inverse;
}

