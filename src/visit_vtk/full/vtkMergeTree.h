/*****************************************************************************
*
* Copyright (c) 2000 - 2017, Lawrence Livermore National Security, LLC
* Produced at the Lawrence Livermore National Laboratory
* LLNL-CODE-442911
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

    vtkTypeMacro(vtkMergeTree,vtkMutableDirectedGraph);

    static vtkMergeTree *New();

    vtkGetMacro(Threshold,double);
    vtkSetMacro(Threshold,double);

    vtkGetMacro(Maximum,double);
    vtkSetMacro(Maximum,double);

    vtkGetMacro(Minimum,double);
    vtkSetMacro(Minimum,double);

    // Create the right vertex data
    virtual void Initialize();


    // Construct a new node
    virtual vtkTypeUInt32 AddNode(vtkIdType id);

    // Return the mesh id corresponding to the given node index
    vtkIdType GetId(vtkTypeUInt32 index)
                {return GetVertexData()->GetArray(vtkMergeTree::MESH_ID)->GetTuple1(index);}

    // Return the node id of the representative for the given node index
    vtkTypeUInt32 GetRep(vtkTypeUInt32 index) 
                {return GetVertexData()->GetArray(vtkMergeTree::REP_ID)->GetTuple1(index);}

    // Set the representative for the given node
    void SetRep(vtkTypeUInt32 index, vtkTypeUInt32 rep)
            {return GetVertexData()->GetArray(vtkMergeTree::REP_ID)->SetTuple1(index,rep);}

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

    vtkTypeMacro(vtkSegmentedMergeTree,vtkMergeTree);

    static vtkSegmentedMergeTree *New();

    // Construct a new node
    virtual vtkTypeUInt32 AddNode(vtkIdType id);

    // Add a vertex to a branch
    void AddVertexToBranch(vtkTypeUInt32 branch, vtkIdType id);

    vtkIdType GetNumberOfBranches() const {return Branches.size();}

    const std::vector<vtkIdType>& GetBranch(vtkTypeUInt32 branch) const {return Branches[branch];}

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
