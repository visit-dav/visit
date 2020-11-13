// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

package llnl.visit.operators;

import llnl.visit.AttributeSubject;
import llnl.visit.CommunicationBuffer;
import llnl.visit.Plugin;

// ****************************************************************************
// Class: ClipAttributes
//
// Purpose:
//    This class contains attributes for the clip operator.
//
// Notes:      Autogenerated by xml2java.
//
// Programmer: xml2java
// Creation:   omitted
//
// Modifications:
//   
// ****************************************************************************

public class ClipAttributes extends AttributeSubject implements Plugin
{
    private static int ClipAttributes_numAdditionalAtts = 17;

    // Enum values
    public final static int CLIPSTYLE_PLANE = 0;
    public final static int CLIPSTYLE_SPHERE = 1;

    public final static int WHICHCLIPPLANE_NONE = 0;
    public final static int WHICHCLIPPLANE_PLANE1 = 1;
    public final static int WHICHCLIPPLANE_PLANE2 = 2;
    public final static int WHICHCLIPPLANE_PLANE3 = 3;

    public final static int QUALITY_FAST = 0;
    public final static int QUALITY_ACCURATE = 1;


    public ClipAttributes()
    {
        super(ClipAttributes_numAdditionalAtts);

        quality = QUALITY_FAST;
        funcType = CLIPSTYLE_PLANE;
        plane1Status = true;
        plane2Status = false;
        plane3Status = false;
        plane1Origin = new double[3];
        plane1Origin[0] = 0;
        plane1Origin[1] = 0;
        plane1Origin[2] = 0;
        plane2Origin = new double[3];
        plane2Origin[0] = 0;
        plane2Origin[1] = 0;
        plane2Origin[2] = 0;
        plane3Origin = new double[3];
        plane3Origin[0] = 0;
        plane3Origin[1] = 0;
        plane3Origin[2] = 0;
        plane1Normal = new double[3];
        plane1Normal[0] = 1;
        plane1Normal[1] = 0;
        plane1Normal[2] = 0;
        plane2Normal = new double[3];
        plane2Normal[0] = 0;
        plane2Normal[1] = 1;
        plane2Normal[2] = 0;
        plane3Normal = new double[3];
        plane3Normal[0] = 0;
        plane3Normal[1] = 0;
        plane3Normal[2] = 1;
        planeInverse = false;
        planeToolControlledClipPlane = WHICHCLIPPLANE_PLANE1;
        center = new double[3];
        center[0] = 0;
        center[1] = 0;
        center[2] = 0;
        radius = 1;
        sphereInverse = false;
        crinkleClip = false;
    }

    public ClipAttributes(int nMoreFields)
    {
        super(ClipAttributes_numAdditionalAtts + nMoreFields);

        quality = QUALITY_FAST;
        funcType = CLIPSTYLE_PLANE;
        plane1Status = true;
        plane2Status = false;
        plane3Status = false;
        plane1Origin = new double[3];
        plane1Origin[0] = 0;
        plane1Origin[1] = 0;
        plane1Origin[2] = 0;
        plane2Origin = new double[3];
        plane2Origin[0] = 0;
        plane2Origin[1] = 0;
        plane2Origin[2] = 0;
        plane3Origin = new double[3];
        plane3Origin[0] = 0;
        plane3Origin[1] = 0;
        plane3Origin[2] = 0;
        plane1Normal = new double[3];
        plane1Normal[0] = 1;
        plane1Normal[1] = 0;
        plane1Normal[2] = 0;
        plane2Normal = new double[3];
        plane2Normal[0] = 0;
        plane2Normal[1] = 1;
        plane2Normal[2] = 0;
        plane3Normal = new double[3];
        plane3Normal[0] = 0;
        plane3Normal[1] = 0;
        plane3Normal[2] = 1;
        planeInverse = false;
        planeToolControlledClipPlane = WHICHCLIPPLANE_PLANE1;
        center = new double[3];
        center[0] = 0;
        center[1] = 0;
        center[2] = 0;
        radius = 1;
        sphereInverse = false;
        crinkleClip = false;
    }

