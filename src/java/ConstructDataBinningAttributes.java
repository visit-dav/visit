// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

package llnl.visit;

import java.util.Vector;
import java.lang.Byte;
import java.lang.Double;
import java.lang.Integer;

// ****************************************************************************
// Class: ConstructDataBinningAttributes
//
// Purpose:
//    Attributes for constructing a data binning
//
// Notes:      Autogenerated by xml2java.
//
// Programmer: xml2java
// Creation:   omitted
//
// Modifications:
//
// ****************************************************************************

public class ConstructDataBinningAttributes extends AttributeSubject
{
    private static int ConstructDataBinningAttributes_numAdditionalAtts = 14;

    // Enum values
    public final static int BINNINGSCHEME_UNIFORM = 0;
    public final static int BINNINGSCHEME_UNKNOWN = 1;

    public final static int REDUCTIONOPERATOR_AVERAGE = 0;
    public final static int REDUCTIONOPERATOR_MINIMUM = 1;
    public final static int REDUCTIONOPERATOR_MAXIMUM = 2;
    public final static int REDUCTIONOPERATOR_STANDARDDEVIATION = 3;
    public final static int REDUCTIONOPERATOR_VARIANCE = 4;
    public final static int REDUCTIONOPERATOR_SUM = 5;
    public final static int REDUCTIONOPERATOR_COUNT = 6;
    public final static int REDUCTIONOPERATOR_RMS = 7;
    public final static int REDUCTIONOPERATOR_PDF = 8;

    public final static int OUTOFBOUNDSBEHAVIOR_CLAMP = 0;
    public final static int OUTOFBOUNDSBEHAVIOR_DISCARD = 1;

    public final static int BINTYPE_VARIABLE = 0;
    public final static int BINTYPE_X = 1;
    public final static int BINTYPE_Y = 2;
    public final static int BINTYPE_Z = 3;


    public ConstructDataBinningAttributes()
    {
        super(ConstructDataBinningAttributes_numAdditionalAtts);

        name = new String("");
        varnames = new Vector();
        binType = new Vector();
        binBoundaries = new Vector();
        reductionOperator = REDUCTIONOPERATOR_AVERAGE;
        varForReductionOperator = new String("");
        undefinedValue = 0;
        binningScheme = BINNINGSCHEME_UNIFORM;
        numBins = new Vector();
        overTime = false;
        timeStart = 0;
        timeEnd = 1;
        timeStride = 1;
        outOfBoundsBehavior = OUTOFBOUNDSBEHAVIOR_CLAMP;
    }

    public ConstructDataBinningAttributes(int nMoreFields)
    {
        super(ConstructDataBinningAttributes_numAdditionalAtts + nMoreFields);

        name = new String("");
        varnames = new Vector();
        binType = new Vector();
        binBoundaries = new Vector();
        reductionOperator = REDUCTIONOPERATOR_AVERAGE;
        varForReductionOperator = new String("");
        undefinedValue = 0;
        binningScheme = BINNINGSCHEME_UNIFORM;
        numBins = new Vector();
        overTime = false;
        timeStart = 0;
        timeEnd = 1;
        timeStride = 1;
        outOfBoundsBehavior = OUTOFBOUNDSBEHAVIOR_CLAMP;
    }

    public ConstructDataBinningAttributes(ConstructDataBinningAttributes obj)
    {
        super(obj);

        int i;

        name = new String(obj.name);
        varnames = new Vector(obj.varnames.size());
        for(i = 0; i < obj.varnames.size(); ++i)
            varnames.addElement(new String((String)obj.varnames.elementAt(i)));

        binType = new Vector(obj.binType.size());
        for(i = 0; i < obj.binType.size(); ++i)
        {
            Byte bv = (Byte)obj.binType.elementAt(i);
            binType.addElement(new Byte(bv.byteValue()));
        }

        binBoundaries = new Vector(obj.binBoundaries.size());
        for(i = 0; i < obj.binBoundaries.size(); ++i)
        {
            Double dv = (Double)obj.binBoundaries.elementAt(i);
            binBoundaries.addElement(new Double(dv.doubleValue()));
        }

        reductionOperator = obj.reductionOperator;
        varForReductionOperator = new String(obj.varForReductionOperator);
        undefinedValue = obj.undefinedValue;
        binningScheme = obj.binningScheme;
        numBins = new Vector();
        for(i = 0; i < obj.numBins.size(); ++i)
        {
            Integer iv = (Integer)obj.numBins.elementAt(i);
            numBins.addElement(new Integer(iv.intValue()));
        }
        overTime = obj.overTime;
        timeStart = obj.timeStart;
        timeEnd = obj.timeEnd;
        timeStride = obj.timeStride;
        outOfBoundsBehavior = obj.outOfBoundsBehavior;

        SelectAll();
    }

