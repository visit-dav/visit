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
//                          avtDataTreeStreamer.h                            //
// ************************************************************************* //

#ifndef AVT_DATA_TREE_STREAMER_H
#define AVT_DATA_TREE_STREAMER_H

#include <pipeline_exports.h>

#include <avtDatasetToDatasetFilter.h>


class  avtExtents;


// ****************************************************************************
//  Class: avtDataTreeStreamer
//
//  Purpose:
//      A derived type of avtDatasetToDatasetFilter.  This will "stream" 
//      datasets through a single filter one at a time during execution.  It
//      could be part of the actual avtDatasetToDatasetFilter class, since
//      almost every derived type of avtDatasetToDatasetFilter will be 
//      interested in streaming their domains through, but it was made a
//      separate class to separate what functionality was for a filter (or
//      process object) in the pipeline and what sent domains through vtk
//      filters one at a time.
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
//    Renamed this class from avtStreamer.  Made ExecuteDomain return
//    an avtDomainTree.
//
//    Kathleen Bonnell, Thu Apr 12 10:25:04 PDT 2001 
//    Renamed this class as avtDataTreeStreamer. Added recursive
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

class PIPELINE_API avtDataTreeStreamer : virtual public 
                                                      avtDatasetToDatasetFilter
{
   public:
                             avtDataTreeStreamer();
     virtual                ~avtDataTreeStreamer();

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


