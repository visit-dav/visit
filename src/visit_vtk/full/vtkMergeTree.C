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
#include <vtkTypeUInt32Array.h>

// ****************************************************************************
// ****************************************************************************
//
// vtkMergeTree Class
//
// ****************************************************************************
// ****************************************************************************

vtkStandardNewMacro(vtkMergeTree);

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
    GetVertexData()->AddArray(vtkTypeUInt32Array::New());
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
    GetVertexData()->AddArray(vtkTypeUInt32Array::New());
}


// ****************************************************************************
//  Method: vtkMergeTree::AddNode
//
//  Programmer: Peer-Timo Bremer
//  Creation:   August 8, 2016
//
// ****************************************************************************
vtkTypeUInt32
vtkMergeTree::AddNode(vtkIdType id)
{
    vtkTypeUInt32 index = AddVertex();

    GetVertexData()->GetArray(vtkMergeTree::MESH_ID)->InsertTuple1(index,id);
    GetVertexData()->GetArray(vtkMergeTree::REP_ID)->InsertTuple1(index,index);

    return index;
}

// ****************************************************************************
// ****************************************************************************
//
// vtkSegmentedMergeTree Class
//
// ****************************************************************************
// ****************************************************************************

vtkStandardNewMacro(vtkSegmentedMergeTree);

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
// ****************************************************************************
vtkTypeUInt32
vtkSegmentedMergeTree::AddNode(vtkIdType id)
{
    vtkTypeUInt32 index = vtkMergeTree::AddNode(id);

    Branches.push_back(std::vector<vtkTypeInt64>());

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
vtkSegmentedMergeTree::AddVertexToBranch(vtkTypeUInt32 branch, vtkIdType id)
{
    Branches[branch].push_back(id);
}

