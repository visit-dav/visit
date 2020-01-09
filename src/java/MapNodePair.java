// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

package llnl.visit;

// ****************************************************************************
// Class: MapNodePair
//
// Purpose:
//   This class is just for storage inside of MapNode.
//
// Notes:
//
// Programmer: Brad Whitlock
// Creation:   Thu Feb  2 11:52:44 PST 2012
//
// Modifications:
//
// ****************************************************************************

public class MapNodePair extends java.lang.Object
{
    public MapNodePair()
    {
        super();
        key = new String();
        value = new MapNode();
    }

    public MapNodePair(MapNodePair obj)
    {
        super();
        key = new String(obj.key);
        value = new MapNode(obj.value);
    }

    public String  key;
    public MapNode value;
}
