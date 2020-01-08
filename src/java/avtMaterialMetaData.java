// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

package llnl.visit;

import java.util.Vector;

// ****************************************************************************
// Class: avtMaterialMetaData
//
// Purpose:
//    Contains material metadata attributes
//
// Notes:      Autogenerated by xml2java.
//
// Programmer: xml2java
// Creation:   omitted
//
// Modifications:
//
// ****************************************************************************

public class avtMaterialMetaData extends avtBaseVarMetaData
{
    private static int avtMaterialMetaData_numAdditionalAtts = 3;

    public avtMaterialMetaData()
    {
        super(avtMaterialMetaData_numAdditionalAtts);

        numMaterials = 0;
        materialNames = new Vector();
        colorNames = new Vector();
    }

    public avtMaterialMetaData(int nMoreFields)
    {
        super(avtMaterialMetaData_numAdditionalAtts + nMoreFields);

        numMaterials = 0;
        materialNames = new Vector();
        colorNames = new Vector();
    }

    public avtMaterialMetaData(avtMaterialMetaData obj)
    {
        super(obj);

        int i;

        numMaterials = obj.numMaterials;
        materialNames = new Vector(obj.materialNames.size());
        for(i = 0; i < obj.materialNames.size(); ++i)
            materialNames.addElement(new String((String)obj.materialNames.elementAt(i)));

        colorNames = new Vector(obj.colorNames.size());
        for(i = 0; i < obj.colorNames.size(); ++i)
            colorNames.addElement(new String((String)obj.colorNames.elementAt(i)));


        SelectAll();
    }

    public int Offset()
    {
        return super.Offset() + super.GetNumAdditionalAttributes();
    }

    public int GetNumAdditionalAttributes()
    {
        return avtMaterialMetaData_numAdditionalAtts;
    }

    public boolean equals(avtMaterialMetaData obj)
    {
        int i;

        // Compare the elements in the materialNames vector.
        boolean materialNames_equal = (obj.materialNames.size() == materialNames.size());
        for(i = 0; (i < materialNames.size()) && materialNames_equal; ++i)
        {
            // Make references to String from Object.
            String materialNames1 = (String)materialNames.elementAt(i);
            String materialNames2 = (String)obj.materialNames.elementAt(i);
            materialNames_equal = materialNames1.equals(materialNames2);
        }
        // Compare the elements in the colorNames vector.
        boolean colorNames_equal = (obj.colorNames.size() == colorNames.size());
        for(i = 0; (i < colorNames.size()) && colorNames_equal; ++i)
        {
            // Make references to String from Object.
            String colorNames1 = (String)colorNames.elementAt(i);
            String colorNames2 = (String)obj.colorNames.elementAt(i);
            colorNames_equal = colorNames1.equals(colorNames2);
        }
        // Create the return value
        return (super.equals(obj) && (numMaterials == obj.numMaterials) &&
                materialNames_equal &&
                colorNames_equal);
    }

    // Property setting methods
    public void SetNumMaterials(int numMaterials_)
    {
        numMaterials = numMaterials_;
        Select((new avtMaterialMetaData()).Offset() + 0);
    }

    public void SetMaterialNames(Vector materialNames_)
    {
        materialNames = materialNames_;
        Select((new avtMaterialMetaData()).Offset() + 1);
    }

    public void SetColorNames(Vector colorNames_)
    {
        colorNames = colorNames_;
        Select((new avtMaterialMetaData()).Offset() + 2);
    }

    // Property getting methods
    public int    GetNumMaterials() { return numMaterials; }
    public Vector GetMaterialNames() { return materialNames; }
    public Vector GetColorNames() { return colorNames; }

    // Write and read methods.
    public void WriteAtts(CommunicationBuffer buf)
    {
        super.WriteAtts(buf);

        int offset = (new avtMaterialMetaData()).Offset();
        if(WriteSelect(offset + 0, buf))
            buf.WriteInt(numMaterials);
        if(WriteSelect(offset + 1, buf))
            buf.WriteStringVector(materialNames);
        if(WriteSelect(offset + 2, buf))
            buf.WriteStringVector(colorNames);
    }

    public void ReadAtts(int id, CommunicationBuffer buf)
    {
        int offset = (new avtMaterialMetaData()).Offset();
        int index = id - offset;
        switch(index)
        {
        case 0:
            SetNumMaterials(buf.ReadInt());
            break;
        case 1:
            SetMaterialNames(buf.ReadStringVector());
            break;
        case 2:
            SetColorNames(buf.ReadStringVector());
            break;
        default:
            super.ReadAtts(id, buf);
            break;
        }
    }

    public String toString(String indent)
    {
        String str = new String();
        str = str + intToString("numMaterials", numMaterials, indent) + "\n";
        str = str + stringVectorToString("materialNames", materialNames, indent) + "\n";
        str = str + stringVectorToString("colorNames", colorNames, indent) + "\n";
        return super.toString(indent) + str;
    }


    // Attributes
    private int    numMaterials;
    private Vector materialNames; // vector of String objects
    private Vector colorNames; // vector of String objects
}

