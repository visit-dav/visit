// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

package llnl.visit;

import java.util.Vector;

// ****************************************************************************
// Class: AttributeSubject
//
// Purpose:
//   This is the base class for all state objects. It contains the basics of
//   determining which attributes in a state object are selected and it also
//   contains the logic to notify observers of the state object when the
//   contents of the state object change.
//
// Notes:
//
// Programmer: Brad Whitlock
// Creation:   Thu Aug 8 12:59:37 PDT 2002
//
// Modifications:
//   Brad Whitlock, Thu Dec 12 12:58:17 PDT 2002
//   Added a method to return the class name.
//
//   Brad Whitlock, Mon Feb 25 14:09:31 PST 2008
//   Added methods to help with string conversion. Added javadoc comments.
//
//   Kathleen Biagas, Fri Mar 31 13:36:59 PDT 2017
//   Added a copy constructor that behaves similar to the constructor that
//   takes an int.
//
//   Kathleen Biagas, Thu Nov 17, 2022
//   Added boolArrayToString and boolVectorToString.
//
// ****************************************************************************
/**
 * Base class for state objects.
 *
 * @author Brad Whitlock
 */
public abstract class AttributeSubject
{
    /**
     * Constructor for the AttributeSubject class.
     * @param nAtts The number of attributes that make up the state object.
     */
    public AttributeSubject(int nAtts)
    {
        super();

        observers = new Vector();
        selected = new boolean[nAtts];
        UnSelectAll();
        attributeId = 0;
    }

    /**
     * Copy Constructor for the AttributeSubject class.
     * @param obj The attribute subject to copy.
     */
    public AttributeSubject(AttributeSubject obj)
    {
        super();

        observers = new Vector();
        selected = new boolean[obj.NumAttributes()];
        UnSelectAll();
        attributeId = 0;
    }

    /**
     * Registers an observer to be called when this object's Notify()
     * method is called.
     * @param o The observer whose Update() method will be called.
     */
    public void Attach(SimpleObserver o)
    {
        observers.addElement(o);
    }

    /**
     * Removes an observer from the list of observers to be called
     * when this object's Notify() method is called.
     * @param o The observer to be removed.
     */
    public void Detach(SimpleObserver o)
    {
        observers.remove(o);
    }

    /**
     * Returns the name of the state object's class.
     * @return The name of the state object's class.
     */
    public String GetClassName()
    {
        return getClass().getName();
    }

    /**
     * Notifies all observers of this object that they need to be updated.
     */
    public void Notify()
    {
        for(int i = 0; i < observers.size(); ++i)
        {
            SimpleObserver ob = (SimpleObserver)observers.elementAt(i);
            if(ob.GetUpdate())
                ob.Update(this);
            else
                ob.SetUpdate(true);
        }

        UnSelectAll();
    }

    /**
     * Selects all of the attributes in the state object so they will all be
     * sent to the viewer when they are transmitted.
     */
    public void SelectAll()
    {
        for(int i = 0; i < selected.length; ++i)
            selected[i] = true;
    }

    /**
     * Unselects all of the fields in the state object.
     */
    public void UnSelectAll()
    {
        for(int i = 0; i < selected.length; ++i)
            selected[i] = false;
    }

    /**
     * Returns the offset into the selected table where this state object's fields begin.
     * @return An offset into the selected table.
     */
    public int Offset()
    {
        return 0;
    }

    /**
     * Returns the total number of attributes in the state object.
     * @return The total number of attributes in the state object.
     */
    public int NumAttributes()
    {
        return selected.length;
    }

    /**
     * Returns the number of attributes added in this level of the state object.
     * @return The number of attributes added in this level of the state object.
     */
    public int GetNumAdditionalAttributes()
    {
        return 0;
    }

    /**
     * Returns the number of selected attributes in the state object.
     * @return The number of selected attributes in the state object.
     */
    public int NumAttributesSelected()
    {
        int sum = 0;
        for(int i = 0; i < selected.length; ++i)
            sum += (selected[i] ? 1 : 0);
        return sum;
    }

    /**
     * Returns whether the specified attribute is selected.
     * @param index The index of the attribute to check.
     * @return true if the attribute is selected; false otherwise.
     */
    public boolean IsSelected(int index)
    {
        return selected[index];
    }

    /**
     * Sets the attribute id, which is the id used in the Xfer object
     * when transmitting this object to the remote process.
     * @param id The new id
     */
    public void SetAttributeId(int id)
    {
        attributeId = id;
    }

    /**
     * Gets the attribute id, which is the id used in the Xfer object
     * when transmitting this object to the remote process.
     * @return The attribute id.
     */
    public int GetAttributeId()
    {
        return attributeId;
    }

    /**
     * Writes the object to a communication buffer, which is usually a
     * socket headed for the remote process.
     * @param buf The communication buffer to which the object will be written.
     */
    public void Write(CommunicationBuffer buf)
    {
        if(NumAttributesSelected() == 0)
            SelectAll();

        // Write the number of selected attributes
        buf.WriteByte((byte)NumAttributesSelected());

        // Write the attributes
        WriteAtts(buf);
    }

    /**
     * Reads the object from a communication buffer, which is usually a
     * socket containing input from the remote process.
     * @param buf The communication buffer from which the object will be read.
     */
    public void Read(CommunicationBuffer buf)
    {
        // Make sure that no attributes are selected.
        UnSelectAll();

        // Read the number of attributes to read. If the number is
        // greater than zero, read the attributes.
        int n = (int)buf.ReadByte();
        for(int i = 0; i < n; ++i)
        {
            int id = (int)buf.ReadByte();
            ReadAtts(id, buf);
        }
    }

    /**
     * Writes the selected fields of the object to the buffer.
     * @param buf The communication buffer to which the object will be written.
     */
    public void WriteAtts(CommunicationBuffer buf)
    {
    }