    public ClipAttributes(ClipAttributes obj)
    {
        super(obj);

        int i;

        quality = obj.quality;
        funcType = obj.funcType;
        plane1Status = obj.plane1Status;
        plane2Status = obj.plane2Status;
        plane3Status = obj.plane3Status;
        plane1Origin = new double[3];
        plane1Origin[0] = obj.plane1Origin[0];
        plane1Origin[1] = obj.plane1Origin[1];
        plane1Origin[2] = obj.plane1Origin[2];

        plane2Origin = new double[3];
        plane2Origin[0] = obj.plane2Origin[0];
        plane2Origin[1] = obj.plane2Origin[1];
        plane2Origin[2] = obj.plane2Origin[2];

        plane3Origin = new double[3];
        plane3Origin[0] = obj.plane3Origin[0];
        plane3Origin[1] = obj.plane3Origin[1];
        plane3Origin[2] = obj.plane3Origin[2];

        plane1Normal = new double[3];
        plane1Normal[0] = obj.plane1Normal[0];
        plane1Normal[1] = obj.plane1Normal[1];
        plane1Normal[2] = obj.plane1Normal[2];

        plane2Normal = new double[3];
        plane2Normal[0] = obj.plane2Normal[0];
        plane2Normal[1] = obj.plane2Normal[1];
        plane2Normal[2] = obj.plane2Normal[2];

        plane3Normal = new double[3];
        plane3Normal[0] = obj.plane3Normal[0];
        plane3Normal[1] = obj.plane3Normal[1];
        plane3Normal[2] = obj.plane3Normal[2];

        planeInverse = obj.planeInverse;
        planeToolControlledClipPlane = obj.planeToolControlledClipPlane;
        center = new double[3];
        center[0] = obj.center[0];
        center[1] = obj.center[1];
        center[2] = obj.center[2];

        radius = obj.radius;
        sphereInverse = obj.sphereInverse;
        crinkleClip = obj.crinkleClip;

        SelectAll();
    }

    public int Offset()
    {
        return super.Offset() + super.GetNumAdditionalAttributes();
    }

    public int GetNumAdditionalAttributes()
    {
        return ClipAttributes_numAdditionalAtts;
    }

    public boolean equals(ClipAttributes obj)
    {
        int i;

        // Compare the plane1Origin arrays.
        boolean plane1Origin_equal = true;
        for(i = 0; i < 3 && plane1Origin_equal; ++i)
            plane1Origin_equal = (plane1Origin[i] == obj.plane1Origin[i]);

        // Compare the plane2Origin arrays.
        boolean plane2Origin_equal = true;
        for(i = 0; i < 3 && plane2Origin_equal; ++i)
            plane2Origin_equal = (plane2Origin[i] == obj.plane2Origin[i]);

        // Compare the plane3Origin arrays.
        boolean plane3Origin_equal = true;
        for(i = 0; i < 3 && plane3Origin_equal; ++i)
            plane3Origin_equal = (plane3Origin[i] == obj.plane3Origin[i]);

        // Compare the plane1Normal arrays.
        boolean plane1Normal_equal = true;
        for(i = 0; i < 3 && plane1Normal_equal; ++i)
            plane1Normal_equal = (plane1Normal[i] == obj.plane1Normal[i]);

        // Compare the plane2Normal arrays.
        boolean plane2Normal_equal = true;
        for(i = 0; i < 3 && plane2Normal_equal; ++i)
            plane2Normal_equal = (plane2Normal[i] == obj.plane2Normal[i]);

        // Compare the plane3Normal arrays.
        boolean plane3Normal_equal = true;
        for(i = 0; i < 3 && plane3Normal_equal; ++i)
            plane3Normal_equal = (plane3Normal[i] == obj.plane3Normal[i]);

        // Compare the center arrays.
        boolean center_equal = true;
        for(i = 0; i < 3 && center_equal; ++i)
            center_equal = (center[i] == obj.center[i]);

        // Create the return value
        return ((quality == obj.quality) &&
                (funcType == obj.funcType) &&
                (plane1Status == obj.plane1Status) &&
                (plane2Status == obj.plane2Status) &&
                (plane3Status == obj.plane3Status) &&
                plane1Origin_equal &&
                plane2Origin_equal &&
                plane3Origin_equal &&
                plane1Normal_equal &&
                plane2Normal_equal &&
                plane3Normal_equal &&
                (planeInverse == obj.planeInverse) &&
                (planeToolControlledClipPlane == obj.planeToolControlledClipPlane) &&
                center_equal &&
                (radius == obj.radius) &&
                true /* can ignore sphereInverse */ &&
                (crinkleClip == obj.crinkleClip));
    }

