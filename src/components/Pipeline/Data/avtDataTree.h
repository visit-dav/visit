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

// ************************************************************************* //
//                               avtDataTree.h                               //
// ************************************************************************* //

#ifndef AVT_DATA_TREE_H
#define AVT_DATA_TREE_H

#include <pipeline_exports.h>

#include <vector>
#include <string>
#include <set>
#include <ref_ptr.h>

#include <avtDataRepresentation.h>

using std::vector;
using std::string;
using std::set;
 
class vtkDataSet;
class avtDataTree;
class avtWebpage;

typedef ref_ptr<avtDataTree> avtDataTree_p;

typedef void (*TraverseFunc)(avtDataRepresentation &, void *, bool &);


// ****************************************************************************
//  Class: avtDataTree
//
//  Purpose:
//     A tree structure allowing for a hierarchy of avtDataRepresentations.  
//     A leaf in the tree is a vtkDataset (wrapped as an avtDataRepresentation).
//     Internal nodes are themselves avtDataTrees.
//
//  Programmer: Kathleen Bonnell 
//  Creation:   February 1, 2001 
//
//  Modifications:
//
//    Hank Childs, Wed Mar  7 15:43:34 PST 2001
//    Removed methods GetDomain(int), Notify, and SetParent as well as
//    data member parent.
//
//    Hank Childs, Sat Apr  7 14:15:02 PDT 2001
//    Added method HasData to determine if there is data.
//
//    Kathleen Bonnell, Thu Apr  5 11:52:25 PDT 2001
//    Renamed as avtDataTree.  avtDomain renamed avtDataRepresentation.
//    Added Merge method.  Added Traverse method. Removed CreateDomain method.
//
//    Hank Childs, Tue Jun  5 09:55:39 PDT 2001
//    Added method GetChunk.
//
//    Hank Childs, Tue Jul 17 09:58:37 PDT 2001
//    Allow for an empty data tree.
//
//    Kathleen Bonnell, Mon Aug 13 13:05:59 PDT 2001 
//    Add method to write tree structure to desired output stream.
//
//    Kathleen Bonnell, Wed Sep 19 12:41:15 PDT 2001 
//    Removed GetChunk method (superceded by PruneTree).  
//
//    Kathleen Bonnell, Mon Sep 24 15:24:18 PDT 2001 
//    Added GetAllLabels method. 
//
//    Kathleen Bonnell, Thu Nov  1 11:10:39 PST 2001 
//    Added IsEmpty method. 
//
//    Hank Childs, Mon Nov 12 14:27:03 PST 2001
//    Added GetSingleLeaf.
//
//    Kathleen Bonnell, Mon Apr 29 17:49:25 PDT 2002 
//    Added GetAllUniqueLabels and GetUniqueLabels.
//
//    Hank Childs, Fri Jan  9 10:04:22 PST 2004
//    Added GetAllDomainIds.
//
//    Hank Childs, Thu Dec 21 10:06:47 PST 2006
//    Add support for debug dumps.
//
// ****************************************************************************

class PIPELINE_API avtDataTree
{
  public:
                             avtDataTree();
                             avtDataTree(vtkDataSet *, int);
                             avtDataTree(vtkDataSet *, int, string s);
                             avtDataTree(avtDataRepresentation &);
                             avtDataTree(int, vtkDataSet **, int *);
                             avtDataTree(int, vtkDataSet **,std::vector<int>&);
                             avtDataTree(int, vtkDataSet **, int);
                             avtDataTree(int, vtkDataSet **, int,
                                         std::vector<std::string>&);
                             avtDataTree(int, vtkDataSet **,int,std::string &);
                             avtDataTree(int, avtDataRepresentation *);
                             avtDataTree(avtDataTree_p, bool dontCopyData = false );
                             avtDataTree(int, avtDataTree_p *);
   
    virtual                 ~avtDataTree();

    avtDataTree             &operator =(avtDataTree&);

    avtDataTree_p            GetChild(int);
    avtDataRepresentation   &GetDataRepresentation(void); 
    int                      GetNumberOfCells(int topoDim, bool polysOnly) const;

    vtkDataSet              *GetSingleLeaf(void);
    int                      GetNumberOfLeaves(void);
    void                     Merge(avtDataTree_p);

    int                      GetNChildren(void) {return nChildren; } ;
    bool                     ChildIsPresent(int i) 
                                 { return *(children[i]) != NULL; } ;
    bool                     HasData(void) 
                                 { return (dataRep == NULL ? false : true); };
    bool                     IsEmpty(void);

    void                     Traverse(TraverseFunc, void *, bool &);

    vtkDataSet             **GetAllLeaves(int &);
    void                     GetAllDomainIds(vector<int> &);
    void                     GetAllLabels(vector<string> &);
    void                     GetAllUniqueLabels(vector<string> &);
    avtDataTree_p            PruneTree(const vector<int> &);
    avtDataTree_p            PruneTree(const vector<int> &, vector<int> &);
    avtDataTree_p            PruneTree(const vector<string> &);
    avtDataTree_p            PruneTree(const vector<string> &, vector<string>&);

    void                     WriteTreeStructure(ostream &, int indent = 0);

    void                     DebugDump(avtWebpage *, const char *, int, int);

  private:
    int                      nChildren;
    avtDataTree_p           *children;
    avtDataRepresentation   *dataRep; 

    void                     AddLeafToList(vtkDataSet**, int &); 
    void                     GetUniqueLabels(vector<string> &, set<string> &);
};


#endif
