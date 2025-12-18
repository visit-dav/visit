// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

/*
 * vtkMergeTree.cxx
 *
 *  Created on: Jul 28, 2016
 *      Author: bremer5
 */

#include "vtkMergeTree.h"

#include <vtkObject.h>
#include <vtkObjectFactory.h>
#include <vtkDataObject.h>
#include <vtkIdTypeArray.h>
#include <vtkIdTypeArray.h>

// ****************************************************************************
// ****************************************************************************
//
// vtkMergeTree Class
//
// ****************************************************************************
// ****************************************************************************

vtkStandardNewMacro(vtkMergeTree)

// ****************************************************************************
//  Method: vtkMergeTree constructor
//
//  Programmer: Peer-Timo Bremer
//  Creation:   August 8, 2016
//
// ****************************************************************************
vtkMergeTree::vtkMergeTree() : vtkMutableDirectedGraph()
{
    Threshold = 0;

    // Create the id array as Ids
    GetVertexData()->AddArray(vtkIdTypeArray::New());

    // Create an array for representatives
    GetVertexData()->AddArray(vtkIdTypeArray::New());
}

// ****************************************************************************
//  Method: vtkMergeTree copy constructor
//
//  Programmer: Peer-Timo Bremer
//  Creation:   August 8, 2016
//
// ****************************************************************************
vtkMergeTree::vtkMergeTree(const vtkMergeTree&) 
{
    // Not implemented.
}

// ****************************************************************************
//  Method: vtkMergeTree assignment op
//
//  Programmer: Peer-Timo Bremer
//  Creation:   August 8, 2016
//
// ****************************************************************************
void
vtkMergeTree::operator=(const vtkMergeTree&)
{
    // Not implemented.
}

// ****************************************************************************
//  Method: vtkMergeTree destructor
//
//  Programmer: Peer-Timo Bremer
//  Creation:   August 8, 2016
//
// ****************************************************************************
vtkMergeTree::~vtkMergeTree()
{
    // empty
}

// ****************************************************************************
//  Method: vtkMergeTree::Initialize
//
//  Programmer: Peer-Timo Bremer
//  Creation:   August 8, 2016
//
// ****************************************************************************
void
vtkMergeTree::Initialize()
{
    // This clears all data
    vtkGraph::Initialize();

    //return;
    // Create the id array as Ids
    GetVertexData()->AddArray(vtkIdTypeArray::New());

    // Create an array for representatives
    GetVertexData()->AddArray(vtkIdTypeArray::New());
}


// ****************************************************************************
//  Method: vtkMergeTree::AddNode
//
//  Programmer: Peer-Timo Bremer
//  Creation:   August 8, 2016
//
// ****************************************************************************
vtkIdType
vtkMergeTree::AddNode(vtkIdType id)
{
    vtkIdType index = AddVertex();

    static_cast<vtkIdTypeArray*>(GetVertexData()->GetArray(vtkMergeTree::MESH_ID))->InsertValue(index,id);
    static_cast<vtkIdTypeArray*>(GetVertexData()->GetArray(vtkMergeTree::REP_ID))->InsertValue(index,index);

    return index;
}

vtkIdType
vtkMergeTree::GetId(vtkIdType index)
{
    return static_cast<vtkIdTypeArray *>(GetVertexData()->GetArray(vtkMergeTree::MESH_ID))->GetValue(index);
}

vtkIdType
vtkMergeTree::GetRep(vtkIdType index)
{
    return static_cast<vtkIdTypeArray *>(GetVertexData()->GetArray(vtkMergeTree::REP_ID))->GetValue(index);
}

void
vtkMergeTree::SetRep(vtkIdType index, vtkIdType rep)
{
    static_cast<vtkIdTypeArray *>(GetVertexData()->GetArray(vtkMergeTree::REP_ID))->SetValue(index, rep);
}


// ****************************************************************************
// ****************************************************************************
//
// vtkSegmentedMergeTree Class
//
// ****************************************************************************
// ****************************************************************************

vtkStandardNewMacro(vtkSegmentedMergeTree)

// ****************************************************************************
//  Method: vtkMergeTree constructor
//
//  Programmer: Peer-Timo Bremer
//  Creation:   August 8, 2016
//
// ****************************************************************************
vtkSegmentedMergeTree::vtkSegmentedMergeTree() : vtkMergeTree()
{
    //empty
}


// ****************************************************************************
//  Method: vtkSegmentedMergeTree copy constructor
//
//  Programmer: Peer-Timo Bremer
//  Creation:   August 8, 2016
//
// ****************************************************************************
vtkSegmentedMergeTree::vtkSegmentedMergeTree(const vtkSegmentedMergeTree&) 
{
    // Not implemented.
}

// ****************************************************************************
//  Method: vtkSegmentedMergeTree assignment op
//
//  Programmer: Peer-Timo Bremer
//  Creation:   August 8, 2016
//
// ****************************************************************************
void
vtkSegmentedMergeTree::operator=(const vtkSegmentedMergeTree&)
{
    // Not implemented.
}


// ****************************************************************************
//  Method: vtkMergeTree destructor
//
//  Programmer: Peer-Timo Bremer
//  Creation:   August 8, 2016
//
// ****************************************************************************
vtkSegmentedMergeTree::~vtkSegmentedMergeTree()
{
    //empty
}

// ****************************************************************************
//  Method: vtkSegmentedMergeTree::AddNode
//
//  Programmer: Peer-Timo Bremer
//  Creation:   August 8, 2016
//
// Modifications:
//   Cyrus Harrison, Wed Mar  6 13:48:48 PST 2002
//   Fixed use of wrong type in push_back (#2882)
//
// ****************************************************************************
vtkIdType
vtkSegmentedMergeTree::AddNode(vtkIdType id)
{
    vtkIdType index = vtkMergeTree::AddNode(id);

    Branches.push_back(std::vector<vtkIdType>());

    assert (Branches.size() == index+1);

    return index;
}

// ****************************************************************************
//  Method: vtkSegmentedMergeTree::AddVertexToBranch
//
//  Programmer: Peer-Timo Bremer
//  Creation:   August 8, 2016
//
// ****************************************************************************
void
vtkSegmentedMergeTree::AddVertexToBranch(size_t branch, vtkIdType id)
{
    Branches[branch].push_back(id);
}