    public String GetName() { return "Clip"; }
    public String GetVersion() { return "1.0"; }

    // Property setting methods
    public void SetQuality(int quality_)
    {
        quality = quality_;
        Select(0);
    }

    public void SetFuncType(int funcType_)
    {
        funcType = funcType_;
        Select(1);
    }

    public void SetPlane1Status(boolean plane1Status_)
    {
        plane1Status = plane1Status_;
        Select(2);
    }

    public void SetPlane2Status(boolean plane2Status_)
    {
        plane2Status = plane2Status_;
        Select(3);
    }

    public void SetPlane3Status(boolean plane3Status_)
    {
        plane3Status = plane3Status_;
        Select(4);
    }

    public void SetPlane1Origin(double[] plane1Origin_)
    {
        plane1Origin[0] = plane1Origin_[0];
        plane1Origin[1] = plane1Origin_[1];
        plane1Origin[2] = plane1Origin_[2];
        Select(5);
    }

    public void SetPlane1Origin(double e0, double e1, double e2)
    {
        plane1Origin[0] = e0;
        plane1Origin[1] = e1;
        plane1Origin[2] = e2;
        Select(5);
    }

    public void SetPlane2Origin(double[] plane2Origin_)
    {
        plane2Origin[0] = plane2Origin_[0];
        plane2Origin[1] = plane2Origin_[1];
        plane2Origin[2] = plane2Origin_[2];
        Select(6);
    }

    public void SetPlane2Origin(double e0, double e1, double e2)
    {
        plane2Origin[0] = e0;
        plane2Origin[1] = e1;
        plane2Origin[2] = e2;
        Select(6);
    }

    public void SetPlane3Origin(double[] plane3Origin_)
    {
        plane3Origin[0] = plane3Origin_[0];
        plane3Origin[1] = plane3Origin_[1];
        plane3Origin[2] = plane3Origin_[2];
        Select(7);
    }

    public void SetPlane3Origin(double e0, double e1, double e2)
    {
        plane3Origin[0] = e0;
        plane3Origin[1] = e1;
        plane3Origin[2] = e2;
        Select(7);
    }

    public void SetPlane1Normal(double[] plane1Normal_)
    {
        plane1Normal[0] = plane1Normal_[0];
        plane1Normal[1] = plane1Normal_[1];
        plane1Normal[2] = plane1Normal_[2];
        Select(8);
    }

    public void SetPlane1Normal(double e0, double e1, double e2)
    {
        plane1Normal[0] = e0;
        plane1Normal[1] = e1;
        plane1Normal[2] = e2;
        Select(8);
    }

    public void SetPlane2Normal(double[] plane2Normal_)
    {
        plane2Normal[0] = plane2Normal_[0];
        plane2Normal[1] = plane2Normal_[1];
        plane2Normal[2] = plane2Normal_[2];
        Select(9);
    }

    public void SetPlane2Normal(double e0, double e1, double e2)
    {
        plane2Normal[0] = e0;
        plane2Normal[1] = e1;
        plane2Normal[2] = e2;
        Select(9);
    }

    public void SetPlane3Normal(double[] plane3Normal_)
    {
        plane3Normal[0] = plane3Normal_[0];
        plane3Normal[1] = plane3Normal_[1];
        plane3Normal[2] = plane3Normal_[2];
        Select(10);
    }

    public void SetPlane3Normal(double e0, double e1, double e2)
    {
        plane3Normal[0] = e0;
        plane3Normal[1] = e1;
        plane3Normal[2] = e2;
        Select(10);
    }

