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
// ****************************************************************************

public abstract class AttributeSubject extends java.lang.Object
{
    public AttributeSubject(int nAtts)
    {
        super();

        observers = new Vector();
        selected = new boolean[nAtts];
        UnSelectAll();
        attributeId = 0;
    }

    public void Attach(SimpleObserver o)
    {
        observers.addElement(o);
    }

    public void Detach(SimpleObserver o)
    {
        observers.remove(o);
    }

    public String GetClassName()
    {
        return getClass().getName();
    }

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

    public void SelectAll()
    {
        for(int i = 0; i < selected.length; ++i)
            selected[i] = true;
    }

    public void UnSelectAll()
    {
        for(int i = 0; i < selected.length; ++i)
            selected[i] = false;
    }

    public int NumAttributes()
    {
        return selected.length;
    }

    public int NumAttributesSelected()
    {
        int sum = 0;
        for(int i = 0; i < selected.length; ++i)
            sum += (selected[i] ? 1 : 0);
        return sum;
    }

    public boolean IsSelected(int index)
    {
        return selected[index];
    }

    public void SetAttributeId(int i)
    {
        attributeId = i;
    }

    public int GetAttributeId()
    {
        return attributeId;
    }

    public void Write(CommunicationBuffer buf)
    {
        if(NumAttributesSelected() == 0)
            SelectAll();

        // Write the number of selected attributes
        buf.WriteByte((byte)NumAttributesSelected());

        // Write the attributes
        WriteAtts(buf);
    }

    public void Read(CommunicationBuffer buf)
    {
        // Make sure that no attributes are selected.
        UnSelectAll();

        // Read the number of attributes to read. If the number is
        // greater than zero, read the attributes.
        int n = (int)buf.ReadByte();
        if(n > 0)
            ReadAtts(n, buf);
    }

    protected void Select(int index)
    {
        selected[index] = true;
    }

    protected boolean WriteSelect(int index, CommunicationBuffer buf)
    {
        if(selected[index])
            buf.WriteByte((byte)index);
        return selected[index];
    }    

    // Abstract methods.
    public abstract void WriteAtts(CommunicationBuffer buf);
    public abstract void ReadAtts(int n, CommunicationBuffer buf);

    private Vector    observers;
    private boolean[] selected;
    private int       attributeId;
}

