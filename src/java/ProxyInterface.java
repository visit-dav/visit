// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

package llnl.visit;

// ****************************************************************************
// Interface: ProxyInterface
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
/**
 * Defines part of the interface that the ViewerProxy must provide.
 *
 * @author Brad Whitlock
 */
public interface ProxyInterface
{
    public int GetPlotIndex(String type);

    public int GetOperatorIndex(String type);

    public void PrintMessage(String msg);

    public boolean Synchronize();

    public ViewerState GetViewerState();

    public void SetSynchronous(boolean val);

    public boolean GetSynchronous();

    public void SendInterruption();
}
