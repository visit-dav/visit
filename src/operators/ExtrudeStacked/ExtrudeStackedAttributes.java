// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

package llnl.visit.operators;

import llnl.visit.AttributeSubject;
import llnl.visit.CommunicationBuffer;
import llnl.visit.Plugin;
import java.util.Vector;
import java.lang.Double;

// ****************************************************************************
// Class: ExtrudeStackedAttributes
//
// Purpose:
//    This class contains attributes for the extrude operator.
//
// Notes:      Autogenerated by xml2java.
//
// Programmer: xml2java
// Creation:   omitted
//
// Modifications:
//
// ****************************************************************************

public class ExtrudeStackedAttributes extends AttributeSubject implements Plugin
{
    private static int ExtrudeStackedAttributes_numAdditionalAtts = 12;

    // Enum values
    public final static int VARIABLEDISPLAYTYPE_NODEHEIGHT = 0;
    public final static int VARIABLEDISPLAYTYPE_CELLHEIGHT = 1;
    public final static int VARIABLEDISPLAYTYPE_VARINDEX = 2;


    public ExtrudeStackedAttributes()
    {
        super(ExtrudeStackedAttributes_numAdditionalAtts);

        axis = new double[3];
        axis[0] = 0;
        axis[1] = 0;
        axis[2] = 1;
        byVariable = true;
        defaultVariable = new String("");
        scalarVariableNames = new Vector();
        visualVariableNames = new Vector();
        extentMinima = new Vector();
        extentMaxima = new Vector();
        extentScale = new Vector();
        variableDisplay = VARIABLEDISPLAYTYPE_VARINDEX;
        length = 1;
        steps = 1;
        preserveOriginalCellNumbers = true;
    }

    public ExtrudeStackedAttributes(int nMoreFields)
    {
        super(ExtrudeStackedAttributes_numAdditionalAtts + nMoreFields);

        axis = new double[3];
        axis[0] = 0;
        axis[1] = 0;
        axis[2] = 1;
        byVariable = true;
        defaultVariable = new String("");
        scalarVariableNames = new Vector();
        visualVariableNames = new Vector();
        extentMinima = new Vector();
        extentMaxima = new Vector();
        extentScale = new Vector();
        variableDisplay = VARIABLEDISPLAYTYPE_VARINDEX;
        length = 1;
        steps = 1;
        preserveOriginalCellNumbers = true;
    }

    public ExtrudeStackedAttributes(ExtrudeStackedAttributes obj)
    {
        super(obj);

        int i;

        axis = new double[3];
        axis[0] = obj.axis[0];
        axis[1] = obj.axis[1];
        axis[2] = obj.axis[2];

        byVariable = obj.byVariable;
        defaultVariable = new String(obj.defaultVariable);
        scalarVariableNames = new Vector(obj.scalarVariableNames.size());
        for(i = 0; i < obj.scalarVariableNames.size(); ++i)
            scalarVariableNames.addElement(new String((String)obj.scalarVariableNames.elementAt(i)));

        visualVariableNames = new Vector(obj.visualVariableNames.size());
        for(i = 0; i < obj.visualVariableNames.size(); ++i)
            visualVariableNames.addElement(new String((String)obj.visualVariableNames.elementAt(i)));

        extentMinima = new Vector(obj.extentMinima.size());
        for(i = 0; i < obj.extentMinima.size(); ++i)
        {
            Double dv = (Double)obj.extentMinima.elementAt(i);
            extentMinima.addElement(new Double(dv.doubleValue()));
        }

        extentMaxima = new Vector(obj.extentMaxima.size());
        for(i = 0; i < obj.extentMaxima.size(); ++i)
        {
            Double dv = (Double)obj.extentMaxima.elementAt(i);
            extentMaxima.addElement(new Double(dv.doubleValue()));
        }

        extentScale = new Vector(obj.extentScale.size());
        for(i = 0; i < obj.extentScale.size(); ++i)
        {
            Double dv = (Double)obj.extentScale.elementAt(i);
            extentScale.addElement(new Double(dv.doubleValue()));
        }

        variableDisplay = obj.variableDisplay;
        length = obj.length;
        steps = obj.steps;
        preserveOriginalCellNumbers = obj.preserveOriginalCellNumbers;

        SelectAll();
    }

    public int Offset()
    {
        return super.Offset() + super.GetNumAdditionalAttributes();
    }

    public int GetNumAdditionalAttributes()
    {
        return ExtrudeStackedAttributes_numAdditionalAtts;
    }

