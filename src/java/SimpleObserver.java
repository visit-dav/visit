// ****************************************************************************
//
// Copyright (c) 2000 - 2009, Lawrence Livermore National Security, LLC
// Produced at the Lawrence Livermore National Laboratory
// LLNL-CODE-400124
// All rights reserved.
//
// This file is  part of VisIt. For  details, see https://visit.llnl.gov/.  The
// full copyright notice is contained in the file COPYRIGHT located at the root
// of the VisIt distribution or at http://www.llnl.gov/visit/copyright.html.
//
// Redistribution  and  use  in  source  and  binary  forms,  with  or  without
// modification, are permitted provided that the following conditions are met:
//
//  - Redistributions of  source code must  retain the above  copyright notice,
//    this list of conditions and the disclaimer below.
//  - Redistributions in binary form must reproduce the above copyright notice,
//    this  list of  conditions  and  the  disclaimer (as noted below)  in  the
//    documentation and/or other materials provided with the distribution.
//  - Neither the name of  the LLNS/LLNL nor the names of  its contributors may
//    be used to endorse or promote products derived from this software without
//    specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT  HOLDERS AND CONTRIBUTORS "AS IS"
// AND ANY EXPRESS OR  IMPLIED WARRANTIES, INCLUDING,  BUT NOT  LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND  FITNESS FOR A PARTICULAR  PURPOSE
// ARE  DISCLAIMED. IN  NO EVENT  SHALL LAWRENCE  LIVERMORE NATIONAL  SECURITY,
// LLC, THE  U.S.  DEPARTMENT OF  ENERGY  OR  CONTRIBUTORS BE  LIABLE  FOR  ANY
// DIRECT,  INDIRECT,   INCIDENTAL,   SPECIAL,   EXEMPLARY,  OR   CONSEQUENTIAL
// DAMAGES (INCLUDING, BUT NOT  LIMITED TO, PROCUREMENT OF  SUBSTITUTE GOODS OR
// SERVICES; LOSS OF  USE, DATA, OR PROFITS; OR  BUSINESS INTERRUPTION) HOWEVER
// CAUSED  AND  ON  ANY  THEORY  OF  LIABILITY,  WHETHER  IN  CONTRACT,  STRICT
// LIABILITY, OR TORT  (INCLUDING NEGLIGENCE OR OTHERWISE)  ARISING IN ANY  WAY
// OUT OF THE  USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH
// DAMAGE.
//
// ****************************************************************************

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