    public void SetPlaneInverse(boolean planeInverse_)
    {
        planeInverse = planeInverse_;
        Select(11);
    }

    public void SetPlaneToolControlledClipPlane(int planeToolControlledClipPlane_)
    {
        planeToolControlledClipPlane = planeToolControlledClipPlane_;
        Select(12);
    }

    public void SetCenter(double[] center_)
    {
        center[0] = center_[0];
        center[1] = center_[1];
        center[2] = center_[2];
        Select(13);
    }

    public void SetCenter(double e0, double e1, double e2)
    {
        center[0] = e0;
        center[1] = e1;
        center[2] = e2;
        Select(13);
    }

    public void SetRadius(double radius_)
    {
        radius = radius_;
        Select(14);
    }

    public void SetSphereInverse(boolean sphereInverse_)
    {
        sphereInverse = sphereInverse_;
        Select(15);
    }

    public void SetCrinkleClip(boolean crinkleClip_)
    {
        crinkleClip = crinkleClip_;
        Select(16);
    }

    // Property getting methods
    public int      GetQuality() { return quality; }
    public int      GetFuncType() { return funcType; }
    public boolean  GetPlane1Status() { return plane1Status; }
    public boolean  GetPlane2Status() { return plane2Status; }
    public boolean  GetPlane3Status() { return plane3Status; }
    public double[] GetPlane1Origin() { return plane1Origin; }
    public double[] GetPlane2Origin() { return plane2Origin; }
    public double[] GetPlane3Origin() { return plane3Origin; }
    public double[] GetPlane1Normal() { return plane1Normal; }
    public double[] GetPlane2Normal() { return plane2Normal; }
    public double[] GetPlane3Normal() { return plane3Normal; }
    public boolean  GetPlaneInverse() { return planeInverse; }
    public int      GetPlaneToolControlledClipPlane() { return planeToolControlledClipPlane; }
    public double[] GetCenter() { return center; }
    public double   GetRadius() { return radius; }
    public boolean  GetSphereInverse() { return sphereInverse; }
    public boolean  GetCrinkleClip() { return crinkleClip; }

    // Write and read methods.
    public void WriteAtts(CommunicationBuffer buf)
    {
        if(WriteSelect(0, buf))
            buf.WriteInt(quality);
        if(WriteSelect(1, buf))
            buf.WriteInt(funcType);
        if(WriteSelect(2, buf))
            buf.WriteBool(plane1Status);
        if(WriteSelect(3, buf))
            buf.WriteBool(plane2Status);
        if(WriteSelect(4, buf))
            buf.WriteBool(plane3Status);
        if(WriteSelect(5, buf))
            buf.WriteDoubleArray(plane1Origin);
        if(WriteSelect(6, buf))
            buf.WriteDoubleArray(plane2Origin);
        if(WriteSelect(7, buf))
            buf.WriteDoubleArray(plane3Origin);
        if(WriteSelect(8, buf))
            buf.WriteDoubleArray(plane1Normal);
        if(WriteSelect(9, buf))
            buf.WriteDoubleArray(plane2Normal);
        if(WriteSelect(10, buf))
            buf.WriteDoubleArray(plane3Normal);
        if(WriteSelect(11, buf))
            buf.WriteBool(planeInverse);
        if(WriteSelect(12, buf))
            buf.WriteInt(planeToolControlledClipPlane);
        if(WriteSelect(13, buf))
            buf.WriteDoubleArray(center);
        if(WriteSelect(14, buf))
            buf.WriteDouble(radius);
        if(WriteSelect(15, buf))
            buf.WriteBool(sphereInverse);
        if(WriteSelect(16, buf))
            buf.WriteBool(crinkleClip);
    }

