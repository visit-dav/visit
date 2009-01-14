/*****************************************************************************
*
* Copyright (c) 2000 - 2008, Lawrence Livermore National Security, LLC
* Produced at the Lawrence Livermore National Laboratory
* LLNL-CODE-400142
* All rights reserved.
*
* This file is  part of VisIt. For  details, see https://visit.llnl.gov/.  The
* full copyright notice is contained in the file COPYRIGHT located at the root
* of the VisIt distribution or at http://www.llnl.gov/visit/copyright.html.
*
* Redistribution  and  use  in  source  and  binary  forms,  with  or  without
* modification, are permitted provided that the following conditions are met:
*
*  - Redistributions of  source code must  retain the above  copyright notice,
*    this list of conditions and the disclaimer below.
*  - Redistributions in binary form must reproduce the above copyright notice,
*    this  list of  conditions  and  the  disclaimer (as noted below)  in  the
*    documentation and/or other materials provided with the distribution.
*  - Neither the name of  the LLNS/LLNL nor the names of  its contributors may
*    be used to endorse or promote products derived from this software without
*    specific prior written permission.
*
* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT  HOLDERS AND CONTRIBUTORS "AS IS"
* AND ANY EXPRESS OR  IMPLIED WARRANTIES, INCLUDING,  BUT NOT  LIMITED TO, THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND  FITNESS FOR A PARTICULAR  PURPOSE
* ARE  DISCLAIMED. IN  NO EVENT  SHALL LAWRENCE  LIVERMORE NATIONAL  SECURITY,
* LLC, THE  U.S.  DEPARTMENT OF  ENERGY  OR  CONTRIBUTORS BE  LIABLE  FOR  ANY
* DIRECT,  INDIRECT,   INCIDENTAL,   SPECIAL,   EXEMPLARY,  OR   CONSEQUENTIAL
* DAMAGES (INCLUDING, BUT NOT  LIMITED TO, PROCUREMENT OF  SUBSTITUTE GOODS OR
* SERVICES; LOSS OF  USE, DATA, OR PROFITS; OR  BUSINESS INTERRUPTION) HOWEVER
* CAUSED  AND  ON  ANY  THEORY  OF  LIABILITY,  WHETHER  IN  CONTRACT,  STRICT
* LIABILITY, OR TORT  (INCLUDING NEGLIGENCE OR OTHERWISE)  ARISING IN ANY  WAY
* OUT OF THE  USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH
* DAMAGE.
*
*****************************************************************************/

#include <MapNode.h>
#include <Connection.h>
#include <visitstream.h>

using namespace std;

// ****************************************************************************
//  Method:  MapNode::MapNode
//
//  Purpose:
//
//
//  Programmer:  Cyrus Harrison
//  Creation:    December 14, 2007
//
// ****************************************************************************
MapNode::MapNode()
{;}

MapNode::MapNode(const MapNode &node)
: Variant(node)
{
    // copy entires
    entries = node.entries;
}

MapNode::MapNode(const XMLNode &node)
{
    SetValue(node);
}

MapNode::MapNode(const XMLNode *node)
{
    SetValue(*node);
}

// ****************************************************************************
//  Method:  MapNode::~MapNode
//
//  Purpose:
//     MapNode destructor.
//
//  Programmer:  Cyrus Harrison
//  Creation:    December 14, 2007
//
// ****************************************************************************
MapNode::~MapNode()
{
    // cleanout nodes
}

// ****************************************************************************
//  Method:  MapNode::operator=(avtPolynomial)
//
//  Purpose:
//     MapNode assignment operator.
//
//  Programmer:  Cyrus Harrison
//  Creation:    December 14, 2007
//
//  Modifications:
//    Brad Whitlock, Mon Jan 12 10:50:14 PST 2009
//    Clear out the entries node before calling Variant::SetValue.
//
// ****************************************************************************

MapNode &
MapNode::operator=(const MapNode &node)
{
    // copy entries
    if(this != &node)
    {
        if(node.entries.size() > 0)
            entries = node.entries;
        else
        {
            entries.clear();
            Variant::SetValue(node);
        }
    }
    return *this;
}

MapNode &
MapNode::operator=(bool val)
{
    Variant::operator=(val);
    return *this;
}

MapNode &
MapNode::operator=(char val)
{
    Variant::operator=(val);
    return *this;
}

MapNode &
MapNode::operator=(unsigned char val)
{
    Variant::operator=(val);
    return *this;
}

MapNode &
MapNode::operator=(const char *val)
{
    Variant::operator=(val);
    return *this;
}

MapNode &
MapNode::operator=(int val)
{
    Variant::operator=(val);
    return *this;
}


MapNode &
MapNode::operator=(long val)
{
    Variant::operator=(val);
    return *this;
}

MapNode &
MapNode::operator=(float val)
{
    Variant::operator=(val);
    return *this;
}

