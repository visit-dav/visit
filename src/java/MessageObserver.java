package llnl.visit;

// ****************************************************************************
// Class: MessageObserver
//
// Purpose:
//   This class is used to observe MessageAttributes in the ViewerProxy. It is
//   used to detect messages coming from the viewer and print them out if
//   desired.
//
// Notes:      
//
// Programmer: Brad Whitlock
// Creation:   Thu Aug 8 12:50:24 PDT 2002
//
// Modifications:
//   
// ****************************************************************************

class MessageObserver implements SimpleObserver
{
    public MessageObserver()
    {
        doUpdate = true;
        messageAtts = null;
        errorFlag = false;
        lastError = new String("");
        verbose = true;
    }

    public void Attach(MessageAttributes atts)
    {
        messageAtts = atts;
        messageAtts.Attach(this);
    }

    public void SetVerbose(boolean val)
    {
        verbose = val;
    }

    public void Update(AttributeSubject s)
    {
        if(messageAtts == null)
            return;

        if(messageAtts.GetSeverity() == MessageAttributes.SEVERITY_ERROR)
        {
            errorFlag = true;
            lastError = new String(messageAtts.GetText());
            if(verbose)
               System.out.println("VisIt: Error - " + messageAtts.GetText());
        }
        else if(verbose)
        {
            if(messageAtts.GetSeverity() == MessageAttributes.SEVERITY_WARNING)
                System.out.println("VisIt: Warning - " + messageAtts.GetText());
            else
                System.out.println("VisIt: Message - " + messageAtts.GetText());
        }
    }

    public void SetUpdate(boolean val) { doUpdate = val; }
    public boolean GetUpdate() { return doUpdate; }

    public void ClearError()
    {
        errorFlag = false;
    }

    public String  GetLastError() { return lastError; }
    public boolean GetErrorFlag() { return errorFlag; }
  
    private boolean           doUpdate;
    private MessageAttributes messageAtts;
    private boolean           errorFlag;
    private String            lastError;
    private boolean           verbose;
}
