/*****************************************************************************
*
* Copyright (c) 2000 - 2006, The Regents of the University of California
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

// ************************************************************************* //
//                           AttributeSubjectMap.C                           //
// ************************************************************************* //

#include <AttributeSubject.h>
#include <AttributeSubjectMap.h>
#include <DataNode.h>

#include <limits.h>

#define MAP_INCR 4

// ****************************************************************************
//  Method: AttributeSubjectMap constructor
//
//  Programmer: Eric Brugger
//  Creation:   November 15, 2002
//
// ****************************************************************************

AttributeSubjectMap::AttributeSubjectMap()
{
    nIndices   = 0;
    maxIndices = MAP_INCR;
    atts       = new AttributeSubject*[MAP_INCR];
    indices    = new int[MAP_INCR];
    for(int i = 0; i < MAP_INCR; ++i)
    {
        atts[i] = 0;
        indices[i] = 0;
    }
}

// ****************************************************************************
// Method: AttributeSubjectMap::AttributeSubjectMap
//
// Purpose: 
//   Copy constructor.
//
// Arguments:
//   obj : The object to copy.
//
// Programmer: Brad Whitlock
// Creation:   Tue Dec 30 17:05:04 PST 2003
//
// Modifications:
//   
// ****************************************************************************

AttributeSubjectMap::AttributeSubjectMap(const AttributeSubjectMap &obj)
{
    int i;

    nIndices   = obj.nIndices;
    maxIndices = obj.maxIndices;

    atts       = new AttributeSubject*[maxIndices];
    indices    = new int[maxIndices];

    for(i = 0; i < nIndices; ++i)
    {
        atts[i] = obj.atts[i]->NewInstance(true);
        indices[i] = obj.indices[i];
    }
    for(i = nIndices; i < maxIndices; ++i)
    {
        atts[i] = 0;
        indices[i] = 0;
    }
}

// ****************************************************************************
//  Method: AttributeSubjectMap destructor
//
//  Programmer: Eric Brugger
//  Creation:   November 15, 2002
//
// ****************************************************************************

AttributeSubjectMap::~AttributeSubjectMap()
{
    int i;

    for (i = 0; i < nIndices; ++i)
    {
        delete atts[i];
    }
    if (maxIndices > 0)
    {
        delete [] atts;
        delete [] indices;
    }
}

// ****************************************************************************
// Method: AttributeSubjectMap::operator =
//
// Purpose: 
//   Assignment operator.
//
// Arguments:
//   obj : The object to copy.
//
// Programmer: Brad Whitlock
// Creation:   Tue Dec 30 17:05:42 PST 2003
//
// Modifications:
//   
// ****************************************************************************

void
AttributeSubjectMap::operator =(const AttributeSubjectMap &obj)
{
    int i;
    for(i = 0; i < nIndices; ++i)
        delete atts[i];
    delete [] atts;

    nIndices   = obj.nIndices;
    maxIndices = obj.maxIndices;

    atts       = new AttributeSubject*[maxIndices];
    indices    = new int[maxIndices];

    for(i = 0; i < nIndices; ++i)
    {
        atts[i] = obj.atts[i]->NewInstance(true);
        indices[i] = obj.indices[i];
    }
    for(i = nIndices; i < maxIndices; ++i)
    {
        atts[i] = 0;
        indices[i] = 0;
    }
}

// ****************************************************************************
//  Method: AttributeSubjectMap::SetAtts
//
//  Purpose:
//    Set the attributes for the entire AttributeSubjectMap.  The map will
//    consist of a single entry with the attributes defined at index 0.
//
//  Arguments:
//    attr      The attribute subject to store.
//
//  Programmer: Eric Brugger
//  Creation:   December 27, 2002
//
// ****************************************************************************

void
AttributeSubjectMap::SetAtts(const AttributeSubject *attr)
{
    //
    // If we have at least one entry in the map, then delete all but the
    // first one, and set its index to 0.
    //
    if (nIndices > 0)
    {
        int i;

        for (i = 1; i < nIndices; ++i)
        {
            delete atts[i];
        }
        indices[0] = 0;
        nIndices = 1;
    }

    //
    // Set the attributes at index 0.
    //
    int i0, i1;

    SetAtts(0, attr, i0, i1);
}

// ****************************************************************************
//  Method: AttributeSubjectMap::SetAtts
//
//  Purpose:
//    Set the attributes for the AttributeSubjectMap at the specified index.
//
//  Arguments:
//    index     The index to store the attribute subject at.
//    attr      The attribute subject to store.
//
//  Programmer: Eric Brugger
//  Creation:   November 15, 2002
//
// ****************************************************************************

void
AttributeSubjectMap::SetAtts(const int index, const AttributeSubject *attr)
{
    int i0, i1;

    SetAtts(index, attr, i0, i1);
}

// ****************************************************************************
//  Method: AttributeSubjectMap::SetAttsLe
//
//  Purpose:
//    Set the attributes for the AttributeSubjectMap at the index that is
//    nearest to but less then or equal to the specified index
//
//  Arguments:
//    index     The given index
//    attr      The attribute subject to store.
//
//  Programmer: Mark C. Miller 
//  Creation:   March 25, 2004 
//
// ****************************************************************************

void
AttributeSubjectMap::SetAttsLe(const int index, const AttributeSubject *attr)
{

    // find an index that is less than or equal to the given index
    int i, i0, i1;
    for (i = 0; i < nIndices && indices[i] <= index; ++i);

    SetAtts(i, attr, i0, i1);
}

// ****************************************************************************
//  Method: AttributeSubjectMap::SetAtts
//
//  Purpose:
//    Set the attributes in the AttributeSubjectMap at the specified index
//    and return the range of the plots invalided by the operation.
//
//  Arguments:
//    index     The index to store the attribute subject at.
//    attr      The attribute subject to store.
//    i0        The returned index of the first attribute invalidated.
//              The argument will contain 0 if it extends to the
//              beginning of the range.
//    i1        The returned index of the last attribute invalidated.
//              The argument will contain INT_MAX if it extends to the
//              end of the range.
//
//  Programmer: Eric Brugger
//  Creation:   November 15, 2002
//
//  Modifications:
//    Brad Whitlock, Wed Jul 23 11:25:12 PDT 2003
//    Made it use NewInstance.
//
// ****************************************************************************

void
AttributeSubjectMap::SetAtts(const int index, const AttributeSubject *attr,
                             int &i0, int &i1)
{
    int i;
 
    for (i = 0; i < nIndices && indices[i] < index; ++i) ;
 
    //
    // If the index matches an existing index exactly then replace
    // the attributes for it, otherwise insert it in the list.
    //
    if (i < nIndices && indices[i] == index)
    {
        atts[i]->CopyAttributes(attr);
    }
    else
    {
        int j;

        //
        // Increase the length of the list if we need to.
        //
        if (nIndices == maxIndices)
        {
            ResizeMap(maxIndices+MAP_INCR);
        }

        //
        // Insert the item in the list.
        //
        for (j = nIndices; j > i; --j)
        {
            atts[j]  = atts[j-1];
            indices[j] = indices[j-1];
        }
        atts[i] = attr->NewInstance(true);
        indices[i] = index;
        nIndices++;
    }

    //
    // Determine the range of attributes invalidated.
    //
    if (i - 1 < 0)
        i0 = 0;
    else
        i0 = indices[i-1] + 1;
    if (i + 1 < nIndices)
        i1 = indices[i+1] - 1;
    else
        i1 = INT_MAX;
}

// ****************************************************************************
//  Method: AttributeSubjectMap::GetAtts
//
//  Purpose:
//    Get the attributes in the AttributeSubjectMap at the specified index.
//
//  Arguments:
//    index     The index to retrieve the attribute subject from.
//    attr      The attribute subject to store the results in.
//
//  Programmer: Eric Brugger
//  Creation:   November 15, 2002
//
// ****************************************************************************

void
AttributeSubjectMap::GetAtts(const int index, AttributeSubject *attr) const
{
    int i;

    if(nIndices > 0)
    {
        for (i = 0; i < nIndices && indices[i] <= index; ++i) ;

        if (i == 0)
        {
            attr->CopyAttributes(atts[0]);
        }
        else if (i == nIndices)
        {
            attr->CopyAttributes(atts[nIndices-1]);
        }
        else
        {
            int i0 = indices[i-1];
            int i1 = indices[i];
            double f = (double) (index - i0) / (double) (i1 - i0);

            attr->InterpolateLinear(atts[i-1], atts[i], f);
        }
    }
}

// ****************************************************************************
//  Method: AttributeSubjectMap::ClearAtts
//
//  Purpose:
//    Clear the attribute subjects in the map.
//
//  Programmer: Eric Brugger
//  Creation:   January 8, 2003
//
// ****************************************************************************

void
AttributeSubjectMap::ClearAtts()
{
    int i;
    for (i = 0; i < nIndices; ++i)
    {
        delete atts[i];
        atts[i] = 0;
        indices[i] = 0;
    }

    nIndices = 0;
}

// ****************************************************************************
//  Method: AttributeSubjectMap::DeleteAtts
//
//  Purpose:
//    Delete the attribute subject at the specified index.
//
//  Arguments:
//    index     The index at which to delete the attribute subject.
//    i0        The returned index of the first attribute invalidated.
//              The argument will contain 0 if it extends to the
//              beginning of the range.
//    i1        The returned index of the last attribute invalidated.
//              The argument will contain INT_MAX if it extends to the
//              end of the range.
//
//  Returns:
//    A boolean indicating if the attribute subject was deleted.
//
//  Programmer: Eric Brugger
//  Creation:   December 23, 2002
//
// ****************************************************************************

bool
AttributeSubjectMap::DeleteAtts(const int index, int &i0, int &i1)
{
    //
    // If the number of indices in the map is less than or equal to one,
    // then return since there must always be at least one index.
    //
    if (nIndices <= 1)
    {
        return false;
    }

    //
    // Find the index.
    //
    int i;
    for (i = 0; i < nIndices && indices[i] < index; ++i) ;
 
    //
    // If we found a match, then delete the attribute subject at
    // the index and compress the list. 
    //
    if (i < nIndices && indices[i] == index)
    {
        int j;

        delete atts[i];
        for (j = i; j < nIndices - 1; ++j)
        {
            indices[j] = indices[j+1];
            atts[j]    = atts[j+1];
        }
        nIndices--;

        //
        // Determine the range of attributes invalidated.
        //
        if (i - 1 < 0)
            i0 = 0;
        else
            i0 = indices[i-1] + 1;
        if (i < nIndices)
            i1 = indices[i] - 1;
        else
            i1 = INT_MAX;

        return true;
    }
    else
    {
        return false;
    }
}

// ****************************************************************************
//  Method: AttributeSubjectMap::MoveAtts
//
//  Purpose:
//    Move the position of an attribute subject.
//
//  Arguments:
//    oldIndex  The old index of the attribute subject.
//    newIndex  The new index of the attribute subject.
//    i0        The returned index of the first attribute invalidated.
//              The argument will contain 0 if it extends to the
//              beginning of the range.
//    i1        The returned index of the last attribute invalidated.
//              The argument will contain INT_MAX if it extends to the
//              end of the range.
//
//  Returns:
//    A boolean indicating if the attribute subject was moved.
//
//  Programmer: Eric Brugger
//  Creation:   January 28, 2003
//
// ****************************************************************************

bool
AttributeSubjectMap::MoveAtts(int oldIndex, int newIndex, int &i0, int &i1)
{
    //
    // If the old and new index are the same, do nothing.
    //
    if (oldIndex == newIndex)
    {
        return false;
    }

    //
    // Find the old index.
    //
    int i;
    for (i = 0; i < nIndices && indices[i] < oldIndex; ++i) ;
 
    //
    // If we found a match, then delete the attribute subject at
    // the index, compress the list, and set the attributes at
    // the new spot.
    //
    if (i < nIndices && indices[i] == oldIndex)
    {
        AttributeSubject *attr = atts[i];

        int j;
        for (j = i; j < nIndices - 1; ++j)
        {
            indices[j] = indices[j+1];
            atts[j]    = atts[j+1];
        }
        nIndices--;

        //
        // Determine the range of attributes invalidated.
        //
        if (i - 1 < 0)
            i0 = 0;
        else
            i0 = indices[i-1] + 1;
        if (i < nIndices)
            i1 = indices[i] - 1;
        else
            i1 = INT_MAX;

        //
        // Set the attributes at the new location.
        //
        int i2, i3;
        SetAtts(newIndex, attr, i2, i3);
        
        //
        // Update the range of attributes invalidated.
        //
        i0 = i0 < i2 ? i0 : i2;
        i1 = i1 > i3 ? i1 : i3;

        return true;
    }
    else
    {
        return false;
    }
}

// ****************************************************************************
//  Method: AttributeSubjectMap::GetNIndices
//
//  Purpose:
//    Get the number of indices in the map.
//
//  Returns:
//    The number of indices in the map.
//
//  Programmer: Eric Brugger
//  Creation:   January 7, 2003
//
// ****************************************************************************

int
AttributeSubjectMap::GetNIndices() const
{
    return nIndices;
}

// ****************************************************************************
//  Method: AttributeSubjectMap::GetIndices
//
//  Purpose:
//    Get the indices associated with the map.
//
//  Arguments:
//    nIndices_  The number of indices in the map.
//
//  Returns:
//    The indices in the map.
//
//  Programmer: Eric Brugger
//  Creation:   November 15, 2002
//
// ****************************************************************************

const int *
AttributeSubjectMap::GetIndices(int &nIndices_) const
{
    nIndices_ = nIndices;

    return indices;
}

// ****************************************************************************
//  Method: AttributeSubjectMap::CreateCompatible
//
//  Purpose:
//    Return a compatible attribute subject.
//
//  Arguments:
//    tname      The name of the attribute subject.  Used as a consistency
//               check.
//
//  Returns:
//    The compatible attribute subject.
//
//  Programmer: Eric Brugger
//  Creation:   November 15, 2002
//
// ****************************************************************************

AttributeSubject *
AttributeSubjectMap::CreateCompatible(const std::string &tname) const
{
    //
    // If we don't have any attributes, we can't return a compatible.
    //
    if (nIndices <= 0)
        return NULL;

    return atts[0]->CreateCompatible(tname);
}

// ****************************************************************************
//  Method: AttributeSubjectMap::CopyAttributes
//
//  Purpose:
//    Copies the attributes into the current object and returns whether or not
//    the attributes were copied.
//
//  Arguments:
//    attr      The attributes that we want to copy into the current object.
//
//  Returns:
//    A boolean indicating if the attributes were copied.
//
//  Programmer: Eric Brugger
//  Creation:   December 10, 2002
//
//  Modifications:
//    Brad Whitlock, Fri Dec 20 15:17:52 PST 2002
//    I made it return true.
//
// ****************************************************************************

bool
AttributeSubjectMap::CopyAttributes(const AttributeSubjectMap *attr)
{
    //
    // If we don't have any attributes, we can't copy them.
    //
    if (nIndices <= 0 || attr == NULL || attr->nIndices <= 0)
        return false;

    //
    // Try copying the first attributes to determine if the individual
    // attributes can be copied.
    //
    if (!atts[0]->CopyAttributes(attr->atts[0]))
        return false;
    indices[0] = attr->indices[0];

    //
    // Resize the map if necessary.
    //
    if (maxIndices < attr->nIndices)
    {
        ResizeMap(attr->maxIndices);
    }

    //
    // Copy the map.  Delete any excess AttributeSubjects if there were
    // fewer indices in the source than in the destination.
    //
    int i;
    for (i = 1; i < attr->nIndices; ++i)
    {
        if (i >= nIndices)
            atts[i] = attr->CreateCompatible(attr->atts[i]->TypeName());
        atts[i]->CopyAttributes(attr->atts[i]);
        indices[i] = attr->indices[i];
    }
    for (; i < nIndices; ++i)
    {
        delete atts[i];
    }
    nIndices = attr->nIndices;

    return true;
}

// ****************************************************************************
//  Method: AttributeSubjectMap::ResizeMap
//
//  Purpose:
//    Resizes the attribute subject map to be the new size.
//
//  Arguments:
//    newSize   The new size of the attribute subject map.
//
//  Programmer: Eric Brugger
//  Creation:   December 10, 2002
//
// ****************************************************************************

void
AttributeSubjectMap::ResizeMap(const int newSize)
{
    //
    // It is illegal to set the new the new size less than the current
    // number of indices, so return if that is the case.
    //
    if (newSize < nIndices)
        return;

    AttributeSubject **atts2 = new AttributeSubject*[newSize];
    int *indices2 = new int[newSize];

    int i;
    for (i = 0; i < nIndices; ++i)
    {
        atts2[i] = atts[i];
        indices2[i] = indices[i];
    }

    delete [] atts;
    delete [] indices;

    atts = atts2;
    indices = indices2;

    maxIndices = newSize;
}

// ****************************************************************************
// Method: AttributeSubjectMap::CreateNode
//
// Purpose: 
//   Saves the AttributeSubjectMap to a DataNode.
//
// Arguments:
//   parentNode : The node to which the map will be saved.
//
// Programmer: Brad Whitlock
// Creation:   Tue Jul 22 11:32:17 PDT 2003
//
// Modifications:
//   Brad Whitlock, Thu Dec 18 11:16:03 PDT 2003
//   I made it call CreateNode with the new completeSave flag set to true.
//
// ****************************************************************************

bool
AttributeSubjectMap::CreateNode(DataNode *parentNode)
{
    bool retval = false;

    if(parentNode == 0)
        return retval;

    if(nIndices > 0)
    {
        DataNode *mapNode = new DataNode("AttributeSubjectMap");
        parentNode->AddNode(mapNode);

        // Add the indices.
        intVector ids;
        int i;
        for(i = 0; i < nIndices; ++i)
            ids.push_back(indices[i]);
        mapNode->AddNode(new DataNode("indices", ids));

        // Add the attributes.
        DataNode *attNode = new DataNode("attributes");
        mapNode->AddNode(attNode);
        for(i = 0; i < nIndices; ++i)
            atts[i]->CreateNode(attNode, true, true);

        retval = true;
    }

    return retval;
}

// ****************************************************************************
// Method: AttributeSubjectMap::SetFromNode
//
// Purpose: 
//   Initializes the map using the data in the config file.
//
// Arguments:
//   parentNode : The data node that will be used to initialize the map.
//   factoryObj : The object that we use to create new instances of attributes.
//
// Programmer: Brad Whitlock
// Creation:   Tue Jul 22 11:33:21 PDT 2003
//
// Modifications:
//   
// ****************************************************************************

void
AttributeSubjectMap::SetFromNode(DataNode *parentNode,
    AttributeSubject *factoryObj)
{
    //
    // Clear the attributes.
    //
    ClearAtts();

    //
    // Look for the required nodes.
    //
    if(parentNode == 0)
        return;

    DataNode *mapNode = parentNode->GetNode("AttributeSubjectMap");
    if(mapNode == 0)
        return;

    DataNode *indicesNode = mapNode->GetNode("indices");
    if(indicesNode == 0)
        return;

    DataNode *attsNode = mapNode->GetNode("attributes");
    if(attsNode == 0)
        return;

    //
    // Now that we have all of the nodes that we need, read in the objects
    // and add them to the "map".
    //
    const intVector &iv = indicesNode->AsIntVector();
    DataNode **attsObjects = attsNode->GetChildren();
    const int numAtts = attsNode->GetNumChildren();
    for(int i = 0; i < iv.size(); ++i)
    {
        if(i < numAtts)
        {
            // Create a fresh AttributeSubject so that its fields are
            // initialized to the default values and not those last read in.
            AttributeSubject *reader = factoryObj->NewInstance(false);

            // Initialize the object using the data node.
            reader->SetFromNode(attsObjects[i]);

            // Add the object to the map.
            SetAtts(iv[i], reader);

            // delete the reader object.
            delete reader;
        }
    }
}