MapNode &
MapNode::operator=(double val)
{
    Variant::operator=(val);
    return *this;
}

MapNode &
MapNode::operator=(const string& val)
{
    Variant::operator=(val);
    return *this;
}

MapNode &
MapNode::operator=(const boolVector& val)
{
    Variant::operator=(val);
    return *this;
}

MapNode &
MapNode::operator=(const charVector& val)
{
    Variant::operator=(val);
    return *this;
}

MapNode &
MapNode::operator=(const unsignedCharVector& val)
{
    Variant::operator=(val);
    return *this;
}

MapNode &
MapNode::operator=(const intVector& val)
{
    Variant::operator=(val);
    return *this;
}

MapNode &
MapNode::operator=(const longVector& val)
{
    Variant::operator=(val);
    return *this;
}

MapNode &
MapNode::operator=(const floatVector& val)
{
    Variant::operator=(val);
    return *this;
}

MapNode &
MapNode::operator=(const doubleVector& val)
{
    Variant::operator=(val);
    return *this;
}


// ****************************************************************************
//  Method:  MapNode::operator[]
//
//  Purpose:
//     Dictionary access operator.
//
//  Programmer:  Cyrus Harrison
//  Creation:    December 14, 2007
//
// ****************************************************************************
MapNode &
MapNode::operator[](const string &key)
{
    return entries[key];
}




// ****************************************************************************
//  Method:  MapNode::GetEntry
//
//  Purpose:
//     MapNode entry access.
//
//  Programmer:  Cyrus Harrison
//  Creation:    December 14, 2007
//
// ****************************************************************************
MapNode *
MapNode::GetEntry(const string &key)
{
    return &entries[key];
}

// ****************************************************************************
//  Method:  MapNode::GetEntry
//
//  Purpose:
//     MapNode entry access.
//
//  Programmer:  Cyrus Harrison
//  Creation:    December 14, 2007
//
// ****************************************************************************
const MapNode *
MapNode::GetEntry(const string &key) const
{
    map<string,MapNode>::const_iterator itr = entries.find(key);
    if(itr == entries.end())
        return NULL;
    else
        return &itr->second;
}

// ****************************************************************************
//  Method:  MapNode::HasEntry
//
//  Purpose:
//     Checks if the map has an entry for the given key.
//
//  Programmer:  Cyrus Harrison
//  Creation:    December 10, 2007
//
// ****************************************************************************
bool
MapNode::HasEntry(const string &key) const
{
    return entries.find(key) != entries.end();
}


// ****************************************************************************
//  Method:  MapNode::GetEntryNames
//
//  Purpose:
//     Returns all map key names in the result vector.
//
//  Programmer:  Cyrus Harrison
//  Creation:    December 14, 2007
//
// ****************************************************************************
void
MapNode::GetEntryNames(stringVector &result) const
{
    result.clear();
    map<string,MapNode>::const_iterator itr;
    for(itr = entries.begin(); itr != entries.end(); ++itr)
        result.push_back(itr->first);
}

// ****************************************************************************
// Method: MapNode::Reset
//
// Purpose: 
//   Reset the mapnode.
//
// Programmer: Brad Whitlock
// Creation:   Fri Jan  9 10:14:09 PST 2009
//
// Modifications:
//   
// ****************************************************************************

void
MapNode::Reset()
{
    entries.clear();
    Variant::Reset();
}

// ****************************************************************************
//  Method:  MapNode::ToXML
//
//  Purpose:
//     Creates an xml string representation of this node.
//
//  Programmer:  Cyrus Harrison
//  Creation:    December 14, 2007
//
// ****************************************************************************
string
MapNode::ToXML() const
{
    return ToXMLNode().ToString();
}

// ****************************************************************************
//  Method:  MapNode::ToXMLNode
//
//  Purpose:
//     Creates an xml representation of this node.
//
//  Programmer:  Cyrus Harrison
//  Creation:    December 14, 2007
//
// ****************************************************************************
XMLNode
MapNode::ToXMLNode() const
{
    XMLNode node;
    node.Name() = "map_node";
    // save children if not a value node
    if(entries.size() > 0 && Type() == 0)
    {
        map<string,MapNode>::const_iterator itr;
        for(itr = entries.begin(); itr != entries.end(); ++itr)
        {
            XMLNode *child = node.AddChild(itr->second.ToXMLNode());
            child->Attribute("key") = itr->first;
        }    
    }
    else // save value, if we have a value
    {
        node.AddChild(Variant::ToXMLNode());
    }
    
    return node;
}