    public int Offset()
    {
        return super.Offset() + super.GetNumAdditionalAttributes();
    }

    public int GetNumAdditionalAttributes()
    {
        return ConstructDataBinningAttributes_numAdditionalAtts;
    }

    public boolean equals(ConstructDataBinningAttributes obj)
    {
        int i;

        // Compare the elements in the varnames vector.
        boolean varnames_equal = (obj.varnames.size() == varnames.size());
        for(i = 0; (i < varnames.size()) && varnames_equal; ++i)
        {
            // Make references to String from Object.
            String varnames1 = (String)varnames.elementAt(i);
            String varnames2 = (String)obj.varnames.elementAt(i);
            varnames_equal = varnames1.equals(varnames2);
        }
        // Compare the elements in the binType vector.
        boolean binType_equal = (obj.binType.size() == binType.size());
        for(i = 0; (i < binType.size()) && binType_equal; ++i)
        {
            // Make references to Byte from Object.
            Byte binType1 = (Byte)binType.elementAt(i);
            Byte binType2 = (Byte)obj.binType.elementAt(i);
            binType_equal = binType1.equals(binType2);
        }
        // Compare the elements in the binBoundaries vector.
        boolean binBoundaries_equal = (obj.binBoundaries.size() == binBoundaries.size());
        for(i = 0; (i < binBoundaries.size()) && binBoundaries_equal; ++i)
        {
            // Make references to Double from Object.
            Double binBoundaries1 = (Double)binBoundaries.elementAt(i);
            Double binBoundaries2 = (Double)obj.binBoundaries.elementAt(i);
            binBoundaries_equal = binBoundaries1.equals(binBoundaries2);
        }
        // Compare the elements in the numBins vector.
        boolean numBins_equal = (obj.numBins.size() == numBins.size());
        for(i = 0; (i < numBins.size()) && numBins_equal; ++i)
        {
            // Make references to Integer from Object.
            Integer numBins1 = (Integer)numBins.elementAt(i);
            Integer numBins2 = (Integer)obj.numBins.elementAt(i);
            numBins_equal = numBins1.equals(numBins2);
        }
        // Create the return value
        return ((name.equals(obj.name)) &&
                varnames_equal &&
                binType_equal &&
                binBoundaries_equal &&
                (reductionOperator == obj.reductionOperator) &&
                (varForReductionOperator.equals(obj.varForReductionOperator)) &&
                (undefinedValue == obj.undefinedValue) &&
                (binningScheme == obj.binningScheme) &&
                numBins_equal &&
                (overTime == obj.overTime) &&
                (timeStart == obj.timeStart) &&
                (timeEnd == obj.timeEnd) &&
                (timeStride == obj.timeStride) &&
                (outOfBoundsBehavior == obj.outOfBoundsBehavior));
    }

    // Property setting methods
    public void SetName(String name_)
    {
        name = name_;
        Select(0);
    }

    public void SetVarnames(Vector varnames_)
    {
        varnames = varnames_;
        Select(1);
    }

    public void SetBinType(Vector binType_)
    {
        binType = binType_;
        Select(2);
    }

    public void SetBinBoundaries(Vector binBoundaries_)
    {
        binBoundaries = binBoundaries_;
        Select(3);
    }

    public void SetReductionOperator(int reductionOperator_)
    {
        reductionOperator = reductionOperator_;
        Select(4);
    }

