// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

package llnl.visit.operators;

import llnl.visit.AttributeSubject;
import llnl.visit.CommunicationBuffer;
import llnl.visit.Plugin;

// ****************************************************************************
// Class: MergeOperatorAttributes
//
// Purpose:
//    Attributes for Merge operaetor
//
// Notes:      Autogenerated by xml2java.
//
// Programmer: xml2java
// Creation:   omitted
//
// Modifications:
//   
// ****************************************************************************

public class MergeOperatorAttributes extends AttributeSubject implements Plugin
{
    private static int MergeOperatorAttributes_numAdditionalAtts = 2;

    public MergeOperatorAttributes()
    {
        super(MergeOperatorAttributes_numAdditionalAtts);

        parallelMerge = false;
        tolerance = 0;
    }

    public MergeOperatorAttributes(int nMoreFields)
    {
        super(MergeOperatorAttributes_numAdditionalAtts + nMoreFields);

        parallelMerge = false;
        tolerance = 0;
    }

    public MergeOperatorAttributes(MergeOperatorAttributes obj)
    {
        super(obj);

        parallelMerge = obj.parallelMerge;
        tolerance = obj.tolerance;

        SelectAll();
    }

    public int Offset()
    {
        return super.Offset() + super.GetNumAdditionalAttributes();
    }

    public int GetNumAdditionalAttributes()
    {
        return MergeOperatorAttributes_numAdditionalAtts;
    }

    public boolean equals(MergeOperatorAttributes obj)
    {
        // Create the return value
        return ((parallelMerge == obj.parallelMerge) &&
                (tolerance == obj.tolerance));
    }

    public String GetName() { return "Merge"; }
    public String GetVersion() { return "1.0"; }

    // Property setting methods
    public void SetParallelMerge(boolean parallelMerge_)
    {
        parallelMerge = parallelMerge_;
        Select(0);
    }

    public void SetTolerance(double tolerance_)
    {
        tolerance = tolerance_;
        Select(1);
    }

    // Property getting methods
    public boolean GetParallelMerge() { return parallelMerge; }
    public double  GetTolerance() { return tolerance; }

    // Write and read methods.
    public void WriteAtts(CommunicationBuffer buf)
    {
        if(WriteSelect(0, buf))
            buf.WriteBool(parallelMerge);
        if(WriteSelect(1, buf))
            buf.WriteDouble(tolerance);
    }

    public void ReadAtts(int index, CommunicationBuffer buf)
    {
        switch(index)
        {
        case 0:
            SetParallelMerge(buf.ReadBool());
            break;
        case 1:
            SetTolerance(buf.ReadDouble());
            break;
        }
    }

    public String toString(String indent)
    {
        String str = new String();
        str = str + boolToString("parallelMerge", parallelMerge, indent) + "\n";
        str = str + doubleToString("tolerance", tolerance, indent) + "\n";
        return str;
    }


    // Attributes
    private boolean parallelMerge;
    private double  tolerance;
}