// ****************************************************************************
//  Method:  MapNode::SetValue
//
//  Purpose:
//     Loads a MapNode from a xml representation.
//
//  Programmer:  Cyrus Harrison
//  Creation:    December 14, 2007
//
//  Modifications:
//
//    Tom Fogal, Thu Aug 21 14:28:43 EDT 2008
//    Remove an unused variable.
//
// ****************************************************************************
void 
MapNode::SetValue(const XMLNode &node)
{
    entries.clear();
    int nchildren = node.GetNumChildren();    
    if(node.Name()!="map_node" || nchildren == 0)
        return; // error

    // find out if we have child nodes, or if this node is a value node
    if(nchildren == 1 && node.GetChild(0)->Name() == "variant")
    {
        Variant::SetValue(node.GetChild(0));
    }
    else
    {
        for(int i=0;i<nchildren;i++)
        {
            XMLNode *child = node.GetChild(i);
            // children should have a key ....
            entries[child->Attribute("key")] = MapNode(child);
        }   
    }
}

// ****************************************************************************
// Method: MapNode::operator ==
//
// Purpose: 
//   Compares 2 MapNode objects.
//
// Arguments:
//   obj  : The object to compare.
//
// Returns:    True if the objects are equal; false otherwise.
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Tue Jan  6 15:33:24 PST 2009
//
// Modifications:
//   
// ****************************************************************************

bool
MapNode::operator ==(const MapNode &obj) const
{
    if(Type() != obj.Type())
        return false;

    bool equal = false;
    if(Type() == EMPTY_TYPE)
    {
        // Compare sizes
        if(entries.size() != obj.entries.size())
            return false;

        // Compare keys and values
        std::map<std::string,MapNode>::const_iterator it1 = entries.begin();
        std::map<std::string,MapNode>::const_iterator it2 = obj.entries.begin();
        for(; it1 != entries.end(); ++it1, ++it2)
        {
            // Compare keys. If they don't sort the same in the map then the
            // maps are different.
            if(it1->first != it2->first)
                return false;
        
            // recurse
            if(!(it1->second == it2->second))
                return false;
        }

        equal = true;
    }
    else
    {
        equal = Variant::operator==(obj);
    }

    return equal;
}

// ****************************************************************************
// Method: MapNode::CalculateMessageSize
//
// Purpose: 
//   Calculates the size of the message needed to store the serialized MapNode.
//
// Arguments:
//   conn : The connection doing the writing.
//
// Returns:    The message size.
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Tue Jan  6 15:35:48 PST 2009
//
// Modifications:
//   
// ****************************************************************************

int
MapNode::CalculateMessageSize(Connection &conn) const
{
    int messageSize = conn.IntSize(conn.DEST);

    if(Type() == EMPTY_TYPE)
    {
        messageSize += conn.IntSize(conn.DEST);

        map<string,MapNode>::const_iterator itr;
        for(itr = entries.begin(); itr != entries.end(); ++itr)
        {
            messageSize += conn.CharSize(conn.DEST) * (itr->first.size() + 1);
            messageSize += itr->second.CalculateMessageSize(conn);
        }
    }
    else
        messageSize += Variant::CalculateMessageSize(conn);

    return messageSize;
}

// ****************************************************************************
// Method: MapNode::Write
//
// Purpose: 
//   Write a MapNode to a Connection.
//
// Arguments:
//   conn : The connection to use for writing.
//
// Programmer: Brad Whitlock
// Creation:   Tue Jan  6 15:36:19 PST 2009
//
// Modifications:
//   
// ****************************************************************************

void
MapNode::Write(Connection &conn) const
{
    conn.WriteInt(Type());

    if(Type() == EMPTY_TYPE)
    {
        // Write the number of entries
        conn.WriteInt(entries.size());

        map<string,MapNode>::const_iterator itr;
        for(itr = entries.begin(); itr != entries.end(); ++itr)
        {
            // Write the name of the item
            conn.WriteString(itr->first);

            // Write the item data.
            itr->second.Write(conn);
        }
    }
    else
    {
        Variant::Write(conn);
    }
}

// ****************************************************************************
// Method: MapNode::Read
//
// Purpose: 
//   Reads the MapNode from the connection.
//
// Arguments:
//   conn : The connection to use for reading,.
//
// Programmer: Brad Whitlock
// Creation:   Tue Jan  6 15:36:46 PST 2009
//
// Modifications:
//   
// ****************************************************************************

void
MapNode::Read(Connection &conn)
{
    entries.clear();

    // Read the data type
    int dt;
    conn.ReadInt(&dt);
    Init(dt);

    if(dt == EMPTY_TYPE)
    {
        int nEntries = 0;
        conn.ReadInt(&nEntries);

        for(int i = 0; i < nEntries; ++i)
        {
            // Read the name of the item
            string name;
            conn.ReadString(name);

            // Read the item's data
#if 0
            MapNode child;
            child.Read(conn);

            // Add the item to the mapnode.
            entries[name] = child;
#else
            // Implicitly create the item and read its data.
            entries[name].Read(conn);
#endif
        }
    }
    else
    {
        Variant::Read(conn);
    }
}
