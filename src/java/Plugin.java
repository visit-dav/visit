package llnl.visit;

// ****************************************************************************
// Interface: Plugin
//
// Purpose:
//   This interface is implemented by plot and operator plugin classes.
//
// Notes:      
//
// Programmer: Brad Whitlock
// Creation:   Wed Aug 14 13:46:57 PST 2002
//
// Modifications:
//   
// ****************************************************************************

public interface Plugin
{
    public String GetName();
    public String GetVersion();
}
