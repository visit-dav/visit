/*****************************************************************************
*
* Copyright (c) 2000 - 2007, The Regents of the University of California
* Produced at the Lawrence Livermore National Laboratory
* All rights reserved.
*
* This file is part of VisIt. For details, see http://www.llnl.gov/visit/. The
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
*    documentation and/or materials provided with the distribution.
*  - Neither the name of the UC/LLNL nor  the names of its contributors may be
*    used to  endorse or  promote products derived from  this software without
*    specific prior written permission.
*
* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT  HOLDERS AND CONTRIBUTORS "AS IS"
* AND ANY EXPRESS OR  IMPLIED WARRANTIES, INCLUDING,  BUT NOT  LIMITED TO, THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND  FITNESS FOR A PARTICULAR  PURPOSE
* ARE  DISCLAIMED.  IN  NO  EVENT  SHALL  THE  REGENTS  OF  THE  UNIVERSITY OF
* CALIFORNIA, THE U.S.  DEPARTMENT  OF  ENERGY OR CONTRIBUTORS BE  LIABLE  FOR
* ANY  DIRECT,  INDIRECT,  INCIDENTAL,  SPECIAL,  EXEMPLARY,  OR CONSEQUENTIAL
* DAMAGES (INCLUDING, BUT NOT  LIMITED TO, PROCUREMENT OF  SUBSTITUTE GOODS OR
* SERVICES; LOSS OF  USE, DATA, OR PROFITS; OR  BUSINESS INTERRUPTION) HOWEVER
* CAUSED  AND  ON  ANY  THEORY  OF  LIABILITY,  WHETHER  IN  CONTRACT,  STRICT
* LIABILITY, OR TORT  (INCLUDING NEGLIGENCE OR OTHERWISE)  ARISING IN ANY  WAY
* OUT OF THE  USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH
* DAMAGE.
*
*****************************************************************************/

#include <MapNode.h>
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
// ****************************************************************************
MapNode &
MapNode::operator=(const MapNode &node)
{
    // copy entires
    if(this != &node)
    {
        if(node.entries.size() > 0)
            entries = node.entries;
        else
            Variant::SetValue(node);
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
// ****************************************************************************
void 
MapNode::SetValue(const XMLNode &node)
{
    entries.clear();
    int nchildren = node.GetNumChildren();    
    if(node.Name()!="map_node" || nchildren == 0)
        return; // error

    XMLNode *child = node.GetChild(0);
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



