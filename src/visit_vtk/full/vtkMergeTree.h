// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

/*
 * vtkMergeTree.h
 *
 *  Created on: Jul 28, 2016
 *      Author: bremer5
 */

#ifndef VTK_VTKMERGETREE_H_
#define VTK_VTKMERGETREE_H_
#include <visit_vtk_exports.h>

#include <stdint.h>
#include <vector>
#include "vtkCommonDataModelModule.h"
#include "vtkMutableDirectedGraph.h"
#include "vtkTypeInt64Array.h"
#include "vtkSmartPointer.h"
#include "vtkDataSetAttributes.h"


// ****************************************************************************
//  Class: vtkMergeTree
//
//  Programmer: Peer-Timo Bremer
//  Creation:   August 8, 2016
//
// ****************************************************************************

class VISIT_VTK_API vtkMergeTree : public vtkMutableDirectedGraph
{
public:

    enum MergeTreeAttributes
    {
        MESH_ID = 0,
        REP_ID = 1,
    };

    static vtkMergeTree *New();
    vtkTypeMacro(vtkMergeTree,vtkMutableDirectedGraph)


    vtkGetMacro(Threshold,double);
    vtkSetMacro(Threshold,double);

    vtkGetMacro(Maximum,double);
    vtkSetMacro(Maximum,double);

    vtkGetMacro(Minimum,double);
    vtkSetMacro(Minimum,double);

    // Create the right vertex data
    virtual void Initialize() override;


    // Construct a new node
    virtual vtkIdType AddNode(vtkIdType id);

    // Return the mesh id corresponding to the given node index
    vtkIdType GetId(vtkIdType index);

    // Return the node id of the representative for the given node index
    vtkIdType GetRep(vtkIdType index);

    // Set the representative for the given node
    void SetRep(vtkIdType index, vtkIdType rep);

protected:

    // Default constructor
    vtkMergeTree();

    // Destructor
    virtual ~vtkMergeTree();

    // Current threshold
    double Threshold;

    // The global "maximum" function value
    double Maximum;

    // The global "minimum" function value
    double Minimum;

private:
    vtkMergeTree(const vtkMergeTree&);   // private, not accessible 
    void operator=(const vtkMergeTree&); // private, not accessible 

};

// ****************************************************************************
//  Class: vtkSegmentedMergeTree
//
//  Programmer: Peer-Timo Bremer
//  Creation:   August 8, 2016
//
// ****************************************************************************

class VISIT_VTK_API vtkSegmentedMergeTree : public vtkMergeTree
{
public:

    vtkTypeMacro(vtkSegmentedMergeTree,vtkMergeTree)

    static vtkSegmentedMergeTree *New();

    // Construct a new node
    virtual vtkIdType AddNode(vtkIdType id) override;

    // Add a vertex to a branch
    void AddVertexToBranch(size_t branch, vtkIdType id);

    size_t GetNumberOfBranches() const {return Branches.size();}

    const std::vector<vtkIdType>& GetBranch(size_t branch) const {return Branches[branch];}

protected:

    // Description
    // A collection of mesh indices for each branch
    std::vector<std::vector<vtkIdType> > Branches;

    // Default constructor
    vtkSegmentedMergeTree();

    // Destructor
    virtual ~vtkSegmentedMergeTree();

private:

    vtkSegmentedMergeTree(const vtkSegmentedMergeTree&); // private, not accessible 
    void operator=(const vtkSegmentedMergeTree&); // private, not accessible 

};



#endif /* VTK_VTKMERGETREE_H_ */
