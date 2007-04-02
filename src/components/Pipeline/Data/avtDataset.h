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
//                                avtDataset.h                               //
// ************************************************************************* //

#ifndef AVT_DATASET_H
#define AVT_DATASET_H
#include <pipeline_exports.h>

#include <avtCommonDataFunctions.h>
#include <avtDataObject.h>
#include <avtDataTree.h>

class   avtDataRepresentation;


// ****************************************************************************
//  Class: avtDataset
//
//  Purpose:
//      The only example of a data object in our component base.  It contains
//      multiple blocks of a dataset, with each block in the form of a 
//      vtkDataSet.
//
//  Programmer: Hank Childs
//  Creation:   June 28, 2000
//
//  Modifications:
//
//    Kathleen Bonnell, Fri Feb  9 17:11:18 PST 2001 
//    Changed representation of domains to avtDomainTree. 
//
//    Kathleen Bonnell, Thu Apr  5 13:34:11 PDT 2001 
//    avtDomainTree renamed as avtDataTree, avtDomain renamed 
//    as avtDataRepresentation.  What used to be domains, an array of 
//    avtDomainTrees, is now just one avtDataTree. 
//
//    Hank Childs, Tue May 22 20:56:19 PDT 2001
//    Use database specifications instead of domain lists.
//
//    Kathleen Bonnell, Wed Sep 19 13:45:33 PDT 200 
//    Added method to write out tree structure. 
//
//    Hank Childs, Fri Sep 28 13:41:36 PDT 2001
//    Add Compact method.
//
//    Hank Childs, Fri Mar 15 17:14:25 PST 2002
//    Pushed some examination routines into the new object avtDatasetExaminer.
//    Removed comments related to that class.
//
//    Hank Childs, Thu Feb  5 17:11:06 PST 2004
//    Moved inlined constructor and destructor definitions to .C files
//    because certain compilers have problems with them.
//
//    Hank Childs, Mon Aug 29 14:57:42 PDT 2005
//    Add friend status for avtCMFEExpression.
//
//    Hank Childs, Thu Sep 22 17:34:12 PDT 2005
//    Add friend status for avtCompactTreeFilter.
//
//    Hank Childs, Sat Jan 21 13:17:12 PST 2006
//    Add friend status for avtSymmEvalExpression.
//
//    Hank Childs, Wed Aug  2 15:10:42 PDT 2006
//    Add friend status for avtLineScanQuery.
//
// ****************************************************************************

class PIPELINE_API avtDataset : public avtDataObject
{
    friend                   class avtDatasetSink;
    friend                   class avtDatasetSource;
    friend                   class avtSourceFromAVTDataset;
    friend                   class avtDatasetExaminer;
    friend                   class avtCMFEExpression;
    friend                   class avtCompactTreeFilter;
    friend                   class avtSymmEvalExpression;
    friend                   class avtLineScanQuery;

  public:
                             avtDataset(avtDataObjectSource *);
                             avtDataset(ref_ptr<avtDataset> ds,
                                        bool dontCopyData = false);
                             avtDataset(vtkDataSet *);
                             avtDataset(vtkDataSet *, avtDataset *);
    virtual                 ~avtDataset();

    virtual const char      *GetType(void)  { return "avtDataset"; };
    virtual int              GetNumberOfCells(bool polysOnly = false) const;
    virtual void             ReleaseData(void);

    virtual avtDataObject   *Instance(void);
    virtual avtDataObjectWriter 
                            *InstantiateWriter(void);

    void                     SetActiveVariable(const char *);

    void                     WriteTreeStructure(ostream &os, int indent = 0);
    void                     Compact(void);

  protected:
    avtDataTree_p            dataTree;

    void                     SetDataTree(avtDataTree_p);
    void                     SetDataTree(avtDataRepresentation &);
    avtDataTree_p           &GetDataTree() { return dataTree; };

    virtual void             DerivedCopy(avtDataObject *);
    virtual void             DerivedMerge(avtDataObject *);
};


typedef ref_ptr<avtDataset> avtDataset_p;


#endif


