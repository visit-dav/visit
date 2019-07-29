// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

package llnl.visit;


// ****************************************************************************
// Class: PlaneAttributes
//
// Purpose:
//    Attributes for a plane
//
// Notes:      Autogenerated by xml2java.
//
// Programmer: xml2java
// Creation:   omitted
//
// Modifications:
//   
// ****************************************************************************

public class PlaneAttributes extends AttributeSubject
{
    private static int PlaneAttributes_numAdditionalAtts = 6;

    public PlaneAttributes()
    {
        super(PlaneAttributes_numAdditionalAtts);

        origin = new double[3];
        origin[0] = 0;
        origin[1] = 0;
        origin[2] = 0;
        normal = new double[3];
        normal[0] = 0;
        normal[1] = 0;
        normal[2] = 1;
        upAxis = new double[3];
        upAxis[0] = 0;
        upAxis[1] = 1;
        upAxis[2] = 0;
        haveRadius = false;
        radius = 1;
        threeSpace = false;
    }

    public PlaneAttributes(int nMoreFields)
    {
        super(PlaneAttributes_numAdditionalAtts + nMoreFields);

        origin = new double[3];
        origin[0] = 0;
        origin[1] = 0;
        origin[2] = 0;
        normal = new double[3];
        normal[0] = 0;
        normal[1] = 0;
        normal[2] = 1;
        upAxis = new double[3];
        upAxis[0] = 0;
        upAxis[1] = 1;
        upAxis[2] = 0;
        haveRadius = false;
        radius = 1;
        threeSpace = false;
    }

    public PlaneAttributes(PlaneAttributes obj)
    {
        super(obj);

        int i;

        origin = new double[3];
        origin[0] = obj.origin[0];
        origin[1] = obj.origin[1];
        origin[2] = obj.origin[2];

        normal = new double[3];
        normal[0] = obj.normal[0];
        normal[1] = obj.normal[1];
        normal[2] = obj.normal[2];

        upAxis = new double[3];
        upAxis[0] = obj.upAxis[0];
        upAxis[1] = obj.upAxis[1];
        upAxis[2] = obj.upAxis[2];

        haveRadius = obj.haveRadius;
        radius = obj.radius;
        threeSpace = obj.threeSpace;

        SelectAll();
    }

    public int Offset()
    {
        return super.Offset() + super.GetNumAdditionalAttributes();
    }

    public int GetNumAdditionalAttributes()
    {
        return PlaneAttributes_numAdditionalAtts;
    }

    public boolean equals(PlaneAttributes obj)
    {
        int i;

        // Compare the origin arrays.
        boolean origin_equal = true;
        for(i = 0; i < 3 && origin_equal; ++i)
            origin_equal = (origin[i] == obj.origin[i]);

        // Compare the normal arrays.
        boolean normal_equal = true;
        for(i = 0; i < 3 && normal_equal; ++i)
            normal_equal = (normal[i] == obj.normal[i]);

        // Compare the upAxis arrays.
        boolean upAxis_equal = true;
        for(i = 0; i < 3 && upAxis_equal; ++i)
            upAxis_equal = (upAxis[i] == obj.upAxis[i]);

        // Create the return value
        return (origin_equal &&
                normal_equal &&
                upAxis_equal &&
                (haveRadius == obj.haveRadius) &&
                (radius == obj.radius) &&
                (threeSpace == obj.threeSpace));
    }

    // Property setting methods
    public void SetOrigin(double[] origin_)
    {
        origin[0] = origin_[0];
        origin[1] = origin_[1];
        origin[2] = origin_[2];
        Select(0);
    }

    public void SetOrigin(double e0, double e1, double e2)
    {
        origin[0] = e0;
        origin[1] = e1;
        origin[2] = e2;
        Select(0);
    }

    public void SetNormal(double[] normal_)
    {
        normal[0] = normal_[0];
        normal[1] = normal_[1];
        normal[2] = normal_[2];
        Select(1);
    }

    public void SetNormal(double e0, double e1, double e2)
    {
        normal[0] = e0;
        normal[1] = e1;
        normal[2] = e2;
        Select(1);
    }

    public void SetUpAxis(double[] upAxis_)
    {
        upAxis[0] = upAxis_[0];
        upAxis[1] = upAxis_[1];
        upAxis[2] = upAxis_[2];
        Select(2);
    }

    public void SetUpAxis(double e0, double e1, double e2)
    {
        upAxis[0] = e0;
        upAxis[1] = e1;
        upAxis[2] = e2;
        Select(2);
    }

    public void SetHaveRadius(boolean haveRadius_)
    {
        haveRadius = haveRadius_;
        Select(3);
    }

    public void SetRadius(double radius_)
    {
        radius = radius_;
        Select(4);
    }

    public void SetThreeSpace(boolean threeSpace_)
    {
        threeSpace = threeSpace_;
        Select(5);
    }

    // Property getting methods
    public double[] GetOrigin() { return origin; }
    public double[] GetNormal() { return normal; }
    public double[] GetUpAxis() { return upAxis; }
    public boolean  GetHaveRadius() { return haveRadius; }
    public double   GetRadius() { return radius; }
    public boolean  GetThreeSpace() { return threeSpace; }

    // Write and read methods.
    public void WriteAtts(CommunicationBuffer buf)
    {
        if(WriteSelect(0, buf))
            buf.WriteDoubleArray(origin);
        if(WriteSelect(1, buf))
            buf.WriteDoubleArray(normal);
        if(WriteSelect(2, buf))
            buf.WriteDoubleArray(upAxis);
        if(WriteSelect(3, buf))
            buf.WriteBool(haveRadius);
        if(WriteSelect(4, buf))
            buf.WriteDouble(radius);
        if(WriteSelect(5, buf))
            buf.WriteBool(threeSpace);
    }

    public void ReadAtts(int index, CommunicationBuffer buf)
    {
        switch(index)
        {
        case 0:
            SetOrigin(buf.ReadDoubleArray());
            break;
        case 1:
            SetNormal(buf.ReadDoubleArray());
            break;
        case 2:
            SetUpAxis(buf.ReadDoubleArray());
            break;
        case 3:
            SetHaveRadius(buf.ReadBool());
            break;
        case 4:
            SetRadius(buf.ReadDouble());
            break;
        case 5:
            SetThreeSpace(buf.ReadBool());
            break;
        }
    }

    public String toString(String indent)
    {
        String str = new String();
        str = str + doubleArrayToString("origin", origin, indent) + "\n";
        str = str + doubleArrayToString("normal", normal, indent) + "\n";
        str = str + doubleArrayToString("upAxis", upAxis, indent) + "\n";
        str = str + boolToString("haveRadius", haveRadius, indent) + "\n";
        str = str + doubleToString("radius", radius, indent) + "\n";
        str = str + boolToString("threeSpace", threeSpace, indent) + "\n";
        return str;
    }


    // Attributes
    private double[] origin;
    private double[] normal;
    private double[] upAxis;
    private boolean  haveRadius;
    private double   radius;
    private boolean  threeSpace;
}

