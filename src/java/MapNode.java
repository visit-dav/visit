// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

package llnl.visit;

import java.util.Vector;

// ****************************************************************************
// Class: MapNode
//
// Purpose:
//   Contains a map of variant data -- a dictionary.
//
// Notes:      Maybe this would be better written using Java's HashTable...
//
// Programmer: Brad Whitlock
// Creation:   Thu Feb  2 10:26:33 PST 2012
//
// Modifications:
//   
// ****************************************************************************

public class MapNode extends Variant
{
    public MapNode()
    {
        super();
        entries = new Vector();
    }

    public MapNode(MapNode obj)
    {
        super(obj);
        entries = new Vector(obj.entries);
    }

    public MapNode(boolean value)
    {
        super();
        entries = new Vector();
        SetValue(value);
    }

    public MapNode(byte value)
    {
        super();
        entries = new Vector();
        SetValue(value);
    }

    public MapNode(int value)
    {
        super();
        entries = new Vector();
        SetValue(value);
    }

    public MapNode(long value)
    {
        super();
        entries = new Vector();
        SetValue(value);
    }

    public MapNode(float value)
    {
        super();
        entries = new Vector();
        SetValue(value);
    }

    public MapNode(double value)
    {
        super();
        entries = new Vector();
        SetValue(value);
    }

    public MapNode(String value)
    {
        super();
        entries = new Vector();
        SetValue(value);
    }

    // Write and read methods.
    public void Write(CommunicationBuffer buf)
    {
        buf.WriteInt(Type());

        if(Type() == VARIANT_EMPTY_TYPE)
        {
            buf.WriteInt(entries.size());

            for(int i = 0; i < entries.size(); ++i)
            {
                MapNodePair m = (MapNodePair)entries.elementAt(i);
                buf.WriteString(m.key);

                m.value.Write(buf);
            }
        }
        else
            super.Write(buf);
    }

    public void Read(CommunicationBuffer buf)
    {
        entries.clear();

        // Read the data type.
        dataType = buf.ReadInt();
        dataValue = null;

        if(dataType == VARIANT_EMPTY_TYPE)
        {
            int nEntries = buf.ReadInt();

            for(int i = 0; i < nEntries; ++i)
            {
                MapNodePair pair = new MapNodePair();

                pair.key = buf.ReadString();
                pair.value.Read(buf);
                
                entries.addElement(pair);
            }
        }
        else
            super.Read(buf);
    }

    public MapNode Lookup(String key)
    {
        MapNode retval = null;

        if(dataType == VARIANT_EMPTY_TYPE)
        {
            for(int i = 0; i < entries.size(); ++i)
            {
                MapNodePair pair = (MapNodePair)entries.elementAt(i);
                if(pair.key.equals(key))
                {
                    retval = pair.value;
                    break;
                }
            }
        }

        return retval;
    }

    public void SetValue(String key, MapNode value)
    {
        MapNode dest = Lookup(key);
        if(dest == null)
        {
            MapNodePair pair = new MapNodePair();
            pair.key = key; //new String(key);
            pair.value = value; //new MapNode(value);

            entries.addElement(pair);
        }
        else
            dest = value;
    }

    public String toString(String indent)
    {
        String str = new String();

        if(dataType == VARIANT_EMPTY_TYPE)
        {
            str = str + indent + "{\n";
            String indent2 = new String(indent + "    ");
            for(int i = 0; i < entries.size(); ++i)
            {
                MapNodePair pair = (MapNodePair)entries.elementAt(i);
                if(pair.value.Type() == VARIANT_EMPTY_TYPE)
                {
                    str = str + indent2 + pair.key.toString() + " = {\n";
                    str = str + pair.value.toString(indent2 + "    ") + "\n";
                    str = str + indent2 + "}\n";
                }
                else
                    str = str + indent2 + pair.key + " = " + pair.value.toString("") + "\n";
            }
            str = str + indent + "}";
        }
        else
            str = super.toString(indent);

        return str;
    }

    private Vector entries;
}