    public void SetVarForReductionOperator(String varForReductionOperator_)
    {
        varForReductionOperator = varForReductionOperator_;
        Select(5);
    }

    public void SetUndefinedValue(double undefinedValue_)
    {
        undefinedValue = undefinedValue_;
        Select(6);
    }

    public void SetBinningScheme(int binningScheme_)
    {
        binningScheme = binningScheme_;
        Select(7);
    }

    public void SetNumBins(Vector numBins_)
    {
        numBins = numBins_;
        Select(8);
    }

    public void SetOverTime(boolean overTime_)
    {
        overTime = overTime_;
        Select(9);
    }

    public void SetTimeStart(int timeStart_)
    {
        timeStart = timeStart_;
        Select(10);
    }

    public void SetTimeEnd(int timeEnd_)
    {
        timeEnd = timeEnd_;
        Select(11);
    }

    public void SetTimeStride(int timeStride_)
    {
        timeStride = timeStride_;
        Select(12);
    }

    public void SetOutOfBoundsBehavior(int outOfBoundsBehavior_)
    {
        outOfBoundsBehavior = outOfBoundsBehavior_;
        Select(13);
    }

    // Property getting methods
    public String  GetName() { return name; }
    public Vector  GetVarnames() { return varnames; }
    public Vector  GetBinType() { return binType; }
    public Vector  GetBinBoundaries() { return binBoundaries; }
    public int     GetReductionOperator() { return reductionOperator; }
    public String  GetVarForReductionOperator() { return varForReductionOperator; }
    public double  GetUndefinedValue() { return undefinedValue; }
    public int     GetBinningScheme() { return binningScheme; }
    public Vector  GetNumBins() { return numBins; }
    public boolean GetOverTime() { return overTime; }
    public int     GetTimeStart() { return timeStart; }
    public int     GetTimeEnd() { return timeEnd; }
    public int     GetTimeStride() { return timeStride; }
    public int     GetOutOfBoundsBehavior() { return outOfBoundsBehavior; }

    // Write and read methods.
    public void WriteAtts(CommunicationBuffer buf)
    {
        if(WriteSelect(0, buf))
            buf.WriteString(name);
        if(WriteSelect(1, buf))
            buf.WriteStringVector(varnames);
        if(WriteSelect(2, buf))
            buf.WriteByteVector(binType);
        if(WriteSelect(3, buf))
            buf.WriteDoubleVector(binBoundaries);
        if(WriteSelect(4, buf))
            buf.WriteInt(reductionOperator);
        if(WriteSelect(5, buf))
            buf.WriteString(varForReductionOperator);
        if(WriteSelect(6, buf))
            buf.WriteDouble(undefinedValue);
        if(WriteSelect(7, buf))
            buf.WriteInt(binningScheme);
        if(WriteSelect(8, buf))
            buf.WriteIntVector(numBins);
        if(WriteSelect(9, buf))
            buf.WriteBool(overTime);
        if(WriteSelect(10, buf))
            buf.WriteInt(timeStart);
        if(WriteSelect(11, buf))
            buf.WriteInt(timeEnd);
        if(WriteSelect(12, buf))
            buf.WriteInt(timeStride);
        if(WriteSelect(13, buf))
            buf.WriteInt(outOfBoundsBehavior);
    }

    public void ReadAtts(int index, CommunicationBuffer buf)
    {
        switch(index)
        {
        case 0:
            SetName(buf.ReadString());
            break;
        case 1:
            SetVarnames(buf.ReadStringVector());
            break;
        case 2:
            SetBinType(buf.ReadByteVector());
            break;
        case 3:
            SetBinBoundaries(buf.ReadDoubleVector());
            break;
        case 4:
            SetReductionOperator(buf.ReadInt());
            break;
        case 5:
            SetVarForReductionOperator(buf.ReadString());
            break;
        case 6:
            SetUndefinedValue(buf.ReadDouble());
            break;
        case 7:
            SetBinningScheme(buf.ReadInt());
            break;
        case 8:
            SetNumBins(buf.ReadIntVector());
            break;
        case 9:
            SetOverTime(buf.ReadBool());
            break;
        case 10:
            SetTimeStart(buf.ReadInt());
            break;
        case 11:
            SetTimeEnd(buf.ReadInt());
            break;
        case 12:
            SetTimeStride(buf.ReadInt());
            break;
        case 13:
            SetOutOfBoundsBehavior(buf.ReadInt());
            break;
        }
    }

