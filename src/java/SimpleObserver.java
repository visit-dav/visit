// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

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
//   Brad Whitlock, Tue Feb 26 15:13:30 PST 2008
//   Added javadoc comments.
//
// ****************************************************************************
/**
 * The SimpleObserver interface defines the methods that all observers of
 * AttributeSubject objects must define.
 *
 * @author Brad Whitlock
 */
public interface SimpleObserver
{
    /**
     * This method is called when an object observed by this object
     * is modified and its observers are notified by the state object's
     * Notify() method.
     *
     * @param s The state object that caused the Update() method to
     *          be called.
     */
    public void Update(AttributeSubject s);

    /**
     * Sets a flag in the implementing class that determines whether or
     * not the Update() method should be called in response to the
     * state object's Notify() method being called.
     *
     * @param val true if the Update() method should be called in response to
     *            a Notify().
     */
    public void SetUpdate(boolean val);

    /**
     * Returns that flag indicating whether or not Update() should be 
     * called in response to a state object's Notify().
     * @return The flag indicating whether Update() should be called.
     */
    public boolean GetUpdate();
}
