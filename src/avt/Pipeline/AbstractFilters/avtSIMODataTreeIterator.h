/*****************************************************************************
*
* Copyright (c) 2000 - 2009, Lawrence Livermore National Security, LLC
* Produced at the Lawrence Livermore National Laboratory
* LLNL-CODE-400124
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
//                        avtSIMODataTreeIterator.h                          //
// ************************************************************************* //

#ifndef AVT_SIMO_DATA_TREE_ITERATOR_H
#define AVT_SIMO_DATA_TREE_ITERATOR_H

#include <pipeline_exports.h>

#include <avtDatasetToDatasetFilter.h>


class  avtExtents;


// ****************************************************************************
//  Class: avtSIMODataTreeIterator
//
//  Purpose:
//      A derived type of avtDatasetToDatasetFilter.  This will iterate over 
//      each vtkDataSet from an input avtDataTree.  This particular flavor
//      is "SIMO": Single Input Multiple Output, meaning that the output for
//      a single vtkDataSet can and likely will be multiple vtkDataSet outputs.
//
//  Programmer: Hank Childs
//  Creation:   July 24, 2000
//
//  Modifications:
//
//    Jeremy Meredith, Thu Sep 28 13:05:02 PDT 2000
//    Nade ExecuteDomain take one vtkDataSet as input and and return
//    a new output one.
//
//    Kathleen Bonnell, Fri Feb  9 14:47:10 PST 2001 
//    Renamed this class from avtDataTreeIterator.  Made ExecuteDomain return
//    an avtDomainTree.
//
//    Kathleen Bonnell, Thu Apr 12 10:25:04 PDT 2001 
//    Renamed this class as avtSIMODataTreeIterator. Added recursive
//    Execute method to walk down input tree.
//
//    Hank Childs, Wed Jun 20 09:39:01 PDT 2001
//    Added support progress callback.
//
//    Kathleen Bonnell, Wed Sep 19 13:45:33 PDT 2001
//    Added string argument to ExecuteDataTree method. 
//
//    Hank Childs, Wed Oct 24 14:21:18 PDT 2001
//    Moved PreExecute and PostExecute to avtFilter.
//
//    Hank Childs, Tue Nov  6 11:46:10 PST 2001
//    Add support for overriding extents.
//
//    Hank Childs, Mon Dec 27 10:58:14 PST 2004
//    Made inheritance virtual.
//
// **************************************************************************** 

class PIPELINE_API avtSIMODataTreeIterator : virtual public 
                                                      avtDatasetToDatasetFilter
{
   public:
                             avtSIMODataTreeIterator();
     virtual                ~avtSIMODataTreeIterator();

  protected:
    int                      currentNode;
    int                      totalNodes;

    virtual void             Execute(void);
    virtual avtDataTree_p    Execute(avtDataTree_p);
    virtual avtDataTree_p    ExecuteDataTree(vtkDataSet *,int,std::string) = 0;

    void                     OverrideTrueSpatialExtents(void)
                                 { overrideTrueSpatialExtents = true; };
    void                     OverrideTrueDataExtents(void)
                                 { overrideTrueDataExtents = true; };

  private:
    bool                     overrideTrueSpatialExtents;
    bool                     overrideTrueDataExtents;
    avtExtents              *trueSpatialExtents;
    avtExtents              *trueDataExtents;

    void                     UpdateExtents(avtDataTree_p);
};


#endif