    public String toString(String indent)
    {
        String str = new String();
        str = str + stringToString("name", name, indent) + "\n";
        str = str + stringVectorToString("varnames", varnames, indent) + "\n";
        str = str + ucharVectorToString("binType", binType, indent) + "\n";
        str = str + doubleVectorToString("binBoundaries", binBoundaries, indent) + "\n";
        str = str + indent + "reductionOperator = ";
        if(reductionOperator == REDUCTIONOPERATOR_AVERAGE)
            str = str + "REDUCTIONOPERATOR_AVERAGE";
        if(reductionOperator == REDUCTIONOPERATOR_MINIMUM)
            str = str + "REDUCTIONOPERATOR_MINIMUM";
        if(reductionOperator == REDUCTIONOPERATOR_MAXIMUM)
            str = str + "REDUCTIONOPERATOR_MAXIMUM";
        if(reductionOperator == REDUCTIONOPERATOR_STANDARDDEVIATION)
            str = str + "REDUCTIONOPERATOR_STANDARDDEVIATION";
        if(reductionOperator == REDUCTIONOPERATOR_VARIANCE)
            str = str + "REDUCTIONOPERATOR_VARIANCE";
        if(reductionOperator == REDUCTIONOPERATOR_SUM)
            str = str + "REDUCTIONOPERATOR_SUM";
        if(reductionOperator == REDUCTIONOPERATOR_COUNT)
            str = str + "REDUCTIONOPERATOR_COUNT";
        if(reductionOperator == REDUCTIONOPERATOR_RMS)
            str = str + "REDUCTIONOPERATOR_RMS";
        if(reductionOperator == REDUCTIONOPERATOR_PDF)
            str = str + "REDUCTIONOPERATOR_PDF";
        str = str + "\n";
        str = str + stringToString("varForReductionOperator", varForReductionOperator, indent) + "\n";
        str = str + doubleToString("undefinedValue", undefinedValue, indent) + "\n";
        str = str + indent + "binningScheme = ";
        if(binningScheme == BINNINGSCHEME_UNIFORM)
            str = str + "BINNINGSCHEME_UNIFORM";
        if(binningScheme == BINNINGSCHEME_UNKNOWN)
            str = str + "BINNINGSCHEME_UNKNOWN";
        str = str + "\n";
        str = str + intVectorToString("numBins", numBins, indent) + "\n";
        str = str + boolToString("overTime", overTime, indent) + "\n";
        str = str + intToString("timeStart", timeStart, indent) + "\n";
        str = str + intToString("timeEnd", timeEnd, indent) + "\n";
        str = str + intToString("timeStride", timeStride, indent) + "\n";
        str = str + indent + "outOfBoundsBehavior = ";
        if(outOfBoundsBehavior == OUTOFBOUNDSBEHAVIOR_CLAMP)
            str = str + "OUTOFBOUNDSBEHAVIOR_CLAMP";
        if(outOfBoundsBehavior == OUTOFBOUNDSBEHAVIOR_DISCARD)
            str = str + "OUTOFBOUNDSBEHAVIOR_DISCARD";
        str = str + "\n";
        return str;
    }


    // Attributes
    private String  name;
    private Vector  varnames; // vector of String objects
    private Vector  binType; // vector of Byte objects
    private Vector  binBoundaries; // vector of Double objects
    private int     reductionOperator;
    private String  varForReductionOperator;
    private double  undefinedValue;
    private int     binningScheme;
    private Vector  numBins; // vector of Integer objects
    private boolean overTime;
    private int     timeStart;
    private int     timeEnd;
    private int     timeStride;
    private int     outOfBoundsBehavior;
}