    public boolean equals(ExtrudeStackedAttributes obj)
    {
        int i;

        // Compare the axis arrays.
        boolean axis_equal = true;
        for(i = 0; i < 3 && axis_equal; ++i)
            axis_equal = (axis[i] == obj.axis[i]);

        // Compare the elements in the scalarVariableNames vector.
        boolean scalarVariableNames_equal = (obj.scalarVariableNames.size() == scalarVariableNames.size());
        for(i = 0; (i < scalarVariableNames.size()) && scalarVariableNames_equal; ++i)
        {
            // Make references to String from Object.
            String scalarVariableNames1 = (String)scalarVariableNames.elementAt(i);
            String scalarVariableNames2 = (String)obj.scalarVariableNames.elementAt(i);
            scalarVariableNames_equal = scalarVariableNames1.equals(scalarVariableNames2);
        }
        // Compare the elements in the visualVariableNames vector.
        boolean visualVariableNames_equal = (obj.visualVariableNames.size() == visualVariableNames.size());
        for(i = 0; (i < visualVariableNames.size()) && visualVariableNames_equal; ++i)
        {
            // Make references to String from Object.
            String visualVariableNames1 = (String)visualVariableNames.elementAt(i);
            String visualVariableNames2 = (String)obj.visualVariableNames.elementAt(i);
            visualVariableNames_equal = visualVariableNames1.equals(visualVariableNames2);
        }
        // Compare the elements in the extentMinima vector.
        boolean extentMinima_equal = (obj.extentMinima.size() == extentMinima.size());
        for(i = 0; (i < extentMinima.size()) && extentMinima_equal; ++i)
        {
            // Make references to Double from Object.
            Double extentMinima1 = (Double)extentMinima.elementAt(i);
            Double extentMinima2 = (Double)obj.extentMinima.elementAt(i);
            extentMinima_equal = extentMinima1.equals(extentMinima2);
        }
        // Compare the elements in the extentMaxima vector.
        boolean extentMaxima_equal = (obj.extentMaxima.size() == extentMaxima.size());
        for(i = 0; (i < extentMaxima.size()) && extentMaxima_equal; ++i)
        {
            // Make references to Double from Object.
            Double extentMaxima1 = (Double)extentMaxima.elementAt(i);
            Double extentMaxima2 = (Double)obj.extentMaxima.elementAt(i);
            extentMaxima_equal = extentMaxima1.equals(extentMaxima2);
        }
        // Compare the elements in the extentScale vector.
        boolean extentScale_equal = (obj.extentScale.size() == extentScale.size());
        for(i = 0; (i < extentScale.size()) && extentScale_equal; ++i)
        {
            // Make references to Double from Object.
            Double extentScale1 = (Double)extentScale.elementAt(i);
            Double extentScale2 = (Double)obj.extentScale.elementAt(i);
            extentScale_equal = extentScale1.equals(extentScale2);
        }
        // Create the return value
        return (axis_equal &&
                (byVariable == obj.byVariable) &&
                (defaultVariable.equals(obj.defaultVariable)) &&
                scalarVariableNames_equal &&
                visualVariableNames_equal &&
                extentMinima_equal &&
                extentMaxima_equal &&
                extentScale_equal &&
                (variableDisplay == obj.variableDisplay) &&
                (length == obj.length) &&
                (steps == obj.steps) &&
                (preserveOriginalCellNumbers == obj.preserveOriginalCellNumbers));
    }

    public String GetName() { return "ExtrudeStacked"; }
    public String GetVersion() { return "1.0"; }

    // Property setting methods
    public void SetAxis(double[] axis_)
    {
        axis[0] = axis_[0];
        axis[1] = axis_[1];
        axis[2] = axis_[2];
        Select(0);
    }

    public void SetAxis(double e0, double e1, double e2)
    {
        axis[0] = e0;
        axis[1] = e1;
        axis[2] = e2;
        Select(0);
    }

    public void SetByVariable(boolean byVariable_)
    {
        byVariable = byVariable_;
        Select(1);
    }

    public void SetDefaultVariable(String defaultVariable_)
    {
        defaultVariable = defaultVariable_;
        Select(2);
    }

    public void SetScalarVariableNames(Vector scalarVariableNames_)
    {
        scalarVariableNames = scalarVariableNames_;
        Select(3);
    }

    public void SetVisualVariableNames(Vector visualVariableNames_)
    {
        visualVariableNames = visualVariableNames_;
        Select(4);
    }

    public void SetExtentMinima(Vector extentMinima_)
    {
        extentMinima = extentMinima_;
        Select(5);
    }

    public void SetExtentMaxima(Vector extentMaxima_)
    {
        extentMaxima = extentMaxima_;
        Select(6);
    }

    public void SetExtentScale(Vector extentScale_)
    {
        extentScale = extentScale_;
        Select(7);
    }

    public void SetVariableDisplay(int variableDisplay_)
    {
        variableDisplay = variableDisplay_;
        Select(8);
    }

    public void SetLength(double length_)
    {
        length = length_;
        Select(9);
    }

    public void SetSteps(int steps_)
    {
        steps = steps_;
        Select(10);
    }

    public void SetPreserveOriginalCellNumbers(boolean preserveOriginalCellNumbers_)
    {
        preserveOriginalCellNumbers = preserveOriginalCellNumbers_;
        Select(11);
    }

