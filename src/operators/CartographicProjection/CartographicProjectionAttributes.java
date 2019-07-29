// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

package llnl.visit.operators;

import llnl.visit.AttributeSubject;
import llnl.visit.CommunicationBuffer;
import llnl.visit.Plugin;

// ****************************************************************************
// Class: CartographicProjectionAttributes
//
// Purpose:
//    
//
// Notes:      Autogenerated by xml2java.
//
// Programmer: xml2java
// Creation:   omitted
//
// Modifications:
//   
// ****************************************************************************

public class CartographicProjectionAttributes extends AttributeSubject implements Plugin
{
    private static int CartographicProjectionAttributes_numAdditionalAtts = 2;

    // Enum values
    public final static int PROJECTIONID_AITOFF = 0;
    public final static int PROJECTIONID_ECK4 = 1;
    public final static int PROJECTIONID_EQDC = 2;
    public final static int PROJECTIONID_HAMMER = 3;
    public final static int PROJECTIONID_LAEA = 4;
    public final static int PROJECTIONID_LCC = 5;
    public final static int PROJECTIONID_MERC = 6;
    public final static int PROJECTIONID_MILL = 7;
    public final static int PROJECTIONID_MOLL = 8;
    public final static int PROJECTIONID_ORTHO = 9;
    public final static int PROJECTIONID_WINK2 = 10;


    public CartographicProjectionAttributes()
    {
        super(CartographicProjectionAttributes_numAdditionalAtts);

        projectionID = PROJECTIONID_AITOFF;
        centralMeridian = 0;
    }

    public CartographicProjectionAttributes(int nMoreFields)
    {
        super(CartographicProjectionAttributes_numAdditionalAtts + nMoreFields);

        projectionID = PROJECTIONID_AITOFF;
        centralMeridian = 0;
    }

    public CartographicProjectionAttributes(CartographicProjectionAttributes obj)
    {
        super(obj);

        projectionID = obj.projectionID;
        centralMeridian = obj.centralMeridian;

        SelectAll();
    }

    public int Offset()
    {
        return super.Offset() + super.GetNumAdditionalAttributes();
    }

    public int GetNumAdditionalAttributes()
    {
        return CartographicProjectionAttributes_numAdditionalAtts;
    }

    public boolean equals(CartographicProjectionAttributes obj)
    {
        // Create the return value
        return ((projectionID == obj.projectionID) &&
                (centralMeridian == obj.centralMeridian));
    }

    public String GetName() { return "CartographicProjection"; }
    public String GetVersion() { return "1.0"; }

    // Property setting methods
    public void SetProjectionID(int projectionID_)
    {
        projectionID = projectionID_;
        Select(0);
    }

    public void SetCentralMeridian(double centralMeridian_)
    {
        centralMeridian = centralMeridian_;
        Select(1);
    }

    // Property getting methods
    public int    GetProjectionID() { return projectionID; }
    public double GetCentralMeridian() { return centralMeridian; }

    // Write and read methods.
    public void WriteAtts(CommunicationBuffer buf)
    {
        if(WriteSelect(0, buf))
            buf.WriteInt(projectionID);
        if(WriteSelect(1, buf))
            buf.WriteDouble(centralMeridian);
    }

    public void ReadAtts(int index, CommunicationBuffer buf)
    {
        switch(index)
        {
        case 0:
            SetProjectionID(buf.ReadInt());
            break;
        case 1:
            SetCentralMeridian(buf.ReadDouble());
            break;
        }
    }

    public String toString(String indent)
    {
        String str = new String();
        str = str + indent + "projectionID = ";
        if(projectionID == PROJECTIONID_AITOFF)
            str = str + "PROJECTIONID_AITOFF";
        if(projectionID == PROJECTIONID_ECK4)
            str = str + "PROJECTIONID_ECK4";
        if(projectionID == PROJECTIONID_EQDC)
            str = str + "PROJECTIONID_EQDC";
        if(projectionID == PROJECTIONID_HAMMER)
            str = str + "PROJECTIONID_HAMMER";
        if(projectionID == PROJECTIONID_LAEA)
            str = str + "PROJECTIONID_LAEA";
        if(projectionID == PROJECTIONID_LCC)
            str = str + "PROJECTIONID_LCC";
        if(projectionID == PROJECTIONID_MERC)
            str = str + "PROJECTIONID_MERC";
        if(projectionID == PROJECTIONID_MILL)
            str = str + "PROJECTIONID_MILL";
        if(projectionID == PROJECTIONID_MOLL)
            str = str + "PROJECTIONID_MOLL";
        if(projectionID == PROJECTIONID_ORTHO)
            str = str + "PROJECTIONID_ORTHO";
        if(projectionID == PROJECTIONID_WINK2)
            str = str + "PROJECTIONID_WINK2";
        str = str + "\n";
        str = str + doubleToString("centralMeridian", centralMeridian, indent) + "\n";
        return str;
    }


    // Attributes
    private int    projectionID;
    private double centralMeridian;
}

