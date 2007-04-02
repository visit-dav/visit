// ****************************************************************************
//
// Copyright (c) 2000 - 2007, The Regents of the University of California
// Produced at the Lawrence Livermore National Laboratory
// All rights reserved.
//
// This file is part of VisIt. For details, see http://www.llnl.gov/visit/. The
// full copyright notice is contained in the file COPYRIGHT located at the root
// of the VisIt distribution or at http://www.llnl.gov/visit/copyright.html.
//
// Redistribution  and  use  in  source  and  binary  forms,  with  or  without
// modification, are permitted provided that the following conditions are met:
//
//  - Redistributions of  source code must  retain the above  copyright notice,
//    this list of conditions and the disclaimer below.
//  - Redistributions in binary form must reproduce the above copyright notice,
//    this  list of  conditions  and  the  disclaimer (as noted below)  in  the
//    documentation and/or materials provided with the distribution.
//  - Neither the name of the UC/LLNL nor  the names of its contributors may be
//    used to  endorse or  promote products derived from  this software without
//    specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT  HOLDERS AND CONTRIBUTORS "AS IS"
// AND ANY EXPRESS OR  IMPLIED WARRANTIES, INCLUDING,  BUT NOT  LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND  FITNESS FOR A PARTICULAR  PURPOSE
// ARE  DISCLAIMED.  IN  NO  EVENT  SHALL  THE  REGENTS  OF  THE  UNIVERSITY OF
// CALIFORNIA, THE U.S.  DEPARTMENT  OF  ENERGY OR CONTRIBUTORS BE  LIABLE  FOR
// ANY  DIRECT,  INDIRECT,  INCIDENTAL,  SPECIAL,  EXEMPLARY,  OR CONSEQUENTIAL
// DAMAGES (INCLUDING, BUT NOT  LIMITED TO, PROCUREMENT OF  SUBSTITUTE GOODS OR
// SERVICES; LOSS OF  USE, DATA, OR PROFITS; OR  BUSINESS INTERRUPTION) HOWEVER
// CAUSED  AND  ON  ANY  THEORY  OF  LIABILITY,  WHETHER  IN  CONTRACT,  STRICT
// LIABILITY, OR TORT  (INCLUDING NEGLIGENCE OR OTHERWISE)  ARISING IN ANY  WAY
// OUT OF THE  USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH
// DAMAGE.
//
// ****************************************************************************

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

