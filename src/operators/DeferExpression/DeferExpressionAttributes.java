// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

package llnl.visit.operators;

import llnl.visit.AttributeSubject;
import llnl.visit.CommunicationBuffer;
import llnl.visit.Plugin;
import java.util.Vector;

// ****************************************************************************
// Class: DeferExpressionAttributes
//
// Purpose:
//    Attributes for the DeferExpression operator
//
// Notes:      Autogenerated by xml2java.
//
// Programmer: xml2java
// Creation:   omitted
//
// Modifications:
//
// ****************************************************************************

public class DeferExpressionAttributes extends AttributeSubject implements Plugin
{
    private static int DeferExpressionAttributes_numAdditionalAtts = 1;

    public DeferExpressionAttributes()
    {
        super(DeferExpressionAttributes_numAdditionalAtts);

        exprs = new Vector();
    }

    public DeferExpressionAttributes(int nMoreFields)
    {
        super(DeferExpressionAttributes_numAdditionalAtts + nMoreFields);

        exprs = new Vector();
    }

    public DeferExpressionAttributes(DeferExpressionAttributes obj)
    {
        super(obj);

        int i;

        exprs = new Vector(obj.exprs.size());
        for(i = 0; i < obj.exprs.size(); ++i)
            exprs.addElement(new String((String)obj.exprs.elementAt(i)));


        SelectAll();
    }

    public int Offset()
    {
        return super.Offset() + super.GetNumAdditionalAttributes();
    }

    public int GetNumAdditionalAttributes()
    {
        return DeferExpressionAttributes_numAdditionalAtts;
    }

    public boolean equals(DeferExpressionAttributes obj)
    {
        int i;

        // Compare the elements in the exprs vector.
        boolean exprs_equal = (obj.exprs.size() == exprs.size());
        for(i = 0; (i < exprs.size()) && exprs_equal; ++i)
        {
            // Make references to String from Object.
            String exprs1 = (String)exprs.elementAt(i);
            String exprs2 = (String)obj.exprs.elementAt(i);
            exprs_equal = exprs1.equals(exprs2);
        }
        // Create the return value
        return (exprs_equal);
    }

    public String GetName() { return "DeferExpression"; }
    public String GetVersion() { return "1.0"; }

    // Property setting methods
    public void SetExprs(Vector exprs_)
    {
        exprs = exprs_;
        Select(0);
    }

    // Property getting methods
    public Vector GetExprs() { return exprs; }

    // Write and read methods.
    public void WriteAtts(CommunicationBuffer buf)
    {
        if(WriteSelect(0, buf))
            buf.WriteStringVector(exprs);
    }

    public void ReadAtts(int index, CommunicationBuffer buf)
    {
        SetExprs(buf.ReadStringVector());
    }

    public String toString(String indent)
    {
        String str = new String();
        str = str + stringVectorToString("exprs", exprs, indent) + "\n";
        return str;
    }


    // Attributes
    private Vector exprs; // vector of String objects
}

