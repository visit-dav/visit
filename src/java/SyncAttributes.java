package llnl.visit;

import java.util.Vector;

// ****************************************************************************
// Class: SyncAttributes
//
// Purpose:
//   This is a state object that encapsulates the synchronization attributes.
//
// Notes:      
//
// Programmer: Brad Whitlock
// Creation:   Thu Aug 8 12:45:45 PDT 2002
//
// Modifications:
//   
// ****************************************************************************

class SyncAttributes extends AttributeSubject
{
    public SyncAttributes()
    {
        super(1);
        syncTag = -1;
    }

    public void SetSyncTag(int val)
    {
        syncTag = val;
        Select(0);
    }

    public int GetSyncTag()
    {
        return syncTag;
    }

    public void WriteAtts(CommunicationBuffer buf)
    {
        if(WriteSelect(0, buf))
            buf.WriteInt(syncTag);
    }

    public void ReadAtts(int n, CommunicationBuffer buf)
    {
        // Read the index of the selected component.
        buf.ReadByte();
        // Read the component.
        syncTag = buf.ReadInt();
        Select(0);
    }

    private int syncTag;
}