    // Property getting methods
    public double[] GetAxis() { return axis; }
    public boolean  GetByVariable() { return byVariable; }
    public String   GetDefaultVariable() { return defaultVariable; }
    public Vector   GetScalarVariableNames() { return scalarVariableNames; }
    public Vector   GetVisualVariableNames() { return visualVariableNames; }
    public Vector   GetExtentMinima() { return extentMinima; }
    public Vector   GetExtentMaxima() { return extentMaxima; }
    public Vector   GetExtentScale() { return extentScale; }
    public int      GetVariableDisplay() { return variableDisplay; }
    public double   GetLength() { return length; }
    public int      GetSteps() { return steps; }
    public boolean  GetPreserveOriginalCellNumbers() { return preserveOriginalCellNumbers; }

    // Write and read methods.
    public void WriteAtts(CommunicationBuffer buf)
    {
        if(WriteSelect(0, buf))
            buf.WriteDoubleArray(axis);
        if(WriteSelect(1, buf))
            buf.WriteBool(byVariable);
        if(WriteSelect(2, buf))
            buf.WriteString(defaultVariable);
        if(WriteSelect(3, buf))
            buf.WriteStringVector(scalarVariableNames);
        if(WriteSelect(4, buf))
            buf.WriteStringVector(visualVariableNames);
        if(WriteSelect(5, buf))
            buf.WriteDoubleVector(extentMinima);
        if(WriteSelect(6, buf))
            buf.WriteDoubleVector(extentMaxima);
        if(WriteSelect(7, buf))
            buf.WriteDoubleVector(extentScale);
        if(WriteSelect(8, buf))
            buf.WriteInt(variableDisplay);
        if(WriteSelect(9, buf))
            buf.WriteDouble(length);
        if(WriteSelect(10, buf))
            buf.WriteInt(steps);
        if(WriteSelect(11, buf))
            buf.WriteBool(preserveOriginalCellNumbers);
    }

    public void ReadAtts(int index, CommunicationBuffer buf)
    {
        switch(index)
        {
        case 0:
            SetAxis(buf.ReadDoubleArray());
            break;
        case 1:
            SetByVariable(buf.ReadBool());
            break;
        case 2:
            SetDefaultVariable(buf.ReadString());
            break;
        case 3:
            SetScalarVariableNames(buf.ReadStringVector());
            break;
        case 4:
            SetVisualVariableNames(buf.ReadStringVector());
            break;
        case 5:
            SetExtentMinima(buf.ReadDoubleVector());
            break;
        case 6:
            SetExtentMaxima(buf.ReadDoubleVector());
            break;
        case 7:
            SetExtentScale(buf.ReadDoubleVector());
            break;
        case 8:
            SetVariableDisplay(buf.ReadInt());
            break;
        case 9:
            SetLength(buf.ReadDouble());
            break;
        case 10:
            SetSteps(buf.ReadInt());
            break;
        case 11:
            SetPreserveOriginalCellNumbers(buf.ReadBool());
            break;
        }
    }

    public String toString(String indent)
    {
        String str = new String();
        str = str + doubleArrayToString("axis", axis, indent) + "\n";
        str = str + boolToString("byVariable", byVariable, indent) + "\n";
        str = str + stringToString("defaultVariable", defaultVariable, indent) + "\n";
        str = str + stringVectorToString("scalarVariableNames", scalarVariableNames, indent) + "\n";
        str = str + stringVectorToString("visualVariableNames", visualVariableNames, indent) + "\n";
        str = str + doubleVectorToString("extentMinima", extentMinima, indent) + "\n";
        str = str + doubleVectorToString("extentMaxima", extentMaxima, indent) + "\n";
        str = str + doubleVectorToString("extentScale", extentScale, indent) + "\n";
        str = str + indent + "variableDisplay = ";
        if(variableDisplay == VARIABLEDISPLAYTYPE_NODEHEIGHT)
            str = str + "VARIABLEDISPLAYTYPE_NODEHEIGHT";
        if(variableDisplay == VARIABLEDISPLAYTYPE_CELLHEIGHT)
            str = str + "VARIABLEDISPLAYTYPE_CELLHEIGHT";
        if(variableDisplay == VARIABLEDISPLAYTYPE_VARINDEX)
            str = str + "VARIABLEDISPLAYTYPE_VARINDEX";
        str = str + "\n";
        str = str + doubleToString("length", length, indent) + "\n";
        str = str + intToString("steps", steps, indent) + "\n";
        str = str + boolToString("preserveOriginalCellNumbers", preserveOriginalCellNumbers, indent) + "\n";
        return str;
    }


    // Attributes
    private double[] axis;
    private boolean  byVariable;
    private String   defaultVariable;
    private Vector   scalarVariableNames; // vector of String objects
    private Vector   visualVariableNames; // vector of String objects
    private Vector   extentMinima; // vector of Double objects
    private Vector   extentMaxima; // vector of Double objects
    private Vector   extentScale; // vector of Double objects
    private int      variableDisplay;
    private double   length;
    private int      steps;
    private boolean  preserveOriginalCellNumbers;
}

