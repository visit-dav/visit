package llnl.visit;

// ****************************************************************************
// Class: LostConnectionException
//
// Purpose:
//   This is an exception class that is thrown when the Java interface loses
//   contact with VisIt's viewer such as when the viewer dies.
//
// Notes:      
//
// Programmer: Brad Whitlock
// Creation:   Thu Aug 8 12:52:57 PDT 2002
//
// Modifications:
//   
// ****************************************************************************

public class LostConnectionException extends java.lang.Exception
{
    public LostConnectionException()
    {
    }
}
