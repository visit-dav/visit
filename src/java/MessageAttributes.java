package llnl.visit;

// ****************************************************************************
// Class: MessageAttributes
//
// Purpose:
//   This class implements the MessageAttributes state object that the viewer
//   sends to the Java interface. This class contains messages that are
//   generally of importance to the user.
//
// Notes:      
//
// Programmer: Brad Whitlock
// Creation:   Thu Aug 8 12:51:24 PDT 2002
//
// Modifications:
//   
// ****************************************************************************

public class MessageAttributes extends AttributeSubject
{
    public MessageAttributes()
    {
        super(2);
        text = new String("");
    }

    public void SetText(String msg)
    {
        text = msg;
        Select(0);
    }

    public void SetSeverity(int val)
    {
        severity = val;
        Select(1);
    }

    public String GetText()     { return text; }
    public int    GetSeverity() { return severity; }

    public void WriteAtts(CommunicationBuffer buf)
    {
        if(WriteSelect(0, buf))
            buf.WriteString(text);
        if(WriteSelect(1, buf))
            buf.WriteInt(severity);
    }

    public void ReadAtts(int n, CommunicationBuffer buf)
    {
        for(int i = 0; i < n; ++i)
        {
            byte index = buf.ReadByte();
            switch(index)
            {
            case 0:
                SetText(buf.ReadString());
                break;
            case 1:
                SetSeverity(buf.ReadInt());
                break;
            }
        }
    }

    public final static int MSG_ERROR   = 0;
    public final static int MSG_WARNING = 1;
    public final static int MSG_MESSAGE = 2;

    private String text;
    private int    severity;
}
