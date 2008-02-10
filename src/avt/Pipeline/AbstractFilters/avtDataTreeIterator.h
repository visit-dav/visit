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

// ************************************************************************* //
//                          avtDataTreeIterator.h                            //
// ************************************************************************* //

#ifndef AVT_DATA_TREE_ITERATOR_H
#define AVT_DATA_TREE_ITERATOR_H

#include <pipeline_exports.h>

#include <avtSIMODataTreeIterator.h>


// ****************************************************************************
//  Class: avtDataTreeIterator
//
//  Purpose:
//      This is an abstract type.  Its purpose is to provide a service.  That
//      service is that it walks through an input avtDataTree and calls the
//      method "ExecuteData" once for each vtkDataSet in the tree.  In addition,
//      it assembles an avtDataTree output from the outputs of each ExecuteData
//      call.
//
//  Programmer: Hank Childs
//  Creation:   July 24, 2000
//
//  Modifications:
//
//    Jeremy Meredith, Thu Sep 28 13:05:02 PDT 2000
//    Made ExecuteDomain take one vtkDataSet as input and and return
//    a new output one.
//
//    Kathleen Bonnell, Feb  9 14:47:10 PST 2001  
//    Removed 'Execute' method, and made this class inherit from 
//    avtDomainTreeDataTreeIterator.  Allows for derived types to still
//    return vtkDataSet * from ExecuteDomain, but now wrapped in
//    avtDomainTree.
//
//    Kathleen Bonnell, Tue Apr 10 10:49:10 PDT 2001 
//    Change inheritance to avtSIMODataTreeIterator. 
//
//    Kathleen Bonnell, Wed Sep 19 13:35:35 PDT 200 
//    Added string argument to Execute method. 
//
//    Hank Childs, Fri Feb  1 14:48:15 PST 2002
//    Added mechanism for managing memory for derived types.
//
//    Hank Childs, Tue Sep 10 13:13:01 PDT 2002
//    Better support for releasing data.
//
//    Hank Childs, Mon Dec 27 10:58:14 PST 2004
//    Made inheritance virtual.
//
//    Hank Childs, Thu Dec 21 09:17:43 PST 2006
//    Remove support for debug dumps.
//
// **************************************************************************** 

class PIPELINE_API avtDataTreeIterator : virtual public avtSIMODataTreeIterator
{
  public:
                             avtDataTreeIterator();
    virtual                 ~avtDataTreeIterator();

    virtual void             ReleaseData(void);

  protected:
    vtkDataSet              *lastDataset;

    virtual avtDataTree_p    ExecuteDataTree(vtkDataSet *, int, std::string);
    virtual vtkDataSet      *ExecuteData(vtkDataSet *, int, std::string) = 0;

    void                     ManageMemory(vtkDataSet *);
};


#endif


