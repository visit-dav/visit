// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

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
