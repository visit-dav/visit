package llnl.visit;

// ****************************************************************************
// Interface: SimpleObserver
//
// Purpose:
//   Classes must implement this interface if they want to observe state
//   objects. This is important for classes that want to perform some action
//   (like updating a GUI window) when the viewer sends new state.
//
// Notes:      
//
// Programmer: Brad Whitlock
// Creation:   Thu Aug 8 12:46:13 PDT 2002
//
// Modifications:
//   
// ****************************************************************************

public interface SimpleObserver
{
    public void Update(AttributeSubject s);
    public void SetUpdate(boolean val);
    public boolean GetUpdate();
}