    public void ReadAtts(int index, CommunicationBuffer buf)
    {
        switch(index)
        {
        case 0:
            SetQuality(buf.ReadInt());
            break;
        case 1:
            SetFuncType(buf.ReadInt());
            break;
        case 2:
            SetPlane1Status(buf.ReadBool());
            break;
        case 3:
            SetPlane2Status(buf.ReadBool());
            break;
        case 4:
            SetPlane3Status(buf.ReadBool());
            break;
        case 5:
            SetPlane1Origin(buf.ReadDoubleArray());
            break;
        case 6:
            SetPlane2Origin(buf.ReadDoubleArray());
            break;
        case 7:
            SetPlane3Origin(buf.ReadDoubleArray());
            break;
        case 8:
            SetPlane1Normal(buf.ReadDoubleArray());
            break;
        case 9:
            SetPlane2Normal(buf.ReadDoubleArray());
            break;
        case 10:
            SetPlane3Normal(buf.ReadDoubleArray());
            break;
        case 11:
            SetPlaneInverse(buf.ReadBool());
            break;
        case 12:
            SetPlaneToolControlledClipPlane(buf.ReadInt());
            break;
        case 13:
            SetCenter(buf.ReadDoubleArray());
            break;
        case 14:
            SetRadius(buf.ReadDouble());
            break;
        case 15:
            SetSphereInverse(buf.ReadBool());
            break;
        case 16:
            SetCrinkleClip(buf.ReadBool());
            break;
        }
    }

    public String toString(String indent)
    {
        String str = new String();
        str = str + indent + "quality = ";
        if(quality == QUALITY_FAST)
            str = str + "QUALITY_FAST";
        if(quality == QUALITY_ACCURATE)
            str = str + "QUALITY_ACCURATE";
        str = str + "\n";
        str = str + indent + "funcType = ";
        if(funcType == CLIPSTYLE_PLANE)
            str = str + "CLIPSTYLE_PLANE";
        if(funcType == CLIPSTYLE_SPHERE)
            str = str + "CLIPSTYLE_SPHERE";
        str = str + "\n";
        str = str + boolToString("plane1Status", plane1Status, indent) + "\n";
        str = str + boolToString("plane2Status", plane2Status, indent) + "\n";
        str = str + boolToString("plane3Status", plane3Status, indent) + "\n";
        str = str + doubleArrayToString("plane1Origin", plane1Origin, indent) + "\n";
        str = str + doubleArrayToString("plane2Origin", plane2Origin, indent) + "\n";
        str = str + doubleArrayToString("plane3Origin", plane3Origin, indent) + "\n";
        str = str + doubleArrayToString("plane1Normal", plane1Normal, indent) + "\n";
        str = str + doubleArrayToString("plane2Normal", plane2Normal, indent) + "\n";
        str = str + doubleArrayToString("plane3Normal", plane3Normal, indent) + "\n";
        str = str + boolToString("planeInverse", planeInverse, indent) + "\n";
        str = str + indent + "planeToolControlledClipPlane = ";
        if(planeToolControlledClipPlane == WHICHCLIPPLANE_NONE)
            str = str + "WHICHCLIPPLANE_NONE";
        if(planeToolControlledClipPlane == WHICHCLIPPLANE_PLANE1)
            str = str + "WHICHCLIPPLANE_PLANE1";
        if(planeToolControlledClipPlane == WHICHCLIPPLANE_PLANE2)
            str = str + "WHICHCLIPPLANE_PLANE2";
        if(planeToolControlledClipPlane == WHICHCLIPPLANE_PLANE3)
            str = str + "WHICHCLIPPLANE_PLANE3";
        str = str + "\n";
        str = str + doubleArrayToString("center", center, indent) + "\n";
        str = str + doubleToString("radius", radius, indent) + "\n";
        str = str + boolToString("sphereInverse", sphereInverse, indent) + "\n";
        str = str + boolToString("crinkleClip", crinkleClip, indent) + "\n";
        return str;
    }


    // Attributes
    private int      quality;
    private int      funcType;
    private boolean  plane1Status;
    private boolean  plane2Status;
    private boolean  plane3Status;
    private double[] plane1Origin;
    private double[] plane2Origin;
    private double[] plane3Origin;
    private double[] plane1Normal;
    private double[] plane2Normal;
    private double[] plane3Normal;
    private boolean  planeInverse;
    private int      planeToolControlledClipPlane;
    private double[] center;
    private double   radius;
    private boolean  sphereInverse;
    private boolean  crinkleClip;
}

