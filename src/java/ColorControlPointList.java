// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

package llnl.visit;

import java.util.Vector;

// ****************************************************************************
// Class: ColorControlPointList
//
// Purpose:
//    This class contains a list of ColorControlPoint objects.
//
// Notes:      Autogenerated by xml2java.
//
// Programmer: xml2java
// Creation:   omitted
//
// Modifications:
//
// ****************************************************************************

public class ColorControlPointList extends AttributeSubject
{
    private static int ColorControlPointList_numAdditionalAtts = 7;

    // Enum values
    public final static int SMOOTHINGMETHOD_NONE = 0;
    public final static int SMOOTHINGMETHOD_LINEAR = 1;
    public final static int SMOOTHINGMETHOD_CUBICSPLINE = 2;


    public ColorControlPointList()
    {
        super(ColorControlPointList_numAdditionalAtts);

        controlPoints = new Vector();
        smoothing = SMOOTHINGMETHOD_LINEAR;
        equalSpacingFlag = false;
        discreteFlag = false;
        externalFlag = false;
        categoryName = new String("");
        tags = new Vector();
    }

    public ColorControlPointList(int nMoreFields)
    {
        super(ColorControlPointList_numAdditionalAtts + nMoreFields);

        controlPoints = new Vector();
        smoothing = SMOOTHINGMETHOD_LINEAR;
        equalSpacingFlag = false;
        discreteFlag = false;
        externalFlag = false;
        categoryName = new String("");
        tags = new Vector();
    }

    public ColorControlPointList(ColorControlPointList obj)
    {
        super(obj);

        int i;

        // *** Copy the controlPoints field ***
        controlPoints = new Vector(obj.controlPoints.size());
        for(i = 0; i < obj.controlPoints.size(); ++i)
        {
            ColorControlPoint oldObj = (ColorControlPoint)obj.controlPoints.elementAt(i);
            controlPoints.addElement(new ColorControlPoint(oldObj));
        }

        smoothing = obj.smoothing;
        equalSpacingFlag = obj.equalSpacingFlag;
        discreteFlag = obj.discreteFlag;
        externalFlag = obj.externalFlag;
        categoryName = new String(obj.categoryName);
        tags = new Vector(obj.tags.size());
        for(i = 0; i < obj.tags.size(); ++i)
            tags.addElement(new String((String)obj.tags.elementAt(i)));


        SelectAll();
    }

    public int Offset()
    {
        return super.Offset() + super.GetNumAdditionalAttributes();
    }

    public int GetNumAdditionalAttributes()
    {
        return ColorControlPointList_numAdditionalAtts;
    }

    public boolean equals(ColorControlPointList obj)
    {
        int i;

        // Compare the elements in the controlPoints vector.
        boolean controlPoints_equal = (obj.controlPoints.size() == controlPoints.size());
        for(i = 0; (i < controlPoints.size()) && controlPoints_equal; ++i)
        {
            // Make references to ColorControlPoint from Object.
            ColorControlPoint controlPoints1 = (ColorControlPoint)controlPoints.elementAt(i);
            ColorControlPoint controlPoints2 = (ColorControlPoint)obj.controlPoints.elementAt(i);
            controlPoints_equal = controlPoints1.equals(controlPoints2);
        }
        // Compare the elements in the tags vector.
        boolean tags_equal = (obj.tags.size() == tags.size());
        for(i = 0; (i < tags.size()) && tags_equal; ++i)
        {
            // Make references to String from Object.
            String tags1 = (String)tags.elementAt(i);
            String tags2 = (String)obj.tags.elementAt(i);
            tags_equal = tags1.equals(tags2);
        }
        // Create the return value
        return (controlPoints_equal &&
                (smoothing == obj.smoothing) &&
                (equalSpacingFlag == obj.equalSpacingFlag) &&
                (discreteFlag == obj.discreteFlag) &&
                (externalFlag == obj.externalFlag) &&
                (categoryName.equals(obj.categoryName)) &&
                tags_equal);
    }

    // Property setting methods
    public void SetSmoothing(int smoothing_)
    {
        smoothing = smoothing_;
        Select(1);
    }

    public void SetEqualSpacingFlag(boolean equalSpacingFlag_)
    {
        equalSpacingFlag = equalSpacingFlag_;
        Select(2);
    }

    public void SetDiscreteFlag(boolean discreteFlag_)
    {
        discreteFlag = discreteFlag_;
        Select(3);
    }

    public void SetExternalFlag(boolean externalFlag_)
    {
        externalFlag = externalFlag_;
        Select(4);
    }

    public void SetCategoryName(String categoryName_)
    {
        categoryName = categoryName_;
        Select(5);
    }

