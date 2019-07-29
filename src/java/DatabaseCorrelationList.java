// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

package llnl.visit;

import java.util.Vector;

// ****************************************************************************
// Class: DatabaseCorrelationList
//
// Purpose:
//    This class contains the database correlations that we know about.
//
// Notes:      Autogenerated by xml2java.
//
// Programmer: xml2java
// Creation:   omitted
//
// Modifications:
//   
// ****************************************************************************

public class DatabaseCorrelationList extends AttributeSubject
{
    private static int DatabaseCorrelationList_numAdditionalAtts = 4;

    // Enum values
    public final static int WHENTOCORRELATE_CORRELATEALWAYS = 0;
    public final static int WHENTOCORRELATE_CORRELATENEVER = 1;
    public final static int WHENTOCORRELATE_CORRELATEONLYIFSAMELENGTH = 2;


    public DatabaseCorrelationList()
    {
        super(DatabaseCorrelationList_numAdditionalAtts);

        correlations = new Vector();
        needPermission = true;
        defaultCorrelationMethod = 0;
        whenToCorrelate = WHENTOCORRELATE_CORRELATEONLYIFSAMELENGTH;
    }

    public DatabaseCorrelationList(int nMoreFields)
    {
        super(DatabaseCorrelationList_numAdditionalAtts + nMoreFields);

        correlations = new Vector();
        needPermission = true;
        defaultCorrelationMethod = 0;
        whenToCorrelate = WHENTOCORRELATE_CORRELATEONLYIFSAMELENGTH;
    }

    public DatabaseCorrelationList(DatabaseCorrelationList obj)
    {
        super(obj);

        int i;

        // *** Copy the correlations field ***
        correlations = new Vector(obj.correlations.size());
        for(i = 0; i < obj.correlations.size(); ++i)
        {
            DatabaseCorrelation oldObj = (DatabaseCorrelation)obj.correlations.elementAt(i);
            correlations.addElement(new DatabaseCorrelation(oldObj));
        }

        needPermission = obj.needPermission;
        defaultCorrelationMethod = obj.defaultCorrelationMethod;
        whenToCorrelate = obj.whenToCorrelate;

        SelectAll();
    }

    public int Offset()
    {
        return super.Offset() + super.GetNumAdditionalAttributes();
    }

    public int GetNumAdditionalAttributes()
    {
        return DatabaseCorrelationList_numAdditionalAtts;
    }

    public boolean equals(DatabaseCorrelationList obj)
    {
        int i;

        // Compare the elements in the correlations vector.
        boolean correlations_equal = (obj.correlations.size() == correlations.size());
        for(i = 0; (i < correlations.size()) && correlations_equal; ++i)
        {
            // Make references to DatabaseCorrelation from Object.
            DatabaseCorrelation correlations1 = (DatabaseCorrelation)correlations.elementAt(i);
            DatabaseCorrelation correlations2 = (DatabaseCorrelation)obj.correlations.elementAt(i);
            correlations_equal = correlations1.equals(correlations2);
        }
        // Create the return value
        return (correlations_equal &&
                (needPermission == obj.needPermission) &&
                (defaultCorrelationMethod == obj.defaultCorrelationMethod) &&
                (whenToCorrelate == obj.whenToCorrelate));
    }

    // Property setting methods
    public void SetNeedPermission(boolean needPermission_)
    {
        needPermission = needPermission_;
        Select(1);
    }

    public void SetDefaultCorrelationMethod(int defaultCorrelationMethod_)
    {
        defaultCorrelationMethod = defaultCorrelationMethod_;
        Select(2);
    }

    public void SetWhenToCorrelate(int whenToCorrelate_)
    {
        whenToCorrelate = whenToCorrelate_;
        Select(3);
    }

    // Property getting methods
    public Vector  GetCorrelations() { return correlations; }
    public boolean GetNeedPermission() { return needPermission; }
    public int     GetDefaultCorrelationMethod() { return defaultCorrelationMethod; }
    public int     GetWhenToCorrelate() { return whenToCorrelate; }

    // Write and read methods.
    public void WriteAtts(CommunicationBuffer buf)
    {
        if(WriteSelect(0, buf))
        {
            buf.WriteInt(correlations.size());
            for(int i = 0; i < correlations.size(); ++i)
            {
                DatabaseCorrelation tmp = (DatabaseCorrelation)correlations.elementAt(i);
                tmp.Write(buf);
            }
        }
        if(WriteSelect(1, buf))
            buf.WriteBool(needPermission);
        if(WriteSelect(2, buf))
            buf.WriteInt(defaultCorrelationMethod);
        if(WriteSelect(3, buf))
            buf.WriteInt(whenToCorrelate);
    }

    public void ReadAtts(int index, CommunicationBuffer buf)
    {
        switch(index)
        {
        case 0:
            {
                int len = buf.ReadInt();
                correlations.clear();
                for(int j = 0; j < len; ++j)
                {
                    DatabaseCorrelation tmp = new DatabaseCorrelation();
                    tmp.Read(buf);
                    correlations.addElement(tmp);
                }
            }
            Select(0);
            break;
        case 1:
            SetNeedPermission(buf.ReadBool());
            break;
        case 2:
            SetDefaultCorrelationMethod(buf.ReadInt());
            break;
        case 3:
            SetWhenToCorrelate(buf.ReadInt());
            break;
        }
    }

    public String toString(String indent)
    {
        String str = new String();
        str = str + indent + "correlations = {\n";
        for(int i = 0; i < correlations.size(); ++i)
        {
            AttributeSubject s = (AttributeSubject)correlations.elementAt(i);
            str = str + s.toString(indent + "    ");
            if(i < correlations.size()-1)
                str = str + ", ";
            str = str + "\n";
        }
        str = str + "}\n";
        str = str + boolToString("needPermission", needPermission, indent) + "\n";
        str = str + intToString("defaultCorrelationMethod", defaultCorrelationMethod, indent) + "\n";
        str = str + indent + "whenToCorrelate = ";
        if(whenToCorrelate == WHENTOCORRELATE_CORRELATEALWAYS)
            str = str + "WHENTOCORRELATE_CORRELATEALWAYS";
        if(whenToCorrelate == WHENTOCORRELATE_CORRELATENEVER)
            str = str + "WHENTOCORRELATE_CORRELATENEVER";
        if(whenToCorrelate == WHENTOCORRELATE_CORRELATEONLYIFSAMELENGTH)
            str = str + "WHENTOCORRELATE_CORRELATEONLYIFSAMELENGTH";
        str = str + "\n";
        return str;
    }

    // Attributegroup convenience methods
    public void AddCorrelations(DatabaseCorrelation obj)
    {
        correlations.addElement(new DatabaseCorrelation(obj));
        Select(0);
    }

    public void ClearCorrelations()
    {
        correlations.clear();
        Select(0);
    }

    public void RemoveCorrelations(int index)
    {
        if(index >= 0 && index < correlations.size())
        {
            correlations.remove(index);
            Select(0);
        }
    }

    public int GetNumCorrelations()
    {
        return correlations.size();
    }

    public DatabaseCorrelation GetCorrelations(int i)
    {
        DatabaseCorrelation tmp = (DatabaseCorrelation)correlations.elementAt(i);
        return tmp;
    }


    // Attributes
    private Vector  correlations; // vector of DatabaseCorrelation objects
    private boolean needPermission;
    private int     defaultCorrelationMethod;
    private int     whenToCorrelate;
}