    /**
     * Reads the specified index's data from the buffer into the object.
     * @param buf The communication buffer to from which the object will be read.
     */
    public abstract void ReadAtts(int index, CommunicationBuffer buf);

    /**
     * Selects the i'th attribute in the object so it will be transmitted
     * to the remote process when written to the communication buffer.
     * @param index The index of the attribute to be selected.
     */
    protected void Select(int index)
    {
        selected[index] = true;
    }

    /**
     * If the attribute is selected then its index is written to the
     * communcation buffer.
     * @param index The index of the attribute to be selected.
     * @param buf Communication buffer
     * @return true if the attribute needs to be written; false otherwise.
     */
    protected boolean WriteSelect(int index, CommunicationBuffer buf)
    {
        if(selected[index])
            buf.WriteByte((byte)index);
        return selected[index];
    }

    // Methods that help in toString implementation.
    protected String boolToString(String name, boolean val, String indent)
    {
        return indent + name + " = " + (val ? "true" : "false");
    }

    protected String boolArrayToString(String name, boolean[] val, String indent)
    {
        String s = indent + name + " = {";
        for(int i = 0; i < val.length; ++i)
        {
            //boolean iv = new boolean(val[i]);
            s = s + String.valueOf(val[i]);
            if(i < val.length - 1)
                s = s + ", ";
        }
        s = s + "}";
        return s;
    }

    protected String boolVectorToString(String name, Vector val, String indent)
    {
        String s = indent + name + " = {";
        for(int i = 0; i < val.size(); ++i)
        {
            s = s + String.valueOf(val.elementAt(i));
            if(i < val.size() - 1)
                s = s + ", ";
        }
        s = s + "}";
        return s;
    }

    protected String intToString(String name, int val, String indent)
    {
        Integer iv = new Integer(val);
        return indent + name + " = " + iv.toString();
    }

    protected String intArrayToString(String name, int[] val, String indent)
    {
        String s = indent + name + " = {";
        for(int i = 0; i < val.length; ++i)
        {
            Integer iv = new Integer(val[i]);
            s = s + iv.toString();
            if(i < val.length - 1)
                s = s + ", ";
        }
        s = s + "}";
        return s;
    }

    protected String intVectorToString(String name, Vector val, String indent)
    {
        String s = indent + name + " = {";
        for(int i = 0; i < val.size(); ++i)
        {
            s = s + val.elementAt(i).toString();
            if(i < val.size() - 1)
                s = s + ", ";
        }
        s = s + "}";
        return s;
    }

    protected String floatToString(String name, float val, String indent)
    {
        Float iv = new Float(val);
        return indent + name + " = " + iv.toString();
    }

    protected String floatArrayToString(String name, float[] val, String indent)
    {
        String s = indent + name + " = {";
        for(int i = 0; i < val.length; ++i)
        {
            Float v = new Float(val[i]);
            s = s + v.toString();
            if(i < val.length - 1)
                s = s + ", ";
        }
        s = s + "}";
        return s;
    }

    protected String floatVectorToString(String name, Vector val, String indent)
    {
        String s = indent + name + " = {";
        for(int i = 0; i < val.size(); ++i)
        {
            s = s + val.elementAt(i).toString();
            if(i < val.size() - 1)
                s = s + ", ";
        }
        s = s + "}";
        return s;
    }

    protected String doubleToString(String name, double val, String indent)
    {
        Double iv = new Double(val);
        return indent + name + " = " + iv.toString();
    }

    protected String doubleArrayToString(String name, double[] val, String indent)
    {
        String s = indent + name + " = {";
        for(int i = 0; i < val.length; ++i)
        {
            Double v = new Double(val[i]);
            s = s + v.toString();
            if(i < val.length - 1)
                s = s + ", ";
        }
        s = s + "}";
        return s;
    }

    protected String doubleVectorToString(String name, Vector val, String indent)
    {
        String s = indent + name + " = {";
        for(int i = 0; i < val.size(); ++i)
        {
            s = s + val.elementAt(i).toString();
            if(i < val.size() - 1)
                s = s + ", ";
        }
        s = s + "}";
        return s;
    }

    protected String ucharToString(String name, byte val, String indent)
    {
        Integer iv = new Integer((int)val);
        return indent + name + " = " + iv.toString();
    }

    protected String ucharArrayToString(String name, byte[] val, String indent)
    {
        String s = indent + name + " = {";
        for(int i = 0; i < val.length; ++i)
        {
            Integer v = new Integer((int)val[i]);
            s = s + v.toString();
            if(i < val.length - 1)
                s = s + ", ";
        }
        s = s + "}";
        return s;
    }

    protected String ucharVectorToString(String name, Vector val, String indent)
    {
        String s = indent + name + " = {";
        for(int i = 0; i < val.size(); ++i)
        {
            Byte b = (Byte)val.elementAt(i);
            Integer v = new Integer(b.intValue());
            s = s + v.toString();
            if(i < val.size() - 1)
                s = s + ", ";
        }
        s = s + "}";
        return s;
    }

    protected String stringToString(String name, String val, String indent)
    {
        return indent + name + " = \"" + val + "\"";
    }

    protected String stringVectorToString(String name, Vector val, String indent)
    {
        String s = indent + name + " = {";
        for(int i = 0; i < val.size(); ++i)
        {
            s = s + "\"" + val.elementAt(i).toString() + "\"";
            if(i < val.size() - 1)
                s = s + ", ";
        }
        s = s + "}";
        return s;
    }

    public String toString(String indent)
    {
        return indent;
    }

    public String toString()
    {
        return toString(new String());
    }

    private Vector    observers;
    private boolean[] selected;
    private int       attributeId;
}

