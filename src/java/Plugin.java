// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

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
/**
 * Defines the interface that all plugin classes must provide.
 *
 * @author Brad Whitlock
 */
public interface Plugin
{
    /**
     * Returns the plugin's name.
     */
    public String GetName();
    /**
     * Returns the plugin's version string.
     */
    public String GetVersion();
}