    public void SetTags(Vector tags_)
    {
        tags = tags_;
        Select(6);
    }

    // Property getting methods
    public Vector  GetControlPoints() { return controlPoints; }
    public int     GetSmoothing() { return smoothing; }
    public boolean GetEqualSpacingFlag() { return equalSpacingFlag; }
    public boolean GetDiscreteFlag() { return discreteFlag; }
    public boolean GetExternalFlag() { return externalFlag; }
    public String  GetCategoryName() { return categoryName; }
    public Vector  GetTags() { return tags; }

    // Write and read methods.
    public void WriteAtts(CommunicationBuffer buf)
    {
        if(WriteSelect(0, buf))
        {
            buf.WriteInt(controlPoints.size());
            for(int i = 0; i < controlPoints.size(); ++i)
            {
                ColorControlPoint tmp = (ColorControlPoint)controlPoints.elementAt(i);
                tmp.Write(buf);
            }
        }
        if(WriteSelect(1, buf))
            buf.WriteInt(smoothing);
        if(WriteSelect(2, buf))
            buf.WriteBool(equalSpacingFlag);
        if(WriteSelect(3, buf))
            buf.WriteBool(discreteFlag);
        if(WriteSelect(4, buf))
            buf.WriteBool(externalFlag);
        if(WriteSelect(5, buf))
            buf.WriteString(categoryName);
        if(WriteSelect(6, buf))
            buf.WriteStringVector(tags);
    }

    public void ReadAtts(int index, CommunicationBuffer buf)
    {
        switch(index)
        {
        case 0:
            {
                int len = buf.ReadInt();
                controlPoints.clear();
                for(int j = 0; j < len; ++j)
                {
                    ColorControlPoint tmp = new ColorControlPoint();
                    tmp.Read(buf);
                    controlPoints.addElement(tmp);
                }
            }
            Select(0);
            break;
        case 1:
            SetSmoothing(buf.ReadInt());
            break;
        case 2:
            SetEqualSpacingFlag(buf.ReadBool());
            break;
        case 3:
            SetDiscreteFlag(buf.ReadBool());
            break;
        case 4:
            SetExternalFlag(buf.ReadBool());
            break;
        case 5:
            SetCategoryName(buf.ReadString());
            break;
        case 6:
            SetTags(buf.ReadStringVector());
            break;
        }
    }

    public String toString(String indent)
    {
        String str = new String();
        str = str + indent + "controlPoints = {\n";
        for(int i = 0; i < controlPoints.size(); ++i)
        {
            AttributeSubject s = (AttributeSubject)controlPoints.elementAt(i);
            str = str + s.toString(indent + "    ");
            if(i < controlPoints.size()-1)
                str = str + ", ";
            str = str + "\n";
        }
        str = str + "}\n";
        str = str + indent + "smoothing = ";
        if(smoothing == SMOOTHINGMETHOD_NONE)
            str = str + "SMOOTHINGMETHOD_NONE";
        if(smoothing == SMOOTHINGMETHOD_LINEAR)
            str = str + "SMOOTHINGMETHOD_LINEAR";
        if(smoothing == SMOOTHINGMETHOD_CUBICSPLINE)
            str = str + "SMOOTHINGMETHOD_CUBICSPLINE";
        str = str + "\n";
        str = str + boolToString("equalSpacingFlag", equalSpacingFlag, indent) + "\n";
        str = str + boolToString("discreteFlag", discreteFlag, indent) + "\n";
        str = str + boolToString("externalFlag", externalFlag, indent) + "\n";
        str = str + stringToString("categoryName", categoryName, indent) + "\n";
        str = str + stringVectorToString("tags", tags, indent) + "\n";
        return str;
    }

    // Attributegroup convenience methods
    public void AddControlPoints(ColorControlPoint obj)
    {
        controlPoints.addElement(new ColorControlPoint(obj));
        Select(0);
    }

    public void ClearControlPoints()
    {
        controlPoints.clear();
        Select(0);
    }

    public void RemoveControlPoints(int index)
    {
        if(index >= 0 && index < controlPoints.size())
        {
            controlPoints.remove(index);
            Select(0);
        }
    }

    public int GetNumControlPoints()
    {
        return controlPoints.size();
    }

    public ColorControlPoint GetControlPoints(int i)
    {
        ColorControlPoint tmp = (ColorControlPoint)controlPoints.elementAt(i);
        return tmp;
    }


    // Attributes
    private Vector  controlPoints; // vector of ColorControlPoint objects
    private int     smoothing;
    private boolean equalSpacingFlag;
    private boolean discreteFlag;
    private boolean externalFlag;
    private String  categoryName;
    private Vector  tags; // vector of String objects
}

